#include "main.hpp"

#include "commons/src/logging/logging.hpp"
#include "commons/src/utils/misc.hpp"
#include "commons/src/utils/tick.hpp"
#include "commons/src/zumo/zumo.hpp"

#include <string>
#include <thread>

void tick_bot() {
  // read state
  auto s = STATE.read();

  std::string temp_x = std::to_string(s.current_pos.x());
  std::string temp_z = std::to_string(s.current_pos.z());
  logging::log(LOG_ENABLED, "x=" + temp_x + ", z=" + temp_z, LOG_LEVEL, 1, "bot_logic");

  if (!s.sleep) {
    logging::log(LOG_ENABLED, "Bot is Not Sleeped", LOG_LEVEL, 3, "bot_logic");

    // if the bot is not asleep
    if (s.aligned) {
      logging::log(LOG_ENABLED, "Bot is Aligned", LOG_LEVEL, 2, "bot_logic");

      // realign if bot on axis of target
      if (misc::in_bound(s.current_pos.x(), s.target_pos.x(), EB_XYZ) xor misc::in_bound(s.current_pos.x(), s.target_pos.x(), EB_XYZ)) {
        logging::catch_debug(LOG_MOVEMENT, "ZUMO: STOP", zumo_movement::stop);
        logging::log(LOG_ENABLED, "Bot: missaligned", LOG_LEVEL, 1, "bot_logic");

        std::lock_guard<std::mutex> lock(STATE.mtx);
        STATE.inner.aligned = false;
      }

      // stop if bot on target
      else if (misc::in_bound(s.current_pos.x(), s.target_pos.x(), EB_XYZ) && misc::in_bound(s.current_pos.x(), s.target_pos.x(), EB_XYZ)) {
        logging::catch_debug(LOG_MOVEMENT, "ZUMO: STOP", zumo_movement::stop);
        logging::log(LOG_ENABLED, "Bot: on Target", LOG_LEVEL, 1, "bot_logic");
      }

      // keep going forward. bot not on target, but still aligned
      else {
        logging::catch_debug(LOG_MOVEMENT, "ZUMO: FORWARD", zumo_movement::forward);
        logging::log(LOG_ENABLED, "Bot: Forward", LOG_LEVEL, 2, "bot_logic");
      }
    } else {
      logging::log(LOG_ENABLED, "Bot: Realigning", LOG_LEVEL, 2, "bot_logic");

      // T = target point, C = current point, Q = vec of mag 1 infront of where th ebot is facing

      // get angle from north
      float theta_q = 2 * asinf(s.current_rot.w());

      // get vec
      float cq_x = cos(theta_q);
      float cq_z = sin(theta_q);

      // figure out where bot is pointing
      float ct_x = s.target_pos.x() - s.current_pos.x();
      float ct_z = s.target_pos.z() - s.current_pos.z();

      // find point
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
          logging::catch_debug(LOG_MOVEMENT, "ZUMO: RIGHT", zumo_movement::turn_right);
          logging::log(LOG_ENABLED, "Bot: Turning Right", LOG_LEVEL, 2, "bot_logic");
        } else {
          logging::catch_debug(LOG_MOVEMENT, "ZUMO: LEFT", zumo_movement::turn_left);
          logging::log(LOG_ENABLED, "Bot: Turning Left", LOG_LEVEL, 2, "bot_logic");
        }
      }
    }
  } else {
    logging::catch_debug(LOG_MOVEMENT, "ZUMO: STOP", zumo_movement::stop);
    logging::log(LOG_ENABLED, "Bot: Sleeping", LOG_LEVEL, 2, "bot_logic");

    // sleep for zero if duration is greater, if less, its permanent
    if (s.sleep > 0) {
      std::this_thread::sleep_for(std::chrono::microseconds(s.sleep));
      std::lock_guard<std::mutex> lock(STATE.mtx);
      STATE.inner.duration = 0;
      STATE.inner.sleep = false;
    }

    // Always re-align after a halt
    if (s.aligned) {
      std::lock_guard<std::mutex> lock(STATE.mtx);
      STATE.inner.aligned = false;
    }
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
