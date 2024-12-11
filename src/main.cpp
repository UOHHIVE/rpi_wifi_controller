#include "commons/hive_commons.hpp"

// #include "commons/src/dotenv/dotenv.cpp"
#include <chrono>
#include <memory>
#include <stdio.h>
#include <sys/time.h>
#include <thread>
#include <type_traits>
#include <utility>

#include "commons/src/flatbuf/commons_generated.h"
#include <cmath>

#define TPS 120            // ticks per second
#define MSPT 1000000 / TPS // microseconds per tick
#define TICK false         //
#define EB_XYZ 0.05        //
#define EB_ROT 0.05        //
#define HEADLESS true      // disables the movement, for use when testing

// State of the bot
struct BotState {
  uint64_t id;
  HiveCommon::Vec3 current_pos;
  HiveCommon::Vec4 current_rot;
  HiveCommon::Vec3 target_pos;
  bool sleep;
  long duration;
  bool aligned;
  bool clockwise;
};

// TODO: make sure buff is right size
static Lock<BotState> STATE;
static char BUFFER[1024];

// URL:
// https://stackoverflow.com/questions/158585/how-do-you-add-a-timed-delay-to-a-c-program
using namespace std::this_thread;     // sleep_for, sleep_until
using namespace std::chrono_literals; // ns, us, ms, s, h, etc.
using std::chrono::high_resolution_clock;
using std::chrono::system_clock;

// using namespace netcode;
// using namespace dotenv;

void bot_logic() {
  // define timing stuff
  std::chrono::_V2::system_clock::duration p1;
  std::chrono::_V2::system_clock::duration p2;

  int64_t t1;
  int64_t t2;

  int t_delay;

  logging::log("Starting Ticking", HEADLESS);

  while (TICK) {

    p1 = std::chrono::high_resolution_clock::now().time_since_epoch();
    t1 = std::chrono::duration_cast<std::chrono::microseconds>(p1).count();

    auto s = STATE.read();

    if (!s.sleep) {
      if (s.aligned) {
        if (misc::in_bound(s.current_pos.x(), s.target_pos.x(), EB_XYZ) xor misc::in_bound(s.current_pos.x(), s.target_pos.x(), EB_XYZ)) {

          logging::catch_debug(HEADLESS, zumo_movement::stop, "ZUMO: STOP");

          std::lock_guard<std::mutex> lock(STATE.mtx);
          STATE.inner.aligned = false;
        } else if (misc::in_bound(s.current_pos.x(), s.target_pos.x(), EB_XYZ) and misc::in_bound(s.current_pos.x(), s.target_pos.x(), EB_XYZ)) {

          logging::catch_debug(HEADLESS, zumo_movement::stop, "ZUMO: STOP");

        } else {
          logging::catch_debug(HEADLESS, zumo_movement::forward, "ZUMO: FORWARD");
        }
      } else {

        // T = target point, C = current point, Q = vec of mag 1 infront of where th ebot is facing

        float theta_q = 2 * asinf(s.current_rot.w());

        float cq_x = cos(theta_q);
        float cq_z = sin(theta_q);

        float ct_x = s.target_pos.x() - s.current_pos.x();
        float ct_z = s.target_pos.z() - s.current_pos.z();

        float q_x = s.current_pos.x() + cq_x;
        float q_z = s.current_pos.z() + cq_z;

        float lhs = sqrtf(ct_x * ct_x + ct_z * ct_z) * sqrtf(cq_x * cq_x + cq_z * cq_z);
        float rhs = ct_x * cq_x + ct_z * cq_z;

        float dot_ct_cq = rhs / lhs;

        if (misc::in_bound(dot_ct_cq, EB_ROT)) {
          std::lock_guard<std::mutex> lock(STATE.mtx);
          STATE.inner.aligned = true;
        } else {

          float qt_x = s.target_pos.x() - q_x;
          float qt_z = s.target_pos.z() - q_z;

          float m = qt_z / qt_x;
          float x = s.current_pos.x() - q_x;
          float z = m * x + q_z;

          bool clockwise = z > s.current_pos.z();

          std::lock_guard<std::mutex> lock(STATE.mtx);
          STATE.inner.clockwise = s.current_pos.x() > s.target_pos.x() ? !clockwise : clockwise;

          if (clockwise) {
            logging::catch_debug(HEADLESS, zumo_movement::turn_right, "ZUMO: RIGHT");

          } else {
            logging::catch_debug(HEADLESS, zumo_movement::turn_left, "ZUMO: LEFT");
          }
        }
      }

    } else {

      logging::catch_debug(HEADLESS, zumo_movement::stop, "ZUMO: STOP");

      if (s.sleep > 0) {
        std::this_thread::sleep_for(std::chrono::microseconds(s.sleep));

        std::lock_guard<std::mutex> lock(STATE.mtx);
        STATE.inner.duration = 0;
        STATE.inner.sleep = false;
      }

      // always reallign after a halt
      // shouldnt deadlock, but keep an eye
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
}

// TCP listener that gets spawned
void tcp_listener() {

  logging::log("Starting Listener...", HEADLESS);

  string dc_address = dotenv::DotEnv::get("DC_ADDRESS");
  string dc_port = dotenv::DotEnv::get("DC_PORT");
  netcode::Socket sock = netcode::Socket(dc_address.data(), std::stoi(dc_port));

  string name = dotenv::DotEnv::get("BOT_NAME");
  uint64_t id = std::stoll(DotEnv::get("ID_OVERRIDE")); // TODO: this borked when using hex

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

  logging::log("Sending Magic Num...", HEADLESS);

  sock.send_data(reinterpret_cast<char *>(fbb2.GetBufferPointer()), fbb2.GetSize());

  logging::log("Connection established", HEADLESS);

  while (1) {
    string message;
    sock.read_data(message); // TODO: look into fixing this

    if (message.size() > 0) {

      logging::log("Message: " + message, HEADLESS, logging::INFO, "dc_response");

      const HiveCommon::State *s = HiveCommon::GetState(message.c_str());

      if (!s) {
        logging::log("No State", HEADLESS, logging::WARN);
        continue;
      }

      const flatbuffers::Vector<flatbuffers::Offset<HiveCommon::Payload>> *p = s->payload();

      if (!p) {
        logging::log("No Payload", HEADLESS, logging::WARN);
        continue;
      }

      for (const auto &e : *p) {
        const HiveCommon::Entity *entity = e->data_nested_root();

        switch (entity->entity_type()) {
        case HiveCommon::EntityUnion_Node: {
          const auto node = entity->entity_as_Node();

          if (node->id() != STATE.read().id) {
            logging::log("Filtered ID", HEADLESS, logging::INFO, "filtered_node");
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

  logging::log("Startup", HEADLESS);

  // TODO: change this path later...
  dotenv::DotEnv::load("../.env");

  logging::log("Loaded EnvFile", HEADLESS);

  std::thread p_listener(tcp_listener);
  logging::log("Spawned Listener", HEADLESS);

  std::thread p_bot(bot_logic);
  logging::log("Spawned Bot Logic", HEADLESS);

  p_bot.join();
  p_listener.join();

  return 0;
}