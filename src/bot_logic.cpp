#include "commons/src/logging/logger.hpp"
#include "commons/src/zumo/zumo.hpp"
#include "main.hpp"

#include "robot.hpp"
#include <string>
#include <thread>

using namespace HIVE::Commons::Math;
using namespace HIVE::Commons::Zumo;
using namespace HIVE::Commons::Logging;
using namespace HIVE::Commons::Utils;

using namespace std::chrono_literals;

extern void bot_logic() {
  const std::string log_name = "bot_logic.cpp::bot_logic";

  Logger::log("Waiting for Connection...", LogLevel::Level::INFO);

  // while not connected, sleep
  while (!STATE.read().connected) {
    std::this_thread::sleep_for(5s);
  }

  // make zumo safe
  HIVE::Commons::Zumo::Utils::safe();
  Logger::log("Connected to Server, starting ticking", LogLevel::Level::INFO);

  // utils::tick(tick_bot, MSPT, TICK);
  // spawn the bot logic
  Robot bot_logic = Robot(TPS);

  Logger::log("Bot Logic Stopped", LogLevel::Level::INFO);
}
