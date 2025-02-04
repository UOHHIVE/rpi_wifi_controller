#include "commons/src/dotenv/dotenv.hpp"
#include "commons/src/logging/logging.hpp"
#include "commons/src/netcode/netcode.hpp"
#include "commons/src/utils/misc.hpp"
#include "commons/src/utils/tick.hpp"
#include "main.hpp"

#include <string>

void tcp_setup(netcode::Socket sock) {
  // Build the Presenter which is to be used in the Payload as a byte vector
  uint16_t sub = 0;

  // sub = encodeSubscriptionType(HiveCommon::SubscriptionType_Headset, sub);
  sub = misc::encodeSubscriptionType(HiveCommon::SubscriptionType_Own, sub);
  flatbuffers::FlatBufferBuilder fbb1;
  const auto fb_name = fbb1.CreateString(STATE.read().name);
  const auto robot = HiveCommon::CreateRobot(fbb1, STATE.read().id, fb_name, sub, HiveCommon::SubscriptionRate_Half);
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

  logging::log(LOG_ENABLED, "Sending Magic Num...", LOG_LEVEL, 1, "tcp_listener");

  sock.send_data(reinterpret_cast<char *>(fbb2.GetBufferPointer()), fbb2.GetSize());

  std::lock_guard<std::mutex> lock(STATE.mtx);
  STATE.inner.connected = true;
}

void tcp_tick(netcode::Socket sock) {
  string message;
  sock.read_data(message); // ~~TODO: look into fixing this~~ fixed

  if (message.size() > 0) {

    // logging::log(LOG_ENABLED, "Message: " + message, LOG_LEVEL, 2, LogType::INFO, "dc_response");

    const HiveCommon::State *s = HiveCommon::GetState(message.c_str());

    if (!s) {
      logging::log(LOG_ENABLED, "No State in message", LOG_LEVEL, 1, LogType::WARN, "tcp_listener");
      return; // TODO: could cause issues in future
    }

    const flatbuffers::Vector<flatbuffers::Offset<HiveCommon::Payload>> *p = s->payload();

    if (!p) {
      logging::log(LOG_ENABLED, "No Payload in state", LOG_LEVEL, 1, LogType::WARN, "tcp_listener");
      return; // TODO: could cause issues in future
    }

    logging::log(LOG_ENABLED, "Recieved Message", LOG_LEVEL, 1, "tcp_listener");

    for (const auto &e : *p) {
      const HiveCommon::Entity *entity = e->data_nested_root();

      logging::log(LOG_ENABLED, "Extracted entity", LOG_LEVEL, 2, "tcp_listener");

      switch (entity->entity_type()) {
      case HiveCommon::EntityUnion_Node: {
        logging::log(LOG_ENABLED, "Decoding Node", LOG_LEVEL, 2, "tcp_listener");

        const auto node = entity->entity_as_Node();

        if (node->id() != STATE.read().id) {
          logging::log(LOG_ENABLED, "Filtered ID: " + std::to_string(node->id()) + " (" + std::to_string(STATE.read().id) + ")", LOG_LEVEL, 2, LogType::INFO, "tcp_listener");
          break;
        } else {
          logging::log(LOG_ENABLED, "ID Matched: " + std::to_string(node->id()), LOG_LEVEL, 2, LogType::INFO, "tcp_listener");
        }

        const auto pos = node->position();
        const auto rot = node->rotation();

        std::lock_guard<std::mutex> lock(STATE.mtx);

        STATE.inner.current_pos = *pos;
        STATE.inner.current_rot = *rot;

        break;
      }
      case HiveCommon::EntityUnion_Command: {
        logging::log(LOG_ENABLED, "Decoding Command", LOG_LEVEL, 2, "tcp_listener");
        const HiveCommon::Command *command = entity->entity_as_Command();

        // TODO: add logging here

        switch (command->command_type()) {
        case HiveCommon::CommandUnion_MoveTo: {
          const auto moveto = command->command_as_MoveTo();

          logging::log(LOG_ENABLED, "Moving to: x=" + std::to_string(moveto->destination()->x()) + " z=" + std::to_string(moveto->destination()->z()), LOG_LEVEL, 1, "tcp_listener");

          std::lock_guard<std::mutex> lock(STATE.mtx);
          STATE.inner.aligned = false;
          STATE.inner.target_completed = false;
          STATE.inner.target_pos = *moveto->destination();
          break;
        }
        case HiveCommon::CommandUnion_Sleep: {
          const auto sleep = command->command_as_Sleep();

          std::lock_guard<std::mutex> lock(STATE.mtx);

          STATE.inner.sleep = sleep->sleep();
          STATE.inner.sleep = (long)(sleep->duration() * 1000);
          break;
        }
        case HiveCommon::CommandUnion_Owner:
        case HiveCommon::CommandUnion_NONE:
          break;
        }

        break;
      }
      case HiveCommon::EntityUnion_Robot:
      case HiveCommon::EntityUnion_Generic:
      case HiveCommon::EntityUnion_Geometry:
      case HiveCommon::EntityUnion_Headset:
      case HiveCommon::EntityUnion_Observer:
      case HiveCommon::EntityUnion_Presenter:
      case HiveCommon::EntityUnion_NONE:
        break;
      }

      logging::log(LOG_ENABLED, "Finished parsing entity...", LOG_LEVEL, 2, "tcp_listener");
    }

    // BotState temp = STATE.read();
    // logging::log(LOG_ENABLED, "Coords - x=" + std::to_string(temp.current_pos.x()) + " z=" + std::to_string(temp.current_pos.x()));

  } else {
    logging::log(LOG_ENABLED, "Zero Bytes Read", LOG_LEVEL, 2, "tcp_listener");
  }
}

// TCP listener that gets spawned
extern void tcp_listener() {

  // return; // TODO: remove this after testing, this is just to stop excess logging while i figure out why the bot wont fukin turn

  logging::log(LOG_ENABLED, "Starting Listener...", LOG_LEVEL, 1, "tcp_listener");

  //! If any of these are blank, code may crash, needs to be fixed
  string dc_address = dotenv::DotEnv::get("DC_ADDRESS");
  string dc_port = dotenv::DotEnv::get("DC_PORT");

  logging::log(LOG_ENABLED, "Read EnVars", LOG_LEVEL, 1, "tcp_listener");
  logging::log(LOG_ENABLED, "Connecting to: " + dc_address + ":" + dc_port, LOG_LEVEL, 0, LogType::INFO, "netcode");

  netcode::Socket sock = netcode::Socket(dc_address.data(), std::stoi(dc_port));

  logging::log(LOG_ENABLED, "socket created", LOG_LEVEL, 1, "tcp_listener");

  tcp_setup(sock);

  logging::log(LOG_ENABLED, "Connection established", LOG_LEVEL, 1, "tcp_listener");

  utils::tick(tcp_tick, 1000, TICK, sock);

  logging::log(LOG_ENABLED, "listener closed", LOG_LEVEL, 1, "tcp_listener");
}
