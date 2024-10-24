#include <stdio.h>
#include <chrono>
#include <thread>

// URL: https://stackoverflow.com/questions/158585/how-do-you-add-a-timed-delay-to-a-c-program
using namespace std::this_thread;     // sleep_for, sleep_until
using namespace std::chrono_literals; // ns, us, ms, s, h, etc.
using std::chrono::system_clock;

int main(void) {
  while (true) {
    printf("Hello World\n");

    sleep_for(30s);
  }
  return 0;
}

