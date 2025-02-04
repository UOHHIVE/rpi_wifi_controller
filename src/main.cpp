#include "main.hpp"
#include "commons/src/dotenv/dotenv.hpp"
#include "commons/src/logging/logging.hpp"
#include "commons/src/zumo/zumo.hpp"

#include <iostream>
#include <sstream>
#include <string>
#include <thread>

utils::Lock<BotState> STATE;
// netcode::Socket SOCK;

void setup() {

  zumo_utils::setup();

  string name = dotenv::DotEnv::get("BOT_NAME");
  string id_str = dotenv::DotEnv::get("ID_OVERRIDE");
  logging::log(LOG_ENABLED, "Read EnVars", LOG_LEVEL, 1);

  logging::log(LOG_ENABLED, "Name Aquired: `" + name + "`", LOG_LEVEL, 1);
  logging::log(LOG_ENABLED, "ID STR: `" + id_str + "`", LOG_LEVEL, 1);

  uint64_t id = std::stoull(id_str, nullptr, 16); // Convert hex string to decimal

  logging::log(LOG_ENABLED, "Saving ID...", LOG_LEVEL, 1);
  std::lock_guard<std::mutex> lock(STATE.mtx);
  STATE.inner.id = id;
  STATE.inner.name = name;
  STATE.inner.aligned = false;
  STATE.inner.target_completed = true;
}

void test_logging() {
  logging::log(LOG_ENABLED, "Test Log", LOG_LEVEL, 0);
  logging::log(LOG_ENABLED, "Test Log", LOG_LEVEL, 1);
  logging::log(LOG_ENABLED, "Test Log", LOG_LEVEL, 2);
  logging::log(LOG_ENABLED, "Test Log", LOG_LEVEL, 3);
  logging::log(LOG_ENABLED, "Test Log", LOG_LEVEL, 4);
  logging::log(LOG_ENABLED, "Test Log", LOG_LEVEL, 5);
  logging::log(LOG_ENABLED, "Test Log", LOG_LEVEL, 6);
}

int main(int argc, char *argv[]) {

  test_logging();

  logging::log(LOG_ENABLED, "Startup", LOG_LEVEL, 0);

  string config_path = "./config.env";

  if (argc > 1) {
    config_path = argv[1];
  }

  logging::log(LOG_ENABLED, "Loading Config: " + config_path, LOG_LEVEL, 0);

  // load envfile
  // TODO: change this path later...
  dotenv::DotEnv::load(config_path);
  logging::log(LOG_ENABLED, "Loaded Envfile", LOG_LEVEL, 0);

  setup();

  logging::log(LOG_ENABLED, "Setup Complete", LOG_LEVEL, 0);

  // connect to server

  // spawn threads
  std::thread p_listener(tcp_listener);
  logging::log(LOG_ENABLED, "Spawned Listener", LOG_LEVEL, 0);

  std::thread p_bot(bot_logic);
  logging::log(LOG_ENABLED, "Spawned Bot Logic", LOG_LEVEL, 0);

  // joins threads
  p_bot.join();
  p_listener.join();

  logging::log(LOG_ENABLED, "Shutdown...", LOG_LEVEL, 0);

  return 0;
}