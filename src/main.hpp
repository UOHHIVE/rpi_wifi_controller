#ifndef H_MAIN
#define H_MAIN

#include "commons/src/flatbuf/commons_generated.h"
#include "commons/src/netcode/netcode.hpp"
#include "commons/src/utils/lock.hpp"

#include <cmath>
#include <cstdint>
#include <sys/time.h>

#define TPS 20             // ticks per second
#define MSPT 1000000 / TPS // microseconds per tick
#define TICK true          //
#define EB_XYZ 0.05        //
#define EB_ROT 0.05        //
#define LOG_ENABLED true   // disables the movement if loggings enabled, for use when testing
#define LOG_LEVEL 1        // log level
#define TRACK_L 17         // Left Track pin
#define TRACK_R 27         // Right Track Pin
#define SAFETY 16          // Safety Pin
#define HBREAK 26          // Break Pin

// Struct representing the bots state
struct BotState {
  uint64_t id;
  string name;
  HiveCommon::Vec3 current_pos;
  HiveCommon::Vec4 current_rot;
  HiveCommon::Vec3 target_pos;
  bool sleep;
  long duration;
  bool aligned;
  bool clockwise;
  bool connected;
};

// Global State
extern utils::Lock<BotState> STATE;
// extern netcode::Socket SOCK; // this could be an issue...

// Function declarations
extern void bot_logic();
extern void tcp_listener();

#endif // MAIN

// keeping these jic:

// #include <chrono>
// #include <cmath>
// #include <memory>
// #include <mutex>
// #include <stdio.h>
// #include <sys/time.h>
// #include <thread>
// #include <type_traits>
// #include <utility>

// // URL:
// // https://stackoverflow.com/questions/158585/how-do-you-add-a-timed-delay-to-a-c-program
// using namespace std::this_thread;     // sleep_for, sleep_until
// using namespace std::chrono_literals; // ns, us, ms, s, h, etc.
// using std::chrono::high_resolution_clock;
// using std::chrono::system_clock;
