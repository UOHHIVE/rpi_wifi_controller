#ifndef H_ROBOTCONTROLLER
#define H_ROBOTCONTROLLER

#include "commons/src/logging/logger.hpp"
#include "commons/src/math/vec/vec2.hpp"
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
  enum EBotActions { FORWARD, BACKWARD, TURN_LEFT, TURN_RIGHT, STOP };

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
    const Vec::Vec3 &T = s.target_pos;

    // log current position
    Logger::log("Q: w=" + std::to_string(Q.getW()) + ", x=" + std::to_string(Q.getX()) + ", y=" + std::to_string(Q.getY()) + ", z=" + std::to_string(Q.getZ()), LogLevel::Level::DEBUG);
    Logger::log("P: x=" + std::to_string(P.getX()) + ", y=" + std::to_string(P.getY()) + ", z=" + std::to_string(P.getZ()), LogLevel::Level::DEBUG);
    Logger::log("T: x=" + std::to_string(T.getX()) + ", y=" + std::to_string(T.getY()) + ", z=" + std::to_string(T.getZ()), LogLevel::Level::DEBUG);

    // Convert current position to Vec2 for 2D distance calculation
    Vec::Vec2 P2D(P.getX(), P.getZ());
    Vec::Vec2 T2D(T.getX(), T.getZ());

    // check if bot is within eb of target (2D distance)
    if (P2D.distance(T2D) < EB_XYZ) {
      std::lock_guard<std::mutex> lock(STATE.mtx);
      Logger::log("Inside Lock: checking if target completed", LogLevel::Level::INFO);

      STATE.inner.target_completed = true;
      return STOP;
    }

    // get axis
    Vec::Vec3 axis = Vec::Vec3(-1, 0, 0);

    // rotate axis by quaternion
    Vec::Vec3 robot_dir = axis.rotate(Q);

    // create required direction in 3D (but with Y=0)
    Vec::Vec3 required_dir_3D(T.getX() - P.getX(), 0, T.getZ() - P.getZ());
    required_dir_3D.norm();

    // project robot_dir onto XZ plane for comparison
    Vec::Vec3 robot_dir_flat(robot_dir.getX(), 0, robot_dir.getZ());
    robot_dir_flat.norm();

    // calculate error in the same 3D space
    float error = 1.0f - robot_dir_flat.dot(required_dir_3D);

    // cross product in 3D (Y component will be turn direction)
    Vec::Vec3 cross = robot_dir_flat.cross(required_dir_3D);
    bool turn_right = cross.getY() > 0;

    // dot product in 3D (go forwards or backwards)
	float dot = robot_dir_flat.dot(required_dir_3D);

    // log error
    Logger::log("Error: " + std::to_string(error), LogLevel::Level::INFO);

    if (dot > 0) {
        if (error < EB_ROT) {
            return FORWARD; // move forward if aligned
        }
        else {
            // need to turn
            if (turn_right) {
				return TURN_RIGHT;
            }
            else {
				return TURN_LEFT;
            }
        }
    }
    else {
		return BACKWARD; // move backward if facing away from target
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
      Logger::log("Target Completed", LogLevel::Level::DEBUG);
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
	case BACKWARD:
      Logger::log("Moving Backward", LogLevel::Level::INFO);
      Movement::backward();
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
