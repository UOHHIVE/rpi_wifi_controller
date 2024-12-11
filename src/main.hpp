#ifndef H_MAIN
#define H_MAIN

#include "commons/hive_commons.hpp"
#include <chrono>
#include <cmath>
#include <memory>
#include <mutex>
#include <stdio.h>
#include <sys/time.h>
#include <thread>
#include <type_traits>
#include <utility>

// URL:
// https://stackoverflow.com/questions/158585/how-do-you-add-a-timed-delay-to-a-c-program
using namespace std::this_thread;     // sleep_for, sleep_until
using namespace std::chrono_literals; // ns, us, ms, s, h, etc.
using std::chrono::high_resolution_clock;
using std::chrono::system_clock;

#define TPS 120            // ticks per second
#define MSPT 1000000 / TPS // microseconds per tick
#define TICK false         //
#define EB_XYZ 0.05        //
#define EB_ROT 0.05        //
#define TESTING true       // disables the movement, for use when testing

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

// Global state
extern Lock<BotState> STATE;

// Function declarations
void bot_logic();
void tcp_listener();

#endif // MAIN