#ifndef H_ROBOTREADER
#define H_ROBOTREADER

#include "commons/src/flatbuffers/generated/commons_generated.h"
#include "commons/src/logging/logger.hpp"
#include "commons/src/netcode/fbreader.hpp"
#include "state.hpp"

using namespace HIVE::Commons::Netcode;
using namespace HIVE::Commons::Logging;

namespace HIVE {
namespace Orchestrator {

class RobotReader : public FBReader {

public:
  RobotReader(std::shared_ptr<Client> client, int tps) : FBReader(client, tps) {}

protected:
  void OnMessage_Node(const Generated::Node *data) override {
    Logger::log("Node Message", LogLevel::Level::INFO);
    if (!data) {
      Logger::log("Node data is null", LogLevel::Level::ERROR);
      return;
    }

    // check if the id matches
    if (data->id() != STATE.read().id) {
      Logger::log("Filtered ID: " + std::to_string(data->id()) + " (" + std::to_string(STATE.read().id) + ")", LogLevel::Level::INFO);
      return;
    } else {
      Logger::log("ID Matched: " + std::to_string(data->id()), LogLevel::Level::INFO);
    }

    // get the position and rotation
    const auto pos = data->position();
    const auto rot = data->rotation();

    Logger::log("New Position: (" + std::to_string(pos->x()) + ", " + std::to_string(pos->y()) + ", " + std::to_string(pos->z()) + ")", LogLevel::Level::INFO);
    Logger::log("New Rotation: (" + std::to_string(rot->x()) + ", " + std::to_string(rot->y()) + ", " + std::to_string(rot->z()) + ", " + std::to_string(rot->w()) + ")", LogLevel::Level::INFO);

    // update the state
    std::lock_guard<std::mutex> lock(STATE.mtx);
    Logger::log("Inside Lock: updating position", LogLevel::Level::INFO);
    STATE.inner.current_pos = HIVE::Commons::Math::Vec::Vec3(pos);
    STATE.inner.current_rot = HIVE::Commons::Math::Vec::Vec4(rot);
  }

  void OnCommand_MoveTo(const Generated::MoveTo *data) override {
    Logger::log("MoveTo Command", LogLevel::Level::INFO);

    if (!data) {
      Logger::log("MoveTo data is null", LogLevel::Level::ERROR);
      return;
    }

    // get the destination
    const auto moveto = data->destination();

    // update the state
    std::lock_guard<std::mutex> lock(STATE.mtx);
    Logger::log("Inside Lock: Updating State From MoveTo", LogLevel::Level::INFO);

    STATE.inner.aligned = false;
    STATE.inner.target_completed = false;
    STATE.inner.target_pos = HIVE::Commons::Math::Vec::Vec3(moveto);
  }

  void OnCommand_Sleep(const Generated::Sleep *data) override {
    Logger::log("Sleep Command", LogLevel::Level::INFO);

    if (!data) {
      Logger::log("Sleep data is null", LogLevel::Level::ERROR);
      return;
    }

    // get the sleep duration
    const auto sleep = data->duration();

    // update the state
    std::lock_guard<std::mutex> lock(STATE.mtx);
    Logger::log("Inside Lock: Updating State From Sleep", LogLevel::Level::INFO);

    STATE.inner.sleep = data->sleep();
    STATE.inner.sleep = (long)(sleep * 1000);
  }
};

} // namespace Orchestrator
} // namespace HIVE

#endif // H_ROBOTREADER