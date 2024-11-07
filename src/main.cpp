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

void blink(int pin) {
  printf("blinking %d", pin);
  digitalWrite(pin, HIGH);
  delay(400);
  digitalWrite(pin, LOW);
  delay(100);
}

void start() { digitalWrite(HBREAK, HIGH); }

void stop() { digitalWrite(HBREAK, LOW); }

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

void clear() {
  sleep_for(0.8s);
  digitalWrite(TRACK_L, LOW);
  digitalWrite(TRACK_R, LOW);
  digitalWrite(HBREAK, LOW);
  // digitalWrite(SAFETY, LOW);
  sleep_for(0.2s);
}

int main(void) {

  // uses BCM numbering of the GPIOs and directly accesses the GPIO registers.
  wiringPiSetupGpio();

  // Initialise pins
  pinMode(17, OUTPUT);
  pinMode(27, OUTPUT);
  pinMode(16, OUTPUT);
  pinMode(26, OUTPUT);

  digitalWrite(17, HIGH);
  digitalWrite(27, HIGH);
  digitalWrite(16, HIGH);
  digitalWrite(26, HIGH);

  sleep_for(2s);

  digitalWrite(17, LOW);
  digitalWrite(27, LOW);
  digitalWrite(16, LOW);
  digitalWrite(26, LOW);

  digitalWrite(SAFETY, HIGH);

  // blink lights in sequence
  for (;;) {
    // blink(17);
    // blink(27);
    // blink(16);
    // blink(26);

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
  }
  return 0;
}