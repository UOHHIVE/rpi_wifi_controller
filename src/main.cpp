#include "main.hpp"
#include "commons/src/logging/logging.hpp"
#include "commons/src/dotenv/dotenv.hpp"
#include "commons/src/netcode/netcode.hpp"

#include <string>
#include <thread>

utils::Lock<BotState> STATE;

int main(void) {

  logging::log("Startup", TESTING);

  // load envfile
  // TODO: change this path later...
  dotenv::DotEnv::load("../.env");
  logging::log("Loaded EnvFile", TESTING);

  // spawn threads
  std::thread p_listener(tcp_listener);
  logging::log("Spawned Listener", TESTING);

  std::thread p_bot(bot_logic);
  logging::log("Spawned Bot Logic", TESTING);

  // joins threads
  p_bot.join();
  p_listener.join();

  return 0;
}