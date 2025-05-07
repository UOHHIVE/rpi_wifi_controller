#include "commons/src/math/vec/vec3.hpp"
#include "commons/src/math/vec/vec4.hpp"
#include "main.hpp"

#include "commons/src/logging/logging.hpp"
// #include "commons/src/utils/tick.hpp"
#include "commons/src/zumo/zumo.hpp"

#include <string>
#include <thread>

using namespace hive::math;

enum EBotActions { FORWARD, TURN_LEFT, TURN_RIGHT, STOP };

inline EBotActions do_action(BotState &s) {
  const std::string log_name = "bot_logic.cpp::do_action";

  // if bot is sleeping or target is completed, stop
  if (s.target_completed || s.sleep) {
    return STOP;
  }

  // pull out the current rotation and position
  const vec::Vec4 &Q = s.current_rot;
  const vec::Vec3 &P = s.current_pos;
  const vec::Vec3 &T = s.target_pos;

  // log current position
  logging::log(LOG_ENABLED, "Q: w=" + std::to_string(Q.getW()) + ", x=" + std::to_string(Q.getX()) + ", y=" + std::to_string(Q.getY()) + ", z=" + std::to_string(Q.getZ()), LOG_LEVEL, 2, log_name);
  logging::log(LOG_ENABLED, "P: x=" + std::to_string(P.getX()) + ", y=" + std::to_string(P.getY()) + ", z=" + std::to_string(P.getZ()), LOG_LEVEL, 2, log_name);
  logging::log(LOG_ENABLED, "T: x=" + std::to_string(T.getX()) + ", y=" + std::to_string(T.getY()) + ", z=" + std::to_string(T.getZ()), LOG_LEVEL, 2, log_name);

  // check if bot is within eb of target
  // if (Math::Vec3::distance(P, T) < EB_XYZ) {
  if (P.distance(T) < EB_XYZ) {
    std::lock_guard<std::mutex> lock(STATE.mtx);
    STATE.inner.target_completed = true;
    return STOP;
  }

  // get axis
  vec::Vec3 axis = vec::Vec3(-1, 0, 0);

  // rotate axis by quaternion
  vec::Vec3 robot_dir = axis.rotate(Q);

  // get required direction
  vec::Vec3 required_dir = (T - P);

  // normalize the vector
  required_dir.norm();

  // dot and cross product to get error and direction
  float error = 1.0f - robot_dir.dot(required_dir);
  bool turn_right = robot_dir.cross(required_dir).getY() > 0;

  // log error
  logging::log(LOG_ENABLED, "Error: " + std::to_string(error), LOG_LEVEL, 2, log_name);

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
    logging::log(LOG_ENABLED, "Bot: Target Completed", LOG_LEVEL, 2, log_name);
    zumo::movement::stop();
    return;
  }

  // if sleeping, sleep
  if (s.sleep) {
    logging::log(LOG_ENABLED, "Bot: Sleeping", LOG_LEVEL, 2, log_name);
    zumo::movement::stop();

    // sleep for zero if duration is greater, if less, its permanent
    if (s.sleep > 0) {
      std::this_thread::sleep_for(std::chrono::microseconds(s.sleep));
      std::lock_guard<std::mutex> lock(STATE.mtx);
      STATE.inner.duration = 0;
      STATE.inner.sleep = false;
    }

    return;
  }

  // if current position is within error bounds, stop
  if (s.current_pos.distance(s.target_pos) < EB_XYZ) {
    logging::log(LOG_ENABLED, "Bot: Within Error Bounds", LOG_LEVEL, 2, log_name);
    zumo::movement::stop();
    std::lock_guard<std::mutex> lock(STATE.mtx);
    STATE.inner.target_completed = true;
    return;
  }

  // always remove handbreak
  zumo::movement::start();

  // move based on math
  switch (do_action(s)) {
  case FORWARD:
    logging::log(LOG_ENABLED, "BOT: forward", LOG_LEVEL, 2, log_name);
    zumo::movement::forward();
    break;
  case TURN_LEFT:
    logging::log(LOG_ENABLED, "BOT: turn left", LOG_LEVEL, 2, log_name);
    zumo::movement::turn_left();
    break;
  case TURN_RIGHT:
    logging::log(LOG_ENABLED, "BOT: turn right", LOG_LEVEL, 2, log_name);
    zumo::movement::turn_right();
    break;
  case STOP:
    logging::log(LOG_ENABLED, "BOT: stop", LOG_LEVEL, 2, log_name);
    zumo::movement::stop();
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
  zumo::utils::safe();
  logging::log(LOG_ENABLED, "Connected to Server, starting ticking", LOG_LEVEL, 0, log_name);

  utils::tick(tick_bot, MSPT, TICK);
  logging::log(LOG_ENABLED, "Killing Bot Logic...", LOG_LEVEL, 0, log_name);
}
