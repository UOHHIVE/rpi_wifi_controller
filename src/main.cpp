// #include "commons/src/dotenv/dotenv.cpp"
#include "commons/hive_commons.hpp"
#include <chrono>
#include <memory>
#include <stdio.h>
#include <sys/time.h>
#include <thread>
#include <type_traits>
#include <utility>

#include "commons/src/flatbuf/commons_generated.h"

// URL:
// https://stackoverflow.com/questions/158585/how-do-you-add-a-timed-delay-to-a-c-program
using namespace std::this_thread;     // sleep_for, sleep_until
using namespace std::chrono_literals; // ns, us, ms, s, h, etc.
using std::chrono::high_resolution_clock;
using std::chrono::system_clock;

using namespace netcode;
using namespace dotenv;

#define TPS 120            // ticks per second
#define MSPT 1000000 / TPS // microseconds per tick
#define TICK false
#define EB_XYZ 0.05
#define EB_ROT 0.05

// TODO: Make a struct or a class to hold the bots dynamic info.
// use that as a placeholder for the loop while figuring out other stuff

// TODO: clean this section up, move to commons.

struct vec3 {
  int x;
  int y;
  int z;
};

struct BotState {
  uint64_t id;
  vec3 current_pos;
  vec3 current_rot;
  vec3 target_pos;
  bool halted;
  bool aligned;
};

template <typename T, typename C> bool in_bound(T val, T comp, C bound) { return val - bound <= val <= val + bound; }
template <typename T, typename C> bool in_bound(T val, C bound) { return val - bound <= val <= val + bound; }
template <typename T> bool in_bound(T val, T comp, T bound) { return comp - bound <= val <= comp + bound; }
template <typename T> bool in_bound(T val, T bound) { return val - bound <= val <= val + bound; }

// TODO: make sure buff is right size
static Lock<BotState> STATE;
static char BUFFER[1024];

inline uint16_t encodeSubscriptionType(const HiveCommon::SubscriptionType type, const uint16_t subscription = 0) { return static_cast<uint16_t>(1 << std::to_underlying(type)) | subscription; }

void tcp_listener() {

  printf("started listening...\n");

  string dc_address = DotEnv::get("DC_ADDRESS");
  string dc_port = DotEnv::get("DC_PORT");
  Socket s = Socket(dc_address.data(), std::stoi(dc_port));

  string name = DotEnv::get("BOT_NAME");
  uint64_t id = std::stoll(DotEnv::get("ID_OVERRIDE")); // TODO: this borked when using hex

  // Build the Presenter which is to be used in the Payload as a byte vector
  uint16_t sub = 0;

  // sub = encodeSubscriptionType(HiveCommon::SubscriptionType_Headset, sub);
  sub = encodeSubscriptionType(HiveCommon::SubscriptionType_Own, sub);
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

  s.send_data(reinterpret_cast<char *>(fbb2.GetBufferPointer()), fbb2.GetSize());

  while (1) {
    // TODO: start listening
    // TODO: on recv, update state

    // std::lock_guard<std::mutex> lock(STATE.mtx);
    // STATE.inner += 1;

    // if (STATE.inner >= 70) {
    //   STATE.inner = 0;
    // }

    // s._send(reinterpret_cast<char *>(&STATE.inner), sizeof(STATE.inner));
  }

  sleep_for(5s);
  s.close_conn();
}

int main(void) {

  // TODO: change this path later...
  dotenv::DotEnv::load("../.env");

  std::thread p_listener(tcp_listener);

  // define timing stuff
  std::chrono::_V2::system_clock::duration p1;
  std::chrono::_V2::system_clock::duration p2;

  int64_t t1;
  int64_t t2;

  int t_delay;

  while (TICK) {

    p1 = std::chrono::high_resolution_clock::now().time_since_epoch();
    t1 = std::chrono::duration_cast<std::chrono::microseconds>(p1).count();

    auto s = STATE.read();

    if (!s.halted) {
      if (s.aligned) {
        if (in_bound(s.current_pos.x, s.target_pos.x, EB_XYZ) xor in_bound(s.current_pos.x, s.target_pos.x, EB_XYZ)) {
          zumo_movement::stop();

          std::lock_guard<std::mutex> lock(STATE.mtx);
          STATE.inner.aligned = false;
        } else if (in_bound(s.current_pos.x, s.target_pos.x, EB_XYZ) and in_bound(s.current_pos.x, s.target_pos.x, EB_XYZ)) {
          zumo_movement::stop();
        } else {
          zumo_movement::forward();
        }
      } else {
        // TODO: calculate which direction to turn
        // TODO: turn til matched
        // TODO: lock state, set aligned
      }

    } else {
      zumo_movement::stop();

      // always reallign after a halt
      std::lock_guard<std::mutex> lock(STATE.mtx);
      STATE.inner.aligned = false;
    }

    printf("tick \n");
    printf("state=%d (tick delay: %d)\n", STATE.read(), t_delay);

    p2 = std::chrono::high_resolution_clock::now().time_since_epoch();
    t2 = std::chrono::duration_cast<std::chrono::microseconds>(p2).count();

    t_delay = MSPT - (t2 - t1);
    std::this_thread::sleep_for(std::chrono::microseconds(t_delay));
  }

  p_listener.join();

  return 0;
}