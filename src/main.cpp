#include <chrono>
#include <stdio.h>
#include <thread>
#include <wiringPi.h> // Include WiringPi library!

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

int main(void) {
  // uses BCM numbering of the GPIOs and directly accesses the GPIO registers.
  wiringPiSetupGpio();

  // // pin mode ..(INPUT, OUTPUT, PWM_OUTPUT, GPIO_CLOCK)
  // // set pin 17 to input
  // pinMode(17, OUTPUT);

  // // pull up/down mode (PUD_OFF, PUD_UP, PUD_DOWN) => down
  // pullUpDnControl(17, PUD_DOWN);

  // // get state of pin 17
  // int value = digitalRead(17);

  // if (HIGH == value) {
  //   // your code
  // }

  pinMode(17, OUTPUT);
  pinMode(27, OUTPUT);
  pinMode(16, OUTPUT);
  pinMode(26, OUTPUT);

  for (;;) {
    blink(17);
    blink(27);
    blink(16);
    blink(26);
  }
  return 0;
}