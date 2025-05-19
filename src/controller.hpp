#ifndef H_ROBOTCONTROLLER
#define H_ROBOTCONTROLLER

#include "commons/src/logging/logger.hpp"
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
    const Vec::Vec3 &T = s.target_pos;

    // log current position
    Logger::log("Q: w=" + std::to_string(Q.getW()) + ", x=" + std::to_string(Q.getX()) + ", y=" + std::to_string(Q.getY()) + ", z=" + std::to_string(Q.getZ()), LogLevel::Level::INFO);
    Logger::log("P: x=" + std::to_string(P.getX()) + ", y=" + std::to_string(P.getY()) + ", z=" + std::to_string(P.getZ()), LogLevel::Level::INFO);
    Logger::log("T: x=" + std::to_string(T.getX()) + ", y=" + std::to_string(T.getY()) + ", z=" + std::to_string(T.getZ()), LogLevel::Level::INFO);

    // check if bot is within eb of target
    if (P.distance(T) < EB_XYZ) {
      std::lock_guard<std::mutex> lock(STATE.mtx);
      STATE.inner.target_completed = true;
      return STOP;
    }

    // get axis
    Vec::Vec3 axis = Vec::Vec3(-1, 0, 0);

    // rotate axis by quaternion
    Vec::Vec3 robot_dir = axis.rotate(Q);

    // get required direction
    Vec::Vec3 required_dir = (T - P);

    // normalize the vector
    required_dir.norm();

    // dot and cross product to get error and direction
    float error = 1.0f - robot_dir.dot(required_dir);
    bool turn_right = robot_dir.cross(required_dir).getY() > 0;

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
        STATE.inner.duration = 0;
        STATE.inner.sleep = false;
      }

      return;
    }

    // if current position is within error bounds, stop
    if (s.current_pos.distance(s.target_pos) < EB_XYZ) {
      Logger::log("Stopping Bot", LogLevel::Level::INFO);
      Movement::stop();
      std::lock_guard<std::mutex> lock(STATE.mtx);
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