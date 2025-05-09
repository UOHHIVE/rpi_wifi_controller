#include "commons/src/dotenv/dotenv.hpp"
#include "commons/src/logging/logger.hpp"
#include "commons/src/math/vec/vec4.hpp"
#include "commons/src/netcode/client.hpp"
#include "commons/src/utils/misc.hpp"
#include "main.hpp"
#include "reader.hpp"

#include <string>

using namespace HIVE::Commons::Netcode;
using namespace HIVE::Commons::Flatbuffers;
using namespace HIVE::Commons::Logging;
using namespace HIVE::Commons::Utils;
using namespace HIVE::Commons::Math;
using namespace HIVE::Commons::Dotenv;

// Function to set up the TCP connection
inline void tcp_setup(Client &client) {
  const std::string log_name = "tcp_listener.cpp::tcp_setup";

  // good luck if this is yr first exposure to flatbuffers lmao

  // create subscriber
  uint16_t sub = 0;
  sub = HIVE::Commons::Utils::encodeSubscriptionType(Generated::SubscriptionType_Own, sub);

  // initise flatbuf builders
  flatbuffers::FlatBufferBuilder fbb1;
  flatbuffers::FlatBufferBuilder fbb2;

  // create robot
  const auto fb_name = fbb1.CreateString(STATE.read().name);
  const auto robot = HIVE::Commons::Flatbuffers::Generated::CreateRobot(fbb1, STATE.read().id, fb_name, sub, HIVE::Commons::Flatbuffers::Generated::SubscriptionRate_Half);
  const auto entity = HIVE::Commons::Flatbuffers::Generated::CreateEntity(fbb1, HIVE::Commons::Flatbuffers::Generated::EntityUnion_Robot, robot.Union());

  // finish entity
  fbb1.Finish(entity);
  fbb1.ForceVectorAlignment(fbb1.GetSize(), sizeof(uint8_t), fbb1.GetBufferMinAlignment());
  Logger::log("Entity Built", LogLevel::Level::INFO);

  // Build the Payload which is to be used in the State as a payload vector
  const auto entityVec = fbb2.CreateVector(fbb1.GetBufferPointer(), fbb1.GetSize());
  const auto payload = HIVE::Commons::Flatbuffers::Generated::CreatePayload(fbb2, entityVec);
  Logger::log("Payload Built", LogLevel::Level::INFO);

  // Build the State
  std::vector<flatbuffers::Offset<HIVE::Commons::Flatbuffers::Generated::Payload>> payloadVector;
  payloadVector.push_back(payload);
  const auto payloads = fbb2.CreateVector(payloadVector);
  const auto state = HIVE::Commons::Flatbuffers::Generated::CreateState(fbb2, payloads);
  fbb2.FinishSizePrefixed(state);
  Logger::log("State Built", LogLevel::Level::INFO);

  // send the connection message
  client.Socket::send_data(reinterpret_cast<char *>(fbb2.GetBufferPointer()), fbb2.GetSize());
  Logger::log("Connection Message Sent", LogLevel::Level::INFO);

  // flag as connected
  std::lock_guard<std::mutex> lock(STATE.mtx);
  STATE.inner.connected = true;
}

// TCP listener that gets spawned
extern void tcp_listener() {
  const std::string log_name = "tcp_listener.cpp::tcp_listener";
  Logger::log("Starting Listener...", LogLevel::Level::INFO);

  //! If any of these are blank, code may crash, needs to be fixed
  string dc_address = DotEnv::get("DC_ADDRESS");
  string dc_port = DotEnv::get("DC_PORT");
  Logger::log("Read EnVars", LogLevel::Level::INFO);
  Logger::log("Connecting to: " + dc_address + ":" + dc_port, LogLevel::Level::INFO);

  // Create the socket
  Client client = Client(dc_address.data(), std::stoi(dc_port));
  Logger::log("Socket Created", LogLevel::Level::INFO);

  // Set up the TCP connection
  tcp_setup(client);
  Logger::log("TCP Setup Complete", LogLevel::Level::INFO);
}
