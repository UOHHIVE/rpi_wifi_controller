#include "zumo.h"
#include <chrono>
#include <stdio.h>
#include <thread>
#include <wiringPi.h> // Include WiringPi library!

#define TRACK_L 17 // Left Track pin
#define TRACK_R 27 // Right Track Pin
#define SAFETY 16  // Safety Pin
#define HBREAK 26  // Break Pin

// URL:
// https://stackoverflow.com/questions/158585/how-do-you-add-a-timed-delay-to-a-c-program
using namespace std::this_thread;     // sleep_for, sleep_until
using namespace std::chrono_literals; // ns, us, ms, s, h, etc.
using std::chrono::system_clock;

void forward() {
  digitalWrite(TRACK_L, HIGH);
  digitalWrite(TRACK_R, HIGH);
}

void turn_left() {
  digitalWrite(TRACK_L, HIGH);
  digitalWrite(TRACK_R, LOW);
}

void turn_right() {
  digitalWrite(TRACK_L, LOW);
  digitalWrite(TRACK_R, HIGH);
}

int main(void) {

  zumo_utils::setup();

  test::testfn();
  sleep_for(0.5s);
  test::testfn();
  sleep_for(2s);

  zumo_utils::safe();

  for (;;) {

    zumo_utils::clear();

    zumo_movement::start();
    zumo_utils::clear();

    zumo_movement::stop();
    zumo_utils::clear();

    forward();
    zumo_movement::start();
    zumo_utils::clear();

    turn_left();
    zumo_movement::start();
    zumo_utils::clear();

    turn_right();
    zumo_movement::start();
    zumo_utils::clear();
  }
  return 0;
}