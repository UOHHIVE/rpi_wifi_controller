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
#include <cmath>

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

struct float3 {
  float x;
  float y;
  float z;
};

struct float4 {
  float x;
  float y;
  float z;
  float w;
};

struct BotState {
  uint64_t id;
  float3 current_pos;
  float4 current_rot;
  float3 target_pos;
  bool halted;
  bool aligned;
  bool clockwise;
};

template <typename T, typename C> bool in_bound(T val, T comp, C bound) { return comp - bound <= val <= comp + bound; }
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
  Socket sock = Socket(dc_address.data(), std::stoi(dc_port));

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

  sock.send_data(reinterpret_cast<char *>(fbb2.GetBufferPointer()), fbb2.GetSize());

  while (1) {
    string message;
    sock.read_data(message); // TODO: look into fixing this

    const HiveCommon::State *s = HiveCommon::GetState(message.c_str() + 4);
    const flatbuffers::Vector<flatbuffers::Offset<HiveCommon::Payload>> *p_arr = s->payload();
    // TODO: get each payload out of p
    // TODO: filter each payload by being in the Node union
    // TODO: filter by bot ID

    std::lock_guard<std::mutex> lock(STATE.mtx);

    // TODO: update state
  }

  sleep_for(5s);
  sock.close_conn();
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

        // T = target point, C = current point, Q = vec of mag 1 infront of where th ebot is facing

        float theta_q = 2 * asinf(s.current_rot.w);

        float cq_x = cos(theta_q);
        float cq_z = sin(theta_q);

        float ct_x = s.target_pos.x - s.current_pos.x;
        float ct_z = s.target_pos.z - s.current_pos.z;

        float q_x = s.current_pos.x + cq_x;
        float q_z = s.current_pos.z + cq_z;

        float lhs = sqrtf(ct_x * ct_x + ct_z * ct_z) * sqrtf(cq_x * cq_x + cq_z * cq_z);
        float rhs = ct_x * cq_x + ct_z * cq_z;

        float dot_ct_cq = rhs / lhs;

        if (in_bound(dot_ct_cq, EB_ROT)) {
          std::lock_guard<std::mutex> lock(STATE.mtx);
          STATE.inner.aligned = true;
        } else {

          float qt_x = s.target_pos.x - q_x;
          float qt_z = s.target_pos.z - q_z;

          float m = qt_z / qt_x;
          float x = s.current_pos.x - q_x;
          float z = m * x + q_z;

          bool clockwise = z > s.current_pos.z;

          std::lock_guard<std::mutex> lock(STATE.mtx);
          STATE.inner.clockwise = s.current_pos.x > s.target_pos.x ? !clockwise : clockwise;

          if (clockwise) {
            zumo_movement::turn_right();
          } else {
            zumo_movement::turn_left();
          }
        }
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