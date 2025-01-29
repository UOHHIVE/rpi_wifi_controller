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

// void precalc(const BotState &s, float &theta_q, float &cq_x, float &cq_z, float &ct_x, float &ct_z, float &q_x, float &q_z) {

//   // get angle from north
//   theta_q = 2 * asinf(s.current_rot.w());

//   // get vec
//   cq_x = cos(theta_q);
//   cq_z = sin(theta_q);

//   // figure out where bot is pointing
//   ct_x = (s.target_pos.x() - s.current_pos.x());
//   ct_z = (s.target_pos.z() - s.current_pos.z());

//   // find point
//   q_x = s.current_pos.x() + cq_x;
//   q_z = s.current_pos.z() + cq_z;
// }

// bool is_aligned(BotState &s) {
//   // get angle from north
//   float theta_q, cq_x, cq_z, ct_x, ct_z, q_x, q_z;
//   precalc(s, theta_q, cq_x, cq_z, ct_x, ct_z, q_x, q_z);

//   // std::cout << std::to_string(theta_q) << std::endl;
//   // std::cout << std::to_string(cq_x) << std::endl;
//   // std::cout << std::to_string(cq_z) << std::endl;
//   // std::cout << std::to_string(ct_x) << std::endl;
//   // std::cout << std::to_string(ct_z) << std::endl;
//   // std::cout << std::to_string(q_x) << std::endl;
//   // std::cout << std::to_string(q_z) << std::endl;

//   float lhs = sqrtf(ct_x * ct_x + ct_z * ct_z) * sqrtf(cq_x * cq_x + cq_z * cq_z);
//   float rhs = ct_x * cq_x + ct_z * cq_z;

//   float dot_ct_cq = rhs / lhs;
//   // std::cout << "AAAAAAAAAAAAAAAAAAAAAAAAAA DOT PRODUCT HERE: " + std::to_string(dot_ct_cq) << std::endl;

//   // return misc::in_bound(dot_ct_cq, EB_ROT);

//   return EB_ROT > dot_ct_cq && dot_ct_cq > (0 - EB_ROT);
// }

// bool clockwise(BotState &s) {
//   float theta_q, cq_x, cq_z, ct_x, ct_z, q_x, q_z;
//   precalc(s, theta_q, cq_x, cq_z, ct_x, ct_z, q_x, q_z);

//   float qt_x = s.target_pos.x() - q_x;
//   float qt_z = s.target_pos.z() - q_z;

//   float m = qt_z / qt_x;
//   float x = s.current_pos.x() - q_x;
//   float z = m * x + q_z;

//   return z > s.current_pos.z();
// }

// TODO: move this to commons
struct Vec2 {
  float x;
  float z;
};

enum EBotActions { FORWARD, TURN_LEFT, TURN_RIGHT, STOP };

// // Function to multiply two quaternions
// HiveCommon::Vec4 multiplyQuaternions(const HiveCommon::Vec4 &q1, const HiveCommon::Vec4 &q2) {
//   const auto &r_x = q1.w() * q2.x() + q1.x() * q2.w() + q1.y() * q2.z() - q1.z() * q2.y();
//   const auto &r_y = q1.w() * q2.y() - q1.x() * q2.z() + q1.y() * q2.w() + q1.z() * q2.x();
//   const auto &r_z = q1.w() * q2.z() + q1.x() * q2.y() - q1.y() * q2.x() + q1.z() * q2.w();
//   const auto &r_w = q1.w() * q2.w() - q1.x() * q2.x() - q1.y() * q2.y() - q1.z() * q2.z();
//   return HiveCommon::Vec4(r_w, r_x, r_y, r_z);
// }

// // Function to rotate a point using a quaternion
// HiveCommon::Vec3 rotatePoint(const HiveCommon::Vec4 &quat, const HiveCommon::Vec4 &point) {
//   // Convert the point to a quaternion (with w = 0)
//   // Vec4 p{point.x, point.y, point.z, 0.0f};
//   HiveCommon::Vec4 p(0.0f, point.x(), point.y(), point.z());

//   // Get the conjugate of the quaternion
//   HiveCommon::Vec4 conj(quat.w(), -quat.x(), -quat.y(), -quat.z());

//   // Rotate the point using the quaternion: p' = q * p * q_conj
//   HiveCommon::Vec4 rotated = multiplyQuaternions(multiplyQuaternions(quat, p), conj);

//   // Return the rotated point as a Float3
//   return HiveCommon::Vec3(rotated.x(), rotated.y(), rotated.z());
// }

// EBotActions do_action(BotState &s) {
//   if (s.target_completed || s.sleep) {
//     return STOP;
//   }

//   Vec2 P = {s.current_pos.x(), s.current_pos.z()};
//   Vec2 N = {0, 1};
//   Vec2 T = {s.target_pos.x(), s.target_pos.z()};

//   // HiveCommon::Vec4 Q = s.current_rot;

//   // float th_q = 2 * acosf(s.current_rot.w());
//   // float y_q = Q.y() / sin(2 / th_q);
//   // float x_q = Q.x() / sin(2 / th_q);
//   // float z_q = Q.z() / sin(2 / th_q);
//   // float Q_abs = sqrtf(Q.x() * Q.x() + Q.y() * Q.y() + Q.z() * Q.z() + Q.w() * Q.w());

//   // float x_q_norm = x_q / Q_abs;
//   // float z_q_norm = y_q / Q_abs;
//   // float zx_abs = sqrtf(x_q_norm * x_q_norm + z_q_norm * z_q_norm);

//   // Vec2 Q_O = {x_q_norm / zx_abs, z_q_norm / zx_abs};

//   HiveCommon::Vec3 rotated_point = rotatePoint(s.current_rot, HiveCommon::Vec4(P.x, 0, P.z, 0));
//   Vec2 Q_O = {rotated_point.x(), rotated_point.z()};

//   // Vec2 Q_O = {x_q / Q_abs, y_q / Q_abs};
//   Vec2 T_O = {T.x - P.x, T.z - P.z};

//   float T_O_abs = sqrtf(T_O.x * T_O.x + T_O.z * T_O.z);

//   Vec2 T_t = {0, T_O_abs};

//   float t_sin = T_O.x / T_O_abs;
//   float t_cos = T_O.z / T_O_abs;

//   Vec2 Q_t = {
//       Q_O.x * t_cos - Q_O.z * t_sin,
//       Q_O.x * t_sin + Q_O.z * t_cos,
//   };

//   float Q_t_abs = sqrtf(Q_t.x * Q_t.x + Q_t.z * Q_t.z);
//   float Q_t_dot_T_t = Q_t.x * T_t.x + Q_t.z * T_t.z;
//   float a = acosf(Q_t_dot_T_t / (Q_t_abs * T_O_abs));

//   if (a < EB_ROT) {
//     return FORWARD;
//   } else {
//     if (Q_t.x > 0) {
//       return TURN_RIGHT;
//     } else {
//       return TURN_LEFT;
//     }
//   }
// }

EBotActions do_acton(BotState &s) {
  if (s.target_completed || s.sleep) {
    return STOP;
  }

  // // Vec4 robotOrientation(0, -0.258819f, 0, 0.9659258f);  // get from tracker
  // Vec4 robotOrientation(0, 0.7071068f, 0, 0.7071068f); // get from tracker
  // Vec3 robotPosition(-10, 0, 0);                       // get from tracker
  // Vec3 targetPosition(10, 0, 10);                      // get from command

  // // calculate world axis
  // Vec3 robotAxis(0, 0, 1);
  // Vec3 robotDir;
  // rotate_vector_by_quaternion(robotAxis, robotOrientation, robotDir);

  // Vec3 requiredDir = targetPosition - robotPosition;
  // requiredDir = requiredDir.unit();

  // float error = 1.0f - fabs(Vec3::dot(robotDir, requiredDir));
  // bool turnRight = Vec3::cross(robotDir, requiredDir)._y > 0; // Note this might need reversing

  Math::Vec4 Q = Math::Vec4(0, s.target_pos.x(), 0, s.target_pos.z());
  Math::Vec3 P = Math::Vec3(s.current_pos.x(), 0, s.current_pos.z());
  Math::Vec3 T = Math::Vec3(s.target_pos.x(), 0, s.target_pos.z());

  Math::Vec3 axis = Math::Vec3(0, 0, 1);
  Math::Vec3 robot_dir = Math::Vec3::rotate(axis, Q);

  Math::Vec3 required_dir = (T - P).unit();

  float error = 1.0f - fabs(Math::Vec3::dot(robot_dir, required_dir));
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
