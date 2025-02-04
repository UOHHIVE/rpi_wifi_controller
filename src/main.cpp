#include "main.hpp"
#include "commons/src/dotenv/dotenv.hpp"
#include "commons/src/logging/logging.hpp"
#include "commons/src/zumo/zumo.hpp"

#include <string>
#include <thread>

utils::Lock<BotState> STATE;
// netcode::Socket SOCK;

inline void setup() {
  const std::string log_name = "main.cpp::setup";

  // setup zumo config
  zumo_utils::setup();

  // pull bot name and id from env
  std::string name = dotenv::DotEnv::get("BOT_NAME");
  std::string id_str = dotenv::DotEnv::get("ID_OVERRIDE");
  logging::log(LOG_ENABLED, "Read EnVars", LOG_LEVEL, 2, log_name);
  logging::log(LOG_ENABLED, "Name Aquired: `" + name + "`", LOG_LEVEL, 3, log_name);
  logging::log(LOG_ENABLED, "ID STR: `" + id_str + "`", LOG_LEVEL, 3, log_name);

  // convert id hex string to decimal
  uint64_t id = std::stoull(id_str, nullptr, 16); // Convert hex string to decimal
  logging::log(LOG_ENABLED, "Saving ID...", LOG_LEVEL, 3, log_name);

  // save id and name to state
  std::lock_guard<std::mutex> lock(STATE.mtx);
  STATE.inner.id = id;
  STATE.inner.name = name;
  STATE.inner.aligned = false;
  STATE.inner.target_completed = true;
}

int main(int argc, char *argv[]) {
  const std::string log_name = "main.cpp::main";
  logging::log(LOG_ENABLED, "Startup", LOG_LEVEL, 0, log_name);

  // load envfile, default path is ./config.env
  std::string config_path = "./config.env";

  // if arg 1 is provided, use that as the path
  if (argc > 1) {
    config_path = argv[1];
  }

  logging::log(LOG_ENABLED, "Loading Config: " + config_path, LOG_LEVEL, 2, log_name);

  // load envfile into dotenv
  dotenv::DotEnv::load(config_path);
  logging::log(LOG_ENABLED, "Loaded Envfile", LOG_LEVEL, 2, log_name);

  // setup bot
  setup();
  logging::log(LOG_ENABLED, "Setup Complete", LOG_LEVEL, 2, log_name);

  // spawn tcp listener
  std::thread p_listener(tcp_listener);
  logging::log(LOG_ENABLED, "Spawned Listener", LOG_LEVEL, 2, log_name);

  // spawn bot logic
  std::thread p_bot(bot_logic);
  logging::log(LOG_ENABLED, "Spawned Bot Logic", LOG_LEVEL, 2, log_name);

  // joins threads
  p_bot.join();
  p_listener.join();

  logging::log(LOG_ENABLED, "Shutdown...", LOG_LEVEL, 0, log_name);
  return 0;
}