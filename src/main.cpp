#include "commons/src/zumo.h"
#include <chrono>
#include <stdio.h>
#include <sys/time.h>
#include <thread>

// URL:
// https://stackoverflow.com/questions/158585/how-do-you-add-a-timed-delay-to-a-c-program
using namespace std::this_thread;     // sleep_for, sleep_until
using namespace std::chrono_literals; // ns, us, ms, s, h, etc.
using std::chrono::system_clock;

#define TPS 120
#define MSPT 1000000 / (float)TPS

int main(void) {

  // TODO:
  // - init state rwlock
  // - init buffer
  // - spawn listener
  // - while true to move the bot

  auto p1 = std::chrono::system_clock::now().time_since_epoch();
  auto t1 = std::chrono::duration_cast<std::chrono::microseconds>(p1).count();
  auto t_target = t1 + MSPT;

  // auto p2 = std::chrono::system_clock::now().time_since_epoch();
  // auto t2 = std::chrono::duration_cast<std::chrono::microseconds>(p2).count();

  while (1) {

    p1 = std::chrono::system_clock::now().time_since_epoch();
    t1 = std::chrono::duration_cast<std::chrono::microseconds>(p1).count();
    t_target = t1 + MSPT;

    // TODO: read state
    // TODO: compute state
    // TODO: do movement stuff...

    // sleep_until(p1); // TODO: how do I get this working?
  }

  // zumo_utils::setup();

  // zumo_utils::blink();
  // sleep_for(0.5s);
  // zumo_utils::blink();
  // sleep_for(2s);

  // zumo_utils::safe();

  // for (;;) {

  //   zumo_utils::clear();

  //   zumo_movement::start();
  //   zumo_utils::clear();

  //   zumo_movement::stop();
  //   zumo_utils::clear();

  //   zumo_movement::forward();
  //   zumo_movement::start();
  //   zumo_utils::clear();

  //   zumo_movement::turn_left();
  //   zumo_movement::start();
  //   zumo_utils::clear();

  //   zumo_movement::turn_right();
  //   zumo_movement::start();
  //   zumo_utils::clear();
  // }
  return 0;
}