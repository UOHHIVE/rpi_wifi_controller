#include "commons/src/dotenv/dotenv.hpp"
#include "commons/src/logging/logger.hpp"
#include "commons/src/netcode/client.hpp"
#include "commons/src/utils/misc.hpp"
#include "commons/src/zumo/zumo.hpp"
#include "controller.hpp"
#include "reader.hpp"
#include "state.hpp"

#include <string>

HIVE::Commons::Utils::Lock<BotState> STATE;

using namespace HIVE::Commons::Logging;
using namespace HIVE::Commons::Zumo;
using namespace HIVE::Commons::Dotenv;
using namespace HIVE::Commons::Math;
using namespace HIVE::Commons::Utils;
using namespace HIVE::Orchestrator;

using namespace std::chrono_literals;

// good luck if this is yr first exposure to flatbuffers lmao

// set up state obj
inline void setup() {

  // setup zumo config
  HIVE::Commons::Zumo::GPIO::setup();

  // pull bot name and id from env

  std::string name = DotEnv::get("BOT_NAME");
  std::string id_str = DotEnv::get("ID_OVERRIDE");

  Logger::log("Bot Name: " + name, LogLevel::Level::INFO);
  Logger::log("Bot ID: " + id_str, LogLevel::Level::INFO);

  // convert id hex string to decimal
  uint64_t id = std::stoull(id_str, nullptr, 16); // Convert hex string to decimal
  Logger::log("Bot ID: " + std::to_string(id), LogLevel::Level::INFO);

  // save id and name to state
  std::lock_guard<std::mutex> lock(STATE.mtx);
  Logger::log("Inside Lock: Setting Bot ID and Name", LogLevel::Level::INFO);
  STATE.inner.id = id;
  STATE.inner.name = name;
  STATE.inner.aligned = false;
  STATE.inner.target_completed = true;
}

// Function to set up the TCP connection
inline void tcp_setup(Client &client) {

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
  Logger::log("Inside Lock: Setting Bot Connected", LogLevel::Level::INFO);
  STATE.inner.connected = true;
}

int main(int argc, char *argv[]) {
  Logger::set_severity(5);
  Logger::log("Starting Bot", LogLevel::Level::INFO);

  // load envfile, default path is ./config.env
  std::string config_path = "./config.env";

  // if arg 1 is provided, use that as the path
  if (argc > 1) {
    config_path = argv[1];
  }

  Logger::log("Loading Config: " + config_path, LogLevel::Level::INFO);

  // load envfile into dotenv
  DotEnv::load(config_path);
  Logger::log("Loaded Envfile", LogLevel::Level::INFO);

  // setup bot
  setup();
  Logger::log("Setup Complete", LogLevel::Level::INFO);

  // make zumo safe
  HIVE::Commons::Zumo::Utils::safe();
  Logger::log("Bot Marked Safe", LogLevel::Level::INFO);

  // spawn the bot logic
  RobotController c = RobotController(TPS);
  Logger::log("Spawned Bot Logic", LogLevel::Level::INFO);

  // start the bot logic
  c.Start();
  Logger::log("Bot Logic Started", LogLevel::Level::INFO);

  Logger::log("Starting Listener...", LogLevel::Level::INFO);

  //! If any of these are blank, code may crash, needs to be fixed
  string dc_address = DotEnv::get("DC_ADDRESS");
  string dc_port = DotEnv::get("DC_PORT");
  Logger::log("Read EnVars", LogLevel::Level::INFO);
  Logger::log("Connecting to: " + dc_address + ":" + dc_port, LogLevel::Level::INFO);

  // Create the socket as a shared pointer
  auto client = std::make_shared<Client>(dc_address.data(), std::stoi(dc_port));
  Logger::log("Socket Created", LogLevel::Level::INFO);

  // Set up the TCP connection
  tcp_setup(*client);
  Logger::log("TCP Setup", LogLevel::Level::INFO);

  // Create the reader
  RobotReader r = RobotReader(client, TPS);
  Logger::log("Reader Created", LogLevel::Level::INFO);

  // Start the reader
  r.Start();
  Logger::log("TCP Setup Complete", LogLevel::Level::INFO);

  // wait for bot logic to finish
  r._thread.join();
  c._thread.join();

  Logger::log("Shutting Down...", LogLevel::Level::INFO);
  return 0;
}