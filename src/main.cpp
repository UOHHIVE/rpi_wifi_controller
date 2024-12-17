#include "main.hpp"
#include "commons/src/dotenv/dotenv.hpp"
#include "commons/src/logging/logging.hpp"

#include <string>
#include <thread>

utils::Lock<BotState> STATE;
// netcode::Socket SOCK;

int main(void) {

  logging::log(LOG_ENABLED, "Startup");

  // load envfile
  // TODO: change this path later...
  dotenv::DotEnv::load("../.env");
  logging::log(LOG_ENABLED, "Loaded EnvFile");

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