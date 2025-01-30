#include "commons/src/math/vec3.hpp"
#include "commons/src/math/vec4.hpp"
#include "main.hpp"

#include "commons/src/logging/logging.hpp"
#include "commons/src/utils/misc.hpp"
#include "commons/src/utils/tick.hpp"
#include "commons/src/zumo/zumo.hpp"

#include <iostream>
#include <string>
#include <thread>

// TODO: move this to commons
struct Vec2 {
  float x;
  float z;
};

enum EBotActions { FORWARD, TURN_LEFT, TURN_RIGHT, STOP };

EBotActions do_action(BotState &s) {
  if (s.target_completed || s.sleep) {
    return STOP;
  }

  Math::Vec4 Q = Math::Vec4(s.current_rot);
  Math::Vec3 P = Math::Vec3(s.current_pos);
  Math::Vec3 T = Math::Vec3(s.target_pos);

  Math::Vec3 axis = Math::Vec3(0, 0, 1);
  Math::Vec3 robot_dir = Math::Vec3::rotate(axis, Q);

  Math::Vec3 required_dir = (T - P).unit();

  float error = 1.0f - Math::Vec3::dot(robot_dir, required_dir);
  bool turn_right = Math::Vec3::cross(robot_dir, required_dir).y() > 0;

  if (error < EB_ROT) {
    return FORWARD;
  } else {
    if (turn_right) {
      return TURN_RIGHT;
    } else {
      return TURN_LEFT;
    }
  }
}

void tick_bot() {

  /*
    Bot Loop Logic:
      - if target is completed (or no target is set),
        - dont move
      - else check if bot is aligned
        - if not, align
      - else check if half way between current and target pos
        - if in bounds of half pos, realign
        - else
          - move forward

  */

  // read state
  auto s = STATE.read();
  // bool aligned = is_aligned(s);

  std::string temp_x = std::to_string(s.current_pos.x());
  std::string temp_z = std::to_string(s.current_pos.z());

  logging::log(LOG_ENABLED, "pos: x=" + temp_x + ", z=" + temp_z, LOG_LEVEL, 1, "bot_logic");

  temp_x = std::to_string(s.target_pos.x());
  temp_z = std::to_string(s.target_pos.z());

  logging::log(LOG_ENABLED, "target: x=" + temp_x + ", z=" + temp_z, LOG_LEVEL, 1, "bot_logic");

  if (s.target_completed) {
    logging::log(LOG_ENABLED, "Target Completed", LOG_LEVEL, 1, "bot_logic");
    zumo_movement::stop();
    return;
  }

  if (s.sleep) {
    logging::log(LOG_ENABLED, "Bot: Sleeping", LOG_LEVEL, 1, "bot_logic");
    zumo_movement::stop();

    // sleep for zero if duration is greater, if less, its permanent
    if (s.sleep > 0) {
      std::this_thread::sleep_for(std::chrono::microseconds(s.sleep));
      std::lock_guard<std::mutex> lock(STATE.mtx);
      STATE.inner.duration = 0;
      STATE.inner.sleep = false;
    }

    // // Always re-align after a halt
    // if (s.aligned) {
    //   std::lock_guard<std::mutex> lock(STATE.mtx);
    //   STATE.inner.aligned = false;
    // }
    // return;
  }

  // always remove handbreak
  zumo_movement::start();

  switch (do_action(s)) {
  case FORWARD:
    logging::log(LOG_ENABLED, "BOT: forward", LOG_LEVEL, 1, "bot_logic");
    zumo_movement::forward();
    break;
  case TURN_LEFT:
    logging::log(LOG_ENABLED, "BOT: turn left", LOG_LEVEL, 1, "bot_logic");
    zumo_movement::turn_left();
    break;
  case TURN_RIGHT:
    logging::log(LOG_ENABLED, "BOT: turn right", LOG_LEVEL, 1, "bot_logic");
    zumo_movement::turn_right();
    break;
  case STOP:
    logging::log(LOG_ENABLED, "BOT: stop", LOG_LEVEL, 1, "bot_logic");
    zumo_movement::stop();
    break;
  }
}

extern void bot_logic() {

  logging::log(LOG_ENABLED, "Waiting for Connection...", LOG_LEVEL, 1, "bot_logic");

  while (!STATE.read().connected) {
    std::this_thread::sleep_for(5s);
  }

  logging::log(LOG_ENABLED, "Starting Ticking Bot", LOG_LEVEL, 1, "bot_logic");

  utils::tick(tick_bot, MSPT, TICK);

  logging::log(LOG_ENABLED, "Killing Bot Logic...", LOG_LEVEL, 1, "bot_logic");
}
