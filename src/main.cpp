#include "state.hpp"

Lock<BotState> STATE;

#include "state.hpp"

// TCP listener that gets spawned
extern void tcp_listener() {

  logging::log("Starting Listener...", TESTING);

  string dc_address = dotenv::DotEnv::get("DC_ADDRESS");
  string dc_port = dotenv::DotEnv::get("DC_PORT");
  netcode::Socket sock = netcode::Socket(dc_address.data(), std::stoi(dc_port));

  string name = dotenv::DotEnv::get("BOT_NAME");
  uint64_t id = std::stoll(dotenv::DotEnv::get("ID_OVERRIDE")); // TODO: this borked when using hex

  // Build the Presenter which is to be used in the Payload as a byte vector
  uint16_t sub = 0;

  // sub = encodeSubscriptionType(HiveCommon::SubscriptionType_Headset, sub);
  sub = misc::encodeSubscriptionType(HiveCommon::SubscriptionType_Own, sub);
  flatbuffers::FlatBufferBuilder fbb1;
  const auto fb_name = fbb1.CreateString(name);
  const auto robot = HiveCommon::CreateRobot(fbb1, id, fb_name, sub, HiveCommon::SubscriptionRate_Half);
  const auto entity = HiveCommon::CreateEntity(fbb1, HiveCommon::EntityUnion_Robot, robot.Union());
  fbb1.Finish(entity);
  fbb1.ForceVectorAlignment(fbb1.GetSize(), sizeof(uint8_t), fbb1.GetBufferMinAlignment());

  // Build the Payload which is to be used in the State as a payload vector
  flatbuffers::FlatBufferBuilder fbb2;
  const auto entityVec = fbb2.CreateVector(fbb1.GetBufferPointer(), fbb1.GetSize());
  const auto payload = HiveCommon::CreatePayload(fbb2, entityVec);

  std::vector<flatbuffers::Offset<HiveCommon::Payload>> payloadVector;
  payloadVector.push_back(payload);
  const auto payloads = fbb2.CreateVector(payloadVector);

  const auto state = HiveCommon::CreateState(fbb2, payloads);
  fbb2.FinishSizePrefixed(state);

  logging::log("Sending Magic Num...", TESTING);

  sock.send_data(reinterpret_cast<char *>(fbb2.GetBufferPointer()), fbb2.GetSize());

  logging::log("Connection established", TESTING);

  while (1) {
    string message;
    sock.read_data(message); // TODO: look into fixing this

    if (message.size() > 0) {

      logging::log("Message: " + message, TESTING, logging::INFO, "dc_response");

      const HiveCommon::State *s = HiveCommon::GetState(message.c_str());

      if (!s) {
        logging::log("No State", TESTING, logging::WARN);
        continue;
      }

      const flatbuffers::Vector<flatbuffers::Offset<HiveCommon::Payload>> *p = s->payload();

      if (!p) {
        logging::log("No Payload", TESTING, logging::WARN);
        continue;
      }

      for (const auto &e : *p) {
        const HiveCommon::Entity *entity = e->data_nested_root();

        switch (entity->entity_type()) {
        case HiveCommon::EntityUnion_Node: {
          const auto node = entity->entity_as_Node();

          if (node->id() != STATE.read().id) {
            logging::log("Filtered ID", TESTING, logging::INFO, "filtered_node");
            continue;
          }

          const auto pos = node->position();
          const auto rot = node->rotation();

          std::lock_guard<std::mutex> lock(STATE.mtx);

          STATE.inner.current_pos = *pos;
          STATE.inner.current_rot = *rot;
        }
        case HiveCommon::EntityUnion_Command: {

          const HiveCommon::Command *command = entity->entity_as_Command();

          // TODO: add logging here

          switch (command->command_type()) {
          case HiveCommon::CommandUnion_MoveTo: {
            const auto moveto = command->command_as_MoveTo();

            std::lock_guard<std::mutex> lock(STATE.mtx);

            STATE.inner.target_pos = *moveto->destination();
          }
          case HiveCommon::CommandUnion_Sleep: {
            const auto sleep = command->command_as_Sleep();

            std::lock_guard<std::mutex> lock(STATE.mtx);

            STATE.inner.sleep = sleep->sleep();
            STATE.inner.sleep = (long)(sleep->duration() * 1000000);
          }
          case HiveCommon::CommandUnion_Owner:
          case HiveCommon::CommandUnion_NONE:
            continue;
          }
        }
        case HiveCommon::EntityUnion_Robot:
        case HiveCommon::EntityUnion_Generic:
        case HiveCommon::EntityUnion_Geometry:
        case HiveCommon::EntityUnion_Headset:
        case HiveCommon::EntityUnion_Observer:
        case HiveCommon::EntityUnion_Presenter:
        case HiveCommon::EntityUnion_NONE:
          continue;
        }
      }
    }
  }
  sock.close_conn();
}

int main(void) {

  logging::log("Startup", TESTING);

  // TODO: change this path later...
  dotenv::DotEnv::load("../.env");

  logging::log("Loaded EnvFile", TESTING);

  std::thread p_listener(tcp_listener);
  logging::log("Spawned Listener", TESTING);

  // std::thread p_bot(bot_logic);
  // logging::log("Spawned Bot Logic", TESTING);

  // define timing stuff
  std::chrono::_V2::system_clock::duration p1;
  std::chrono::_V2::system_clock::duration p2;

  int64_t t1;
  int64_t t2;

  int t_delay;

  logging::log("Starting Ticking", TESTING);

  while (TICK) {
    p1 = std::chrono::high_resolution_clock::now().time_since_epoch();
    t1 = std::chrono::duration_cast<std::chrono::microseconds>(p1).count();

    auto s = STATE.read();

    if (!s.sleep) {
      if (s.aligned) {
        if (misc::in_bound(s.current_pos.x(), s.target_pos.x(), EB_XYZ) xor misc::in_bound(s.current_pos.x(), s.target_pos.x(), EB_XYZ)) {
          logging::catch_debug(TESTING, zumo_movement::stop, "ZUMO: STOP");
          std::lock_guard<std::mutex> lock(STATE.mtx);
          STATE.inner.aligned = false;
        } else if (misc::in_bound(s.current_pos.x(), s.target_pos.x(), EB_XYZ) && misc::in_bound(s.current_pos.x(), s.target_pos.x(), EB_XYZ)) {
          logging::catch_debug(TESTING, zumo_movement::stop, "ZUMO: STOP");
        } else {
          logging::catch_debug(TESTING, zumo_movement::forward, "ZUMO: FORWARD");
        }
      } else {
        // Handle bot's alignment logic...
        // Continue with your logic here
      }
    } else {
      logging::catch_debug(TESTING, zumo_movement::stop, "ZUMO: STOP");
      if (s.sleep > 0) {
        std::this_thread::sleep_for(std::chrono::microseconds(s.sleep));
        std::lock_guard<std::mutex> lock(STATE.mtx);
        STATE.inner.duration = 0;
        STATE.inner.sleep = false;
      }

      // Always re-align after a halt
      if (s.aligned) {
        std::lock_guard<std::mutex> lock(STATE.mtx);
        STATE.inner.aligned = false;
      }
    }

    p2 = std::chrono::high_resolution_clock::now().time_since_epoch();
    t2 = std::chrono::duration_cast<std::chrono::microseconds>(p2).count();
    t_delay = MSPT - (t2 - t1);
    std::this_thread::sleep_for(std::chrono::microseconds(t_delay));
  }

  // p_bot.join();
  p_listener.join();

  return 0;
}