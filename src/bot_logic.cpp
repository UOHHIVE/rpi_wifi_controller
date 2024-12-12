#include "commons/src/logging/logging.hpp"
#include "commons/src/utils/misc.hpp"
#include "commons/src/zumo/zumo.hpp"
#include "main.hpp"

#include <string>
#include <thread>

extern void bot_logic() {

  // define timing stuff
  std::chrono::_V2::system_clock::duration p1;
  std::chrono::_V2::system_clock::duration p2;

  int64_t t1;
  int64_t t2;

  int t_delay;

  logging::log(LOG_ENABLED, "Starting Ticking");

  // Tick loop
  while (TICK) {

    // get first time
    p1 = std::chrono::high_resolution_clock::now().time_since_epoch();
    t1 = std::chrono::duration_cast<std::chrono::microseconds>(p1).count();

    // read state
    auto s = STATE.read();

    if (!s.sleep) {

      // if the bot is not asleep
      if (s.aligned) {

        // realign if bot on axis of target
        if (misc::in_bound(s.current_pos.x(), s.target_pos.x(), EB_XYZ) xor misc::in_bound(s.current_pos.x(), s.target_pos.x(), EB_XYZ)) {
          logging::catch_debug(LOG_ENABLED, "ZUMO: STOP", zumo_movement::stop);

          std::lock_guard<std::mutex> lock(STATE.mtx);
          STATE.inner.aligned = false;
        }

        // stop if bot on target
        else if (misc::in_bound(s.current_pos.x(), s.target_pos.x(), EB_XYZ) && misc::in_bound(s.current_pos.x(), s.target_pos.x(), EB_XYZ)) {
          logging::catch_debug(LOG_ENABLED, "ZUMO: STOP", zumo_movement::stop);
        }

        // keep going forward. bot not on target, but still aligned
        else {
          logging::catch_debug(LOG_ENABLED, "ZUMO: FORWARD", zumo_movement::forward);
        }
      } else {
        // TODO: document this...

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
            logging::catch_debug(LOG_ENABLED, "ZUMO: RIGHT", zumo_movement::turn_right);
          } else {
            logging::catch_debug(LOG_ENABLED, "ZUMO: LEFT", zumo_movement::turn_left);
          }
        }
      }
    } else {
      logging::catch_debug(LOG_ENABLED, "ZUMO: STOP", zumo_movement::stop);

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

    // get second timepoint
    p2 = std::chrono::high_resolution_clock::now().time_since_epoch();
    t2 = std::chrono::duration_cast<std::chrono::microseconds>(p2).count();

    // delay if less than mspt
    t_delay = MSPT - (t2 - t1);
    std::this_thread::sleep_for(std::chrono::microseconds(t_delay));
  }
}