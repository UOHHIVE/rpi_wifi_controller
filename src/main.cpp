// #include "zumo.h"
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

// namespace zumo_movement {

void start() { digitalWrite(HBREAK, HIGH); }

void stop() { digitalWrite(HBREAK, LOW); }

void forward() {
  digitalWrite(TRACK_L, HIGH);
  digitalWrite(TRACK_R, HIGH);
}

void reverse() {
  digitalWrite(TRACK_L, LOW);
  digitalWrite(TRACK_R, LOW);
}

void turn_left() {
  digitalWrite(TRACK_L, HIGH);
  digitalWrite(TRACK_R, LOW);
}

void turn_right() {
  digitalWrite(TRACK_L, LOW);
  digitalWrite(TRACK_R, HIGH);
}

// } // namespace zumo_movement

// namespace zumo_utils {

void safe() { digitalWrite(SAFETY, HIGH); }

void unsafe() { digitalWrite(SAFETY, LOW); }

void setup() {
  wiringPiSetupGpio(); // uses BCM numbering, direct GPIO register access
  pinMode(TRACK_L, OUTPUT);
  pinMode(TRACK_R, OUTPUT);
  pinMode(SAFETY, OUTPUT);
  pinMode(HBREAK, OUTPUT);
}

void clear() {
  digitalWrite(TRACK_L, LOW);
  digitalWrite(TRACK_R, LOW);
  digitalWrite(SAFETY, LOW);
  digitalWrite(HBREAK, LOW);
}

void block() {
  digitalWrite(TRACK_L, HIGH);
  digitalWrite(TRACK_R, HIGH);
  digitalWrite(SAFETY, HIGH);
  digitalWrite(HBREAK, HIGH);
}

void blink(int pin) {
  // printf("blinking %d", pin);
  digitalWrite(pin, HIGH);
  delay(400);
  digitalWrite(pin, LOW);
  delay(100);
}

// } // namespace zumo_utils

int main(void) {

  // zumo_utils::setup();

  wiringPiSetupGpio(); // uses BCM numbering, direct GPIO register access
  pinMode(TRACK_L, OUTPUT);
  pinMode(TRACK_R, OUTPUT);
  pinMode(SAFETY, OUTPUT);
  pinMode(HBREAK, OUTPUT);

  printf("1");
  block();
  sleep_for(2s);

  printf("2");
  clear();
  sleep_for(0.5s);
  block();

  printf("3");
  sleep_for(2s);
  clear();
  sleep_for(1s);

  safe();

  // blink lights in sequence
  // for (;;) {

  clear();

  start();
  clear();

  stop();
  clear();

  forward();
  start();
  clear();

  turn_left();
  start();
  clear();

  turn_right();
  start();
  clear();
  // }
  return 0;
}