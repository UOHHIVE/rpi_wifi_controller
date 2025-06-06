#ifndef H_MAIN
#define H_MAIN

#include "commons/src/math/vec/vec3.hpp"
#include "commons/src/utils/lock.hpp"

#include <cstdint>
#include <string>

#define TPS 240            // ticks per second
#define MSPT 1000000 / TPS // microseconds per tick
#define TICK true          // true: run every tick, false: run once
#define EB_XYZ 0.15        // error bound for position
#define EB_ROT 0.10        // error bound for rotation
#define LOG_MOVEMENT true  // enables/disables track movement
#define LOG_ENABLED true   // enables/disables logging
#define LOG_LEVEL 4        // log level to be displayed
#define TRACK_L 17         // Left Track pin
#define TRACK_R 27         // Right Track Pin
#define SAFETY 16          // Safety Pin
#define HBREAK 26          // Break Pin
#define DRY_RUN            // use dry run for testing, pins will send debug messages instead of moving

using namespace HIVE::Commons::Math;

// Struct representing the bots state
struct BotState {
  uint64_t id;           // Bot's ID, pulled from the environment
  std::string name;      // Bot's name, pulled from the environment
  Vec::Vec3 current_pos; // Bot's current position (x, y, z)
  Vec::Vec3 target_pos;  // Bot's target position (x, y, z)
  Vec::Vec4 current_rot; // Bot's current rotation quaternion (x, y, z, w)
  bool target_completed; // Flag to indicate if the target has been reached
  bool sleep;            // Flag to indicate if the bot is sleeping
  long duration;         // Duration of the sleep, if 0, sleep is permanent
  bool aligned;          // Flag to indicate if the bot is aligned with the target
  bool clockwise;        // Flag to indicate if the bot should turn clockwise
  bool connected;        // Flag to indicate if the bot is connected to the server
};

// Global State
extern HIVE::Commons::Utils::Lock<BotState> STATE;

#endif // MAIN
