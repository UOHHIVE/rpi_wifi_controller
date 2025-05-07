#include "main.hpp"
#include "commons/src/dotenv/dotenv.hpp"
#include "commons/src/logging/logger.hpp"
#include "commons/src/zumo/zumo.hpp"

#include <string>
#include <thread>

HIVE::Commons::Utils::Lock<BotState> STATE;
// netcode::Socket SOCK;

using namespace HIVE::Commons::Logging;
using namespace HIVE::Commons::Zumo;
using namespace HIVE::Commons::Dotenv;

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
  STATE.inner.id = id;
  STATE.inner.name = name;
  STATE.inner.aligned = false;
  STATE.inner.target_completed = true;
}

int main(int argc, char *argv[]) {
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

  // spawn tcp listener
  std::thread p_listener(tcp_listener);
  Logger::log("Spawned TCP Listener", LogLevel::Level::INFO);

  // spawn bot logic
  std::thread p_bot(bot_logic);
  Logger::log("Spawned Bot Logic", LogLevel::Level::INFO);

  // joins threads
  p_bot.join();
  p_listener.join();

  Logger::log("Shutdown Complete", LogLevel::Level::INFO);
  return 0;
}