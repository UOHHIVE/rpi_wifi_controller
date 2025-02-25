#include "commons/src/dotenv/dotenv.hpp"
#include "commons/src/logging/elogtype.hpp"
#include "commons/src/logging/logging.hpp"
#include "commons/src/math/vec/vec3.hpp"
#include "commons/src/math/vec/vec4.hpp"
#include "commons/src/netcode/netcode.hpp"
#include "commons/src/utils/misc.hpp"
#include "commons/src/utils/tick.hpp"
#include "main.hpp"

#include <string>

using namespace logging;

// Function to be executed every tick of the TCP listener
void tcp_tick(netcode::Socket sock) {
  const std::string log_name = "tcp_listener.cpp::tcp_tick";

  // read data from the socket
  string message;
  sock.read_data(message);

  // if message is empty, return
  if (message.size() > 0) {
    logging::log(LOG_ENABLED, "Message: " + message, LOG_LEVEL, 4, LogType::INFO, log_name);
    logging::log(LOG_ENABLED, "Message Size: " + std::to_string(message.size()), LOG_LEVEL, 4, LogType::INFO, log_name);

    // get the state from the message
    const HiveCommon::State *s = HiveCommon::GetState(message.c_str());

    // if state is null, return
    if (!s) {
      logging::log(LOG_ENABLED, "No State in message", LOG_LEVEL, 1, LogType::WARN, log_name);
      return; // TODO: could cause issues in future
    }

    // get the payload from the state
    const flatbuffers::Vector<flatbuffers::Offset<HiveCommon::Payload>> *p = s->payload();

    // if payload is null, return
    if (!p) {
      logging::log(LOG_ENABLED, "No Payload in state", LOG_LEVEL, 1, LogType::WARN, log_name);
      return; // TODO: could cause issues in future
    }

    logging::log(LOG_ENABLED, "Recieved Message", LOG_LEVEL, 2, log_name);

    // for each entity in the payload
    for (const auto &e : *p) {

      // extract the entity
      const HiveCommon::Entity *entity = e->data_nested_root();
      logging::log(LOG_ENABLED, "Extracted entity", LOG_LEVEL, 3, log_name);

      // switch over the entity type
      switch (entity->entity_type()) {
      case HiveCommon::EntityUnion_Node: {
        logging::log(LOG_ENABLED, "Decoding Node", LOG_LEVEL, 3, log_name);

        // extract the node
        const auto node = entity->entity_as_Node();
        logging::log(LOG_ENABLED, "extracted node", LOG_LEVEL, 4, log_name);

        // check if the id matches
        if (node->id() != STATE.read().id) {
          logging::log(LOG_ENABLED, "Filtered ID: " + std::to_string(node->id()) + " (" + std::to_string(STATE.read().id) + ")", LOG_LEVEL, 2, LogType::INFO, log_name);
          break;
        } else {
          logging::log(LOG_ENABLED, "ID Matched: " + std::to_string(node->id()), LOG_LEVEL, 2, LogType::INFO, log_name);
        }

        // get the position and rotation
        const auto pos = node->position();
        const auto rot = node->rotation();

        // update the state
        std::lock_guard<std::mutex> lock(STATE.mtx);
        STATE.inner.current_pos = hive::math::vec::Vec3(pos);
        STATE.inner.current_rot = hive::math::vec::Vec4(rot);
        break;
      }
      case HiveCommon::EntityUnion_Command: {
        logging::log(LOG_ENABLED, "Decoding Command", LOG_LEVEL, 3, log_name);
        const HiveCommon::Command *command = entity->entity_as_Command();

        // TODO: add logging here

        switch (command->command_type()) {
        case HiveCommon::CommandUnion_MoveTo: {
          logging::log(LOG_ENABLED, "MoveTo Command", LOG_LEVEL, 3, log_name);

          // get the destination
          const auto moveto = command->command_as_MoveTo();
          logging::log(LOG_ENABLED, "extracted destination", LOG_LEVEL, 4, log_name);

          // update the state
          std::lock_guard<std::mutex> lock(STATE.mtx);
          STATE.inner.aligned = false;
          STATE.inner.target_completed = false;
          STATE.inner.target_pos = hive::math::vec::Vec3(moveto->destination());
          break;
        }
        case HiveCommon::CommandUnion_Sleep: {
          logging::log(LOG_ENABLED, "Sleep Command", LOG_LEVEL, 3, log_name);

          // get the sleep duration
          const auto sleep = command->command_as_Sleep();
          logging::log(LOG_ENABLED, "extracted sleep", LOG_LEVEL, 4, log_name);

          // update the state
          std::lock_guard<std::mutex> lock(STATE.mtx);
          STATE.inner.sleep = sleep->sleep();
          STATE.inner.sleep = (long)(sleep->duration() * 1000);
          break;
        }
        case HiveCommon::CommandUnion_Owner:
        case HiveCommon::CommandUnion_NONE:
          logging::log(LOG_ENABLED, "Invalid Command", LOG_LEVEL, 2, log_name);
          break;
        }

        break;
      }
      case HiveCommon::EntityUnion_Robot:
      case HiveCommon::EntityUnion_Generic:
      case HiveCommon::EntityUnion_Geometry:
      case HiveCommon::EntityUnion_Headset:
      case HiveCommon::EntityUnion_Observer:
      case HiveCommon::EntityUnion_Presenter:
      case HiveCommon::EntityUnion_NONE:
        logging::log(LOG_ENABLED, "Invalid Entity Type", LOG_LEVEL, 2, log_name);
        break;
      }

      logging::log(LOG_ENABLED, "Finished parsing entity...", LOG_LEVEL, 4, "tcp_listener");
    }
  } else {
    logging::log(LOG_ENABLED, "Zero Bytes Read", LOG_LEVEL, 1, "tcp_listener");
  }
}

// Function to set up the TCP connection
inline void tcp_setup(netcode::Socket sock) {
  const std::string log_name = "tcp_listener.cpp::tcp_setup";

  // good luck if this is yr first exposure to flatbuffers lmao

  // create subscriber
  uint16_t sub = 0;
  sub = utils::misc::encodeSubscriptionType(HiveCommon::SubscriptionType_Own, sub);

  // initise flatbuf builders
  flatbuffers::FlatBufferBuilder fbb1;
  flatbuffers::FlatBufferBuilder fbb2;

  // create robot
  const auto fb_name = fbb1.CreateString(STATE.read().name);
  const auto robot = HiveCommon::CreateRobot(fbb1, STATE.read().id, fb_name, sub, HiveCommon::SubscriptionRate_Half);
  const auto entity = HiveCommon::CreateEntity(fbb1, HiveCommon::EntityUnion_Robot, robot.Union());

  // finish entity
  fbb1.Finish(entity);
  fbb1.ForceVectorAlignment(fbb1.GetSize(), sizeof(uint8_t), fbb1.GetBufferMinAlignment());
  logging::log(LOG_ENABLED, "Entity Built", LOG_LEVEL, 4, log_name);

  // Build the Payload which is to be used in the State as a payload vector
  const auto entityVec = fbb2.CreateVector(fbb1.GetBufferPointer(), fbb1.GetSize());
  const auto payload = HiveCommon::CreatePayload(fbb2, entityVec);
  logging::log(LOG_ENABLED, "Payload Built", LOG_LEVEL, 4, log_name);

  // Build the State
  std::vector<flatbuffers::Offset<HiveCommon::Payload>> payloadVector;
  payloadVector.push_back(payload);
  const auto payloads = fbb2.CreateVector(payloadVector);
  const auto state = HiveCommon::CreateState(fbb2, payloads);
  fbb2.FinishSizePrefixed(state);
  logging::log(LOG_ENABLED, "State Built", LOG_LEVEL, 4, log_name);

  // send the connection message
  sock.send_data(reinterpret_cast<char *>(fbb2.GetBufferPointer()), fbb2.GetSize());
  logging::log(LOG_ENABLED, "Connection Message Sent", LOG_LEVEL, 2, log_name);

  // flag as connected
  std::lock_guard<std::mutex> lock(STATE.mtx);
  STATE.inner.connected = true;
}

// TCP listener that gets spawned
extern void tcp_listener() {
  const std::string log_name = "tcp_listener.cpp::tcp_listener";
  logging::log(LOG_ENABLED, "Starting Listener...", LOG_LEVEL, 0, log_name);

  //! If any of these are blank, code may crash, needs to be fixed
  string dc_address = dotenv::DotEnv::get("DC_ADDRESS");
  string dc_port = dotenv::DotEnv::get("DC_PORT");
  logging::log(LOG_ENABLED, "Read EnVars", LOG_LEVEL, 3, log_name);
  logging::log(LOG_ENABLED, "Connecting to: " + dc_address + ":" + dc_port, LOG_LEVEL, 2, LogType::INFO, log_name);

  // Create the socket
  netcode::Socket sock = netcode::Socket(dc_address.data(), std::stoi(dc_port));
  logging::log(LOG_ENABLED, "socket created", LOG_LEVEL, 3, log_name);

  // Set up the TCP connection
  tcp_setup(sock);
  logging::log(LOG_ENABLED, "Connection established", LOG_LEVEL, 3, log_name);

  // Start ticking the TCP listener
  utils::tick(tcp_tick, 1000, TICK, sock);
  logging::log(LOG_ENABLED, "listener closed", LOG_LEVEL, 0, log_name);
}
