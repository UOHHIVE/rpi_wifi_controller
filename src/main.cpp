#include "zumo.h"
#include <chrono>
#include <stdio.h>
#include <thread>
#include <wiringPi.h> // Include WiringPi library!

// URL:
// https://stackoverflow.com/questions/158585/how-do-you-add-a-timed-delay-to-a-c-program
using namespace std::this_thread;     // sleep_for, sleep_until
using namespace std::chrono_literals; // ns, us, ms, s, h, etc.
using std::chrono::system_clock;

int main(void) {

  zumo_utils::setup();

  zumo_utils::block();
  sleep_for(2s);

  zumo_utils::clear();
  sleep_for(0.5s);
  zumo_utils::block();

  sleep_for(2s);
  zumo_utils::clear();
  sleep_for(1s);

  digitalWrite(SAFETY, HIGH);

  // blink lights in sequence
  for (;;) {

    zumo_utils::clear();

    zumo_movement::start();
    zumo_utils::clear();

    zumo_movement::stop();
    zumo_utils::clear();

    zumo_movement::forward();
    zumo_movement::start();
    zumo_utils::clear();

    zumo_movement::turn_left();
    zumo_movement::start();
    zumo_utils::clear();

    zumo_movement::turn_right();
    zumo_movement::start();
    zumo_utils::clear();
  }
  return 0;
}