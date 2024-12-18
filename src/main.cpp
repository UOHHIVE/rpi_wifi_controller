#include "main.hpp"
#include "commons/src/dotenv/dotenv.hpp"
#include "commons/src/logging/logging.hpp"

#include <string>
#include <thread>

utils::Lock<BotState> STATE;
// netcode::Socket SOCK;

void setup() {
  string name = dotenv::DotEnv::get("BOT_NAME");
  string id_str = dotenv::DotEnv::get("ID_OVERRIDE");

  logging::log(true, "Name Aquired: `" + name + "`");

  logging::log(true, "Read EnVars");
  logging::log(true, "ID STR: `" + id_str + "`");

  uint64_t id = std::stoll(id_str); // TODO: this borked when using hex

  logging::log(true, "Saving ID");
  std::lock_guard<std::mutex> lock(STATE.mtx);
  STATE.inner.id = id;
  STATE.inner.name = name;
}

int main(void) {

  logging::log(LOG_ENABLED, "Startup");

  // load envfile
  // TODO: change this path later...
  dotenv::DotEnv::load("../.env");
  logging::log(LOG_ENABLED, "Loaded EnvFile");

  setup();

  logging::log(true, "Setup Complete");

  // connect to server

  // spawn threads
  std::thread p_listener(tcp_listener);
  logging::log(LOG_ENABLED, "Spawned Listener");

  std::thread p_bot(bot_logic);
  logging::log(LOG_ENABLED, "Spawned Bot Logic");

  // joins threads
  p_bot.join();
  p_listener.join();

  return 0;
}