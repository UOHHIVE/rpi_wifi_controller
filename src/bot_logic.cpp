#include "main.hpp"

#include "commons/src/logging/logging.hpp"
#include "commons/src/utils/misc.hpp"
#include "commons/src/utils/tick.hpp"
#include "commons/src/zumo/zumo.hpp"

#include <iostream>
#include <string>
#include <thread>

void precalc(const BotState &s, float &theta_q, float &cq_x, float &cq_z, float &ct_x, float &ct_z, float &q_x, float &q_z) {

  // get angle from north
  theta_q = 2 * asinf(s.current_rot.w());

  // get vec
  cq_x = cos(theta_q);
  cq_z = sin(theta_q);

  // figure out where bot is pointing
  ct_x = s.target_pos.x() - s.current_pos.x();
  ct_z = s.target_pos.z() - s.current_pos.z();

  // find point
  q_x = s.current_pos.x() + cq_x;
  q_z = s.current_pos.z() + cq_z;
}

bool is_aligned(BotState &s) {
  // get angle from north
  float theta_q, cq_x, cq_z, ct_x, ct_z, q_x, q_z;
  precalc(s, theta_q, cq_x, cq_z, ct_x, ct_z, q_x, q_z);

  std::cout << std::to_string(theta_q) << std::endl;
  std::cout << std::to_string(cq_x) << std::endl;
  std::cout << std::to_string(cq_z) << std::endl;
  std::cout << std::to_string(ct_x) << std::endl;
  std::cout << std::to_string(ct_z) << std::endl;
  std::cout << std::to_string(q_x) << std::endl;
  std::cout << std::to_string(q_z) << std::endl;

  float lhs = sqrtf(ct_x * ct_x + ct_z * ct_z) * sqrtf(cq_x * cq_x + cq_z * cq_z);
  float rhs = ct_x * cq_x + ct_z * cq_z;

  float dot_ct_cq = rhs / lhs;

  return misc::in_bound(dot_ct_cq, EB_ROT);
}

bool clockwise(BotState &s) {
  float theta_q, cq_x, cq_z, ct_x, ct_z, q_x, q_z;
  precalc(s, theta_q, cq_x, cq_z, ct_x, ct_z, q_x, q_z);

  float qt_x = s.target_pos.x() - q_x;
  float qt_z = s.target_pos.z() - q_z;

  float m = qt_z / qt_x;
  float x = s.current_pos.x() - q_x;
  float z = m * x + q_z;

  return z > s.current_pos.z();
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

  std::string temp_x = std::to_string(s.current_pos.x());
  std::string temp_z = std::to_string(s.current_pos.z());

  logging::log(LOG_ENABLED, "pos: x=" + temp_x + ", z=" + temp_z, LOG_LEVEL, 1, "bot_logic");

  temp_x = std::to_string(s.target_pos.x());
  temp_z = std::to_string(s.target_pos.z());

  logging::log(LOG_ENABLED, "target: x=" + temp_x + ", z=" + temp_z, LOG_LEVEL, 1, "bot_logic");

  if (s.target_completed) {
    logging::log(LOG_ENABLED, "Target Completed", LOG_LEVEL, 1, "bot_logic");
    zumo_movement::stop();
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

    // Always re-align after a halt
    if (s.aligned) {
      std::lock_guard<std::mutex> lock(STATE.mtx);
      STATE.inner.aligned = false;
    }
  }

  zumo_movement::start(); // make sure handbreak is off

  bool at_half_pos = misc::in_bound(s.current_pos.x(), s.half_pos.x(), EB_XYZ) && misc::in_bound(s.current_pos.z(), s.half_pos.z(), EB_XYZ);

  if (s.aligned and !at_half_pos) {

    if (misc::in_bound(s.current_pos.x(), s.target_pos.x(), EB_XYZ) && misc::in_bound(s.current_pos.z(), s.target_pos.z(), EB_XYZ)) {
      logging::log(LOG_ENABLED, "BOT: Target Reached", LOG_LEVEL, 1, "bot_logic");
      zumo_movement::stop();
      std::lock_guard<std::mutex> lock(STATE.mtx);
      STATE.inner.target_completed = true;
    } else {
      logging::log(LOG_ENABLED, "BOT: forward", LOG_LEVEL, 1, "bot_logic");
      zumo_movement::forward();

      if (is_aligned(s)) {
        std::lock_guard<std::mutex> lock(STATE.mtx);
        STATE.inner.aligned = true;
      }
    }

  } else {
    if (is_aligned(s)) {
      logging::log(LOG_ENABLED, "BOT: aligned", LOG_LEVEL, 1, "bot_logic");

      if (!s.aligned) {
        std::lock_guard<std::mutex> lock(STATE.mtx);
        STATE.inner.aligned = true;
      }
    } else {
      if (clockwise(s)) {
        logging::log(LOG_ENABLED, "BOT: clockwise", LOG_LEVEL, 1, "bot_logic");
        zumo_movement::turn_right();
      } else {
        logging::log(LOG_ENABLED, "BOT: anticlockwise", LOG_LEVEL, 1, "bot_logic");
        zumo_movement::turn_left();
      }

      if (s.aligned) {
        std::lock_guard<std::mutex> lock(STATE.mtx);
        STATE.inner.aligned = false;
      }
    }

    // afer align, set half pos to between target and current pos
    std::lock_guard<std::mutex> lock(STATE.mtx);
    STATE.inner.half_pos = HiveCommon::Vec3((s.target_pos.x() + s.current_pos.x()) / 2, (s.target_pos.z() + s.current_pos.z()) / 2, 0);
  }
}

extern void bot_logic() {

  // while (true) {
  //   logging::log(LOG_ENABLED, "Starting Ticking Bot", LOG_LEVEL, 1, "bot_logic");
  //   zumo_movement::forward();
  //   std::this_thread::sleep_for(1s);
  //   zumo_movement::turn_left();
  //   std::this_thread::sleep_for(1s);
  //   zumo_movement::forward();
  //   std::this_thread::sleep_for(1s);
  //   zumo_movement::turn_right();
  //   std::this_thread::sleep_for(1s);
  //   zumo_movement::forward();
  //   std::this_thread::sleep_for(1s);
  // }

  logging::log(LOG_ENABLED, "Waiting for Connection...", LOG_LEVEL, 1, "bot_logic");

  while (!STATE.read().connected) {
    std::this_thread::sleep_for(5s);
  }

  logging::log(LOG_ENABLED, "Starting Ticking Bot", LOG_LEVEL, 1, "bot_logic");

  utils::tick(tick_bot, MSPT, TICK);

  logging::log(LOG_ENABLED, "Killing Bot Logic...", LOG_LEVEL, 1, "bot_logic");
}
