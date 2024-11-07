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

namespace test {

void testfn() {
  digitalWrite(TRACK_L, HIGH);
  digitalWrite(TRACK_R, HIGH);
  digitalWrite(SAFETY, HIGH);
  digitalWrite(HBREAK, HIGH);

  sleep_for(2s);

  digitalWrite(TRACK_L, LOW);
  digitalWrite(TRACK_R, LOW);
  digitalWrite(SAFETY, LOW);
  digitalWrite(HBREAK, LOW);
}

} // namespace test

namespace zumo_movement {

void start() { digitalWrite(HBREAK, HIGH); }

void stop() { digitalWrite(HBREAK, LOW); }

} // namespace zumo_movement

namespace zumo_utils {

void setup() {
  // uses BCM numbering of the GPIOs and directly accesses the GPIO registers.
  wiringPiSetupGpio();

  // Initialise pins
  pinMode(TRACK_L, OUTPUT);
  pinMode(TRACK_R, OUTPUT);
  pinMode(SAFETY, OUTPUT);
  pinMode(HBREAK, OUTPUT);
}

void safe() { digitalWrite(SAFETY, HIGH); }

void unsafe() { digitalWrite(SAFETY, LOW); }

void clear() {
  sleep_for(0.8s);
  digitalWrite(TRACK_L, LOW);
  digitalWrite(TRACK_R, LOW);
  digitalWrite(HBREAK, LOW);
  // digitalWrite(SAFETY, LOW);
  sleep_for(0.2s);
}

} // namespace zumo_utils