#include <wiringPi.h> // Include WiringPi library!

int main(void) {
  // uses BCM numbering of the GPIOs and directly accesses the GPIO registers.
  wiringPiSetupGpio();

  // pin mode ..(INPUT, OUTPUT, PWM_OUTPUT, GPIO_CLOCK)
  // set pin 17 to input
  pinMode(17, INPUT);

  // pull up/down mode (PUD_OFF, PUD_UP, PUD_DOWN) => down
  pullUpDnControl(17, PUD_DOWN);

  // get state of pin 17
  int value = digitalRead(17);

  if (HIGH == value) {
    // your code
  }
}