#include "main.hpp"

Lock<BotState> STATE;

int main(void) {

  logging::log("Startup", TESTING);

  // TODO: change this path later...
  dotenv::DotEnv::load("../.env");

  logging::log("Loaded EnvFile", TESTING);

  std::thread p_listener(tcp_listener);
  logging::log("Spawned Listener", TESTING);

  std::thread p_bot(bot_logic);
  logging::log("Spawned Bot Logic", TESTING);

  p_bot.join();
  p_listener.join();

  return 0;
}