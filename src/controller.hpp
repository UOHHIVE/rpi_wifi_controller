#ifndef H_ROBOTCONTROLLER
#define H_ROBOTCONTROLLER

#include "commons/src/logging/logger.hpp"
#include "commons/src/logging/vec2.hpp"
#include "commons/src/math/vec/vec3.hpp"
#include "commons/src/math/vec/vec4.hpp"
#include "commons/src/threading/tickable.hpp"
#include "commons/src/zumo/zumo.hpp"

#include "state.hpp"

#include <string>
#include <thread>

using namespace HIVE::Commons::Math;
using namespace HIVE::Commons::Zumo;
using namespace HIVE::Commons::Logging;
using namespace HIVE::Commons::Utils;

using namespace std::chrono_literals;

class RobotController : public HIVE::Commons::Threading::Tickable {
public:
  enum EBotActions { FORWARD, TURN_LEFT, TURN_RIGHT, STOP };

  RobotController(int tps) : Tickable(tps, "Robot") { HIVE::Commons::Zumo::GPIO::setup(); }

  inline EBotActions do_action(BotState &s) {
    const std::string log_name = "bot_logic.cpp::do_action";

    // if bot is sleeping or target is completed, stop
    if (s.target_completed || s.sleep) {
      return STOP;
    }

    // pull out the current rotation and position
    const Vec::Vec4 &Q = s.current_rot;
    const Vec::Vec3 &P = s.current_pos;
    const Vec::Vec2 T(s.target_pos.getX(), s.target_pos.getZ()); // Using X and Z for 2D plane

    // log current position
    Logger::log("Q: w=" + std::to_string(Q.getW()) + ", x=" + std::to_string(Q.getX()) + ", y=" + std::to_string(Q.getY()) + ", z=" + std::to_string(Q.getZ()), LogLevel::Level::INFO);
    Logger::log("P: x=" + std::to_string(P.getX()) + ", y=" + std::to_string(P.getY()) + ", z=" + std::to_string(P.getZ()), LogLevel::Level::INFO);
     Logger::log("T: x=" + std::to_string(T.getX()) + ", y=" + std::to_string(T.getY()), LogLevel::Level::INFO);

    // Convert current position to Vec2 for 2D distance calculation
    Vec::Vec2 P2D(P.getX(), P.getZ());

    // check if bot is within eb of target (2D distance)
    if (P2D.distance(T) < EB_XYZ) {
      std::lock_guard<std::mutex> lock(STATE.mtx);
      Logger::log("Inside Lock: checking if target completed", LogLevel::Level::INFO);

      STATE.inner.target_completed = true;
      return STOP;
    }

    // get axis
    Vec::Vec3 axis = Vec::Vec3(-1, 0, 0);

    // rotate axis by quaternion
    Vec::Vec3 robot_dir = axis.rotate(Q);

    // Convert robot direction to 2D (project onto XZ plane)
    Vec::Vec2 robot_dir_2D(robot_dir.getX(), robot_dir.getZ());
    robot_dir_2D.norm();

    // get required direction (2D)
    Vec::Vec2 required_dir = T - P2D;
    required_dir.norm(); // normalize the vector

    // dot and cross product to get error and direction (2D)
    float error = 1.0f - robot_dir_2D.dot(required_dir);
    // For 2D, cross product gives scalar (z-component)
    float cross_z = robot_dir_2D.getX() * required_dir.getY() - robot_dir_2D.getY() * required_dir.getX();
    bool turn_right = cross_z > 0;

    // log error
    Logger::log("Error: " + std::to_string(error), LogLevel::Level::INFO);

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

  void OnTick() override {
    auto s = STATE.read();

    // if not connected, set mspt to 1s, and sleep
    if (!s.connected) {
      Logger::log("Waiting for Connection... Sleeping bot", LogLevel::Level::DEBUG);
      Movement::stop();
      std::this_thread::sleep_for(std::chrono::seconds(1));
      return;
    }

    // if target is completed, stop
    if (s.target_completed) {
      Logger::log("Target Completed", LogLevel::Level::INFO);
      Movement::stop();
      return;
    }

    // if sleeping, sleep
    if (s.sleep) {
      Logger::log("Sleeping Bot", LogLevel::Level::INFO);
      Movement::stop();

      // sleep for zero if duration is greater, if less, its permanent
      if (s.sleep > 0) {
        std::this_thread::sleep_for(std::chrono::microseconds(s.sleep));
        std::lock_guard<std::mutex> lock(STATE.mtx);
        Logger::log("Inside Lock: Waking Bot", LogLevel::Level::INFO);
        STATE.inner.duration = 0;
        STATE.inner.sleep = false;
      }

      return;
    }

     // Convert positions to Vec2 for 2D distance check
    Vec::Vec2 P2D(s.current_pos.getX(), s.current_pos.getZ());
    Vec::Vec2 T2D(s.target_pos.getX(), s.target_pos.getZ());

    // if current position is within error bounds (2D), stop
    if (P2D.distance(T2D) < EB_XYZ) {
      Logger::log("Stopping Bot", LogLevel::Level::INFO);
      Movement::stop();
      std::lock_guard<std::mutex> lock(STATE.mtx);
      Logger::log("Inside Lock: Target Completed", LogLevel::Level::INFO);
      STATE.inner.target_completed = true;
      return;
    }
    
    // always remove handbreak
    Movement::start();

    // move based on math
    switch (do_action(s)) {
    case FORWARD:
      Logger::log("Moving Forward", LogLevel::Level::INFO);
      Movement::forward();
      break;
    case TURN_LEFT:
      Logger::log("Turning Left", LogLevel::Level::INFO);
      Movement::turn_left();
      break;
    case TURN_RIGHT:
      Logger::log("Turning Right", LogLevel::Level::INFO);
      Movement::turn_right();
      break;
    case STOP:
      Logger::log("Stopping Bot", LogLevel::Level::INFO);
      Movement::stop();
      break;
    }
  }

private:
  BotState state;
};

#endif // H_ROBOT
