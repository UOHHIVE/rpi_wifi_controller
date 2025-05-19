
#include "commons/src/flatbuffers/generated/commons_generated.h"
#include "commons/src/logging/logger.hpp"
#include "commons/src/netcode/reader.hpp"
#include "main.hpp"

using namespace HIVE::Commons::Netcode;
using namespace HIVE::Commons::Logging;

namespace HIVE {
namespace Orchestrator {

class Reader : public HIVE::Commons::Netcode::Reader {

public:
  void OnRead(const std::string &data) override {

    if (data.empty()) {
      Logger::log("No data received", LogLevel::Level::ERROR);
      return;
    }

    Logger::log("Message Size: " + std::to_string(data.size()), LogLevel::Level::INFO);
    Logger::log("Message: " + data, LogLevel::Level::INFO);

    // get the state from the message
    const HIVE::Commons::Flatbuffers::Generated::State *s = HIVE::Commons::Flatbuffers::Generated::GetState(data.c_str());

    // if state is null, return
    if (!s) {
      Logger::log("No State in message", LogLevel::Level::ERROR);
      return; // TODO: could cause issues in future
    }

    // get the payload from the state
    const flatbuffers::Vector<flatbuffers::Offset<HIVE::Commons::Flatbuffers::Generated::Payload>> *p = s->payload();

    // if payload is null, return
    if (!p) {
      Logger::log("No Payload in state", LogLevel::Level::ERROR);
      return; // TODO: could cause issues in future
    }

    Logger::log("Recieved Message", LogLevel::Level::INFO);

    // for each entity in the payload
    for (const auto &e : *p) {

      // extract the entity
      const HIVE::Commons::Flatbuffers::Generated::Entity *entity = e->data_nested_root();
      Logger::log("Extracted entity", LogLevel::Level::INFO);

      // switch over the entity type
      switch (entity->entity_type()) {
      case HIVE::Commons::Flatbuffers::Generated::EntityUnion_Node: {
        Logger::log("Decoding Node", LogLevel::Level::INFO);

        // extract the node
        const auto node = entity->entity_as_Node();
        Logger::log("extracted node", LogLevel::Level::INFO);

        // check if the id matches
        if (node->id() != STATE.read().id) {
          Logger::log("Filtered ID: " + std::to_string(node->id()) + " (" + std::to_string(STATE.read().id) + ")", LogLevel::Level::INFO);
          break;
        } else {
          Logger::log("ID Matched: " + std::to_string(node->id()), LogLevel::Level::INFO);
        }

        // get the position and rotation
        const auto pos = node->position();
        const auto rot = node->rotation();

        // update the state
        std::lock_guard<std::mutex> lock(STATE.mtx);
        STATE.inner.current_pos = HIVE::Commons::Math::Vec::Vec3(pos);
        STATE.inner.current_rot = HIVE::Commons::Math::Vec::Vec4(rot);
        break;
      }
      case HIVE::Commons::Flatbuffers::Generated::EntityUnion_Command: {
        Logger::log("Decoding Command", LogLevel::Level::INFO);
        const HIVE::Commons::Flatbuffers::Generated::Command *command = entity->entity_as_Command();

        // TODO: add logging here

        switch (command->command_type()) {
        case HIVE::Commons::Flatbuffers::Generated::CommandUnion_MoveTo: {
          Logger::log("MoveTo Command", LogLevel::Level::INFO);

          // get the destination
          const auto moveto = command->command_as_MoveTo();
          Logger::log("extracted destination", LogLevel::Level::INFO);

          // update the state
          std::lock_guard<std::mutex> lock(STATE.mtx);
          STATE.inner.aligned = false;
          STATE.inner.target_completed = false;
          STATE.inner.target_pos = HIVE::Commons::Math::Vec::Vec3(moveto->destination());
          break;
        }
        case HIVE::Commons::Flatbuffers::Generated::CommandUnion_Sleep: {
          Logger::log("Sleep Command", LogLevel::Level::INFO);

          // get the sleep duration
          const auto sleep = command->command_as_Sleep();
          Logger::log("Sleep Duration: " + std::to_string(sleep->duration()), LogLevel::Level::INFO);

          // update the state
          std::lock_guard<std::mutex> lock(STATE.mtx);
          STATE.inner.sleep = sleep->sleep();
          STATE.inner.sleep = (long)(sleep->duration() * 1000);
          break;
        }
        case HIVE::Commons::Flatbuffers::Generated::CommandUnion_Owner:
        case HIVE::Commons::Flatbuffers::Generated::CommandUnion_NONE:
          Logger::log("Invalid Command", LogLevel::Level::ERROR);
          break;
        }

        break;
      }
      case HIVE::Commons::Flatbuffers::Generated::EntityUnion_Robot:
      case HIVE::Commons::Flatbuffers::Generated::EntityUnion_Generic:
      case HIVE::Commons::Flatbuffers::Generated::EntityUnion_Geometry:
      case HIVE::Commons::Flatbuffers::Generated::EntityUnion_Headset:
      case HIVE::Commons::Flatbuffers::Generated::EntityUnion_Observer:
      case HIVE::Commons::Flatbuffers::Generated::EntityUnion_Presenter:
      case HIVE::Commons::Flatbuffers::Generated::EntityUnion_NONE:
        Logger::log("Invalid Entity Type", LogLevel::Level::ERROR);
        break;
      }

      Logger::log("Finished parsing entity...", LogLevel::Level::INFO);
    }
  }
};

class RobotReader : public FBReader {

  void OnMessage_Node(const Generated::Node *data) override {

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

    // update the state
    std::lock_guard<std::mutex> lock(STATE.mtx);
    STATE.inner.current_pos = HIVE::Commons::Math::Vec::Vec3(pos);
    STATE.inner.current_rot = HIVE::Commons::Math::Vec::Vec4(rot);
  }

  void OnCommand_MoveTo(const Generated::MoveTo *data) override {
    if (!data) {
      Logger::log("MoveTo data is null", LogLevel::Level::ERROR);
      return;
    }

    // get the destination
    const auto moveto = data->destination();

    // update the state
    std::lock_guard<std::mutex> lock(STATE.mtx);
    STATE.inner.aligned = false;
    STATE.inner.target_completed = false;
    STATE.inner.target_pos = HIVE::Commons::Math::Vec::Vec3(moveto);
  }

  void OnCommand_Sleep(const Generated::Sleep *data) override {
    if (!data) {
      Logger::log("Sleep data is null", LogLevel::Level::ERROR);
      return;
    }

    // get the sleep duration
    const auto sleep = data->duration();

    // update the state
    std::lock_guard<std::mutex> lock(STATE.mtx);
    STATE.inner.sleep = data->sleep();
    STATE.inner.sleep = (long)(sleep * 1000);
  }
};

} // namespace Orchestrator
} // namespace HIVE