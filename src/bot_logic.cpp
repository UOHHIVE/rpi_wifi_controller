#include "commons/src/math/vec3.hpp"
#include "commons/src/math/vec4.hpp"
#include "main.hpp"

#include "commons/src/logging/logging.hpp"
#include "commons/src/utils/tick.hpp"
#include "commons/src/zumo/zumo.hpp"

#include <string>
#include <thread>

enum EBotActions { FORWARD, TURN_LEFT, TURN_RIGHT, STOP };

inline EBotActions do_action(BotState &s) {
  const std::string log_name = "bot_logic.cpp::do_action";

  // if bot is sleeping or target is completed, stop
  if (s.target_completed || s.sleep) {
    return STOP;
  }

  // pull out the current rotation and position
  Math::Vec4 Q = Math::Vec4(s.current_rot);
  Math::Vec3 P = Math::Vec3(s.current_pos);
  Math::Vec3 T = Math::Vec3(s.target_pos);

  // log current position
  logging::log(LOG_ENABLED, "Q: w=" + std::to_string(Q.w()) + ", x=" + std::to_string(Q.x()) + ", y=" + std::to_string(Q.y()) + ", z=" + std::to_string(Q.z()), LOG_LEVEL, 1, log_name);
  logging::log(LOG_ENABLED, "P: x=" + std::to_string(P.x()) + ", y=" + std::to_string(P.y()) + ", z=" + std::to_string(P.z()), LOG_LEVEL, 1, log_name);
  logging::log(LOG_ENABLED, "T: x=" + std::to_string(T.x()) + ", y=" + std::to_string(T.y()) + ", z=" + std::to_string(T.z()), LOG_LEVEL, 1, log_name);

  // check if bot is within eb of target
  if (Math::Vec3::distance(P, T) < EB_XYZ) {
    std::lock_guard<std::mutex> lock(STATE.mtx);
    STATE.inner.target_completed = true;
    return STOP;
  }

  // get direction of bot
  Math::Vec3 axis = Math::Vec3(-1, 0, 0);
  Math::Vec3 robot_dir = Math::Vec3::rotate(axis, Q);

  // get required direction
  Math::Vec3 required_dir = (T - P).unit();

  // dot and cross product to get error and direction
  float error = 1.0f - Math::Vec3::dot(robot_dir, required_dir);
  bool turn_right = Math::Vec3::cross(robot_dir, required_dir).y() > 0;

  // log error
  logging::log(LOG_ENABLED, "Error: " + std::to_string(error), LOG_LEVEL, 1, log_name);

  if (error < EB_ROT) {  // if error is within bounds
    return FORWARD;      //   move forward
  } else {               // else
    if (turn_right) {    //   if turn right
      return TURN_RIGHT; //     turn right
    } else {             //   else
      return TURN_LEFT;  //     turn left
    }
  }
}

void tick_bot() {
  const std::string log_name = "bot_logic.cpp::tick_bot";

  // read state
  auto s = STATE.read();

  // if target is completed, stop
  if (s.target_completed) {
    logging::log(LOG_ENABLED, "Bot: Target Completed", LOG_LEVEL, 1, log_name);
    zumo_movement::stop();
    return;
  }

  // if sleeping, sleep
  if (s.sleep) {
    logging::log(LOG_ENABLED, "Bot: Sleeping", LOG_LEVEL, 1, log_name);
    zumo_movement::stop();

    // sleep for zero if duration is greater, if less, its permanent
    if (s.sleep > 0) {
      std::this_thread::sleep_for(std::chrono::microseconds(s.sleep));
      std::lock_guard<std::mutex> lock(STATE.mtx);
      STATE.inner.duration = 0;
      STATE.inner.sleep = false;
    }

    return;
  }

  // always remove handbreak
  zumo_movement::start();

  // move based on math
  switch (do_action(s)) {
  case FORWARD:
    logging::log(LOG_ENABLED, "BOT: forward", LOG_LEVEL, 1, log_name);
    zumo_movement::forward();
    break;
  case TURN_LEFT:
    logging::log(LOG_ENABLED, "BOT: turn left", LOG_LEVEL, 1, log_name);
    zumo_movement::turn_left();
    break;
  case TURN_RIGHT:
    logging::log(LOG_ENABLED, "BOT: turn right", LOG_LEVEL, 1, log_name);
    zumo_movement::turn_right();
    break;
  case STOP:
    logging::log(LOG_ENABLED, "BOT: stop", LOG_LEVEL, 1, log_name);
    zumo_movement::stop();
    break;
  }
}

extern void bot_logic() {
  const std::string log_name = "bot_logic.cpp::bot_logic";

  logging::log(LOG_ENABLED, "Waiting for Connection...", LOG_LEVEL, 0, log_name);

  // while not connected, sleep
  while (!STATE.read().connected) {
    std::this_thread::sleep_for(5s);
  }

  // make zumo safe
  zumo_utils::safe();
  logging::log(LOG_ENABLED, "Connected to Server, starting ticking", LOG_LEVEL, 0, log_name);

  utils::tick(tick_bot, MSPT, TICK);
  logging::log(LOG_ENABLED, "Killing Bot Logic...", LOG_LEVEL, 0, log_name);
}
