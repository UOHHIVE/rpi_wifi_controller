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

#define TPS 120            // ticks per second
#define MSPT 1000000 / TPS // microseconds per tick

void tcp_listener() {
  while (1) {
    printf("Listening...");

    // TODO: add code to set up the listener stuff
  }
}

int main(void) {

  // TODO: init state rwlock
  // TODO: init buffer
  // TODO: spawn listener

  // define timing stuff
  std::chrono::_V2::system_clock::duration p1;
  int64_t t1;
  int t_delay;

  std::chrono::_V2::system_clock::duration p2 = std::chrono::system_clock::now().time_since_epoch();
  int64_t t2 = std::chrono::duration_cast<std::chrono::microseconds>(p1).count();

  for (;;) {

    p1 = p2;
    t1 = t2;
    t_delay = MSPT;

    // TODO: read state
    // TODO: compute state
    // TODO: do movement stuff...

    p2 = std::chrono::system_clock::now().time_since_epoch();
    t2 = std::chrono::duration_cast<std::chrono::microseconds>(p2).count();
    t_delay -= (t2 - t1);

    std::this_thread::sleep_for(std::chrono::microseconds(t_delay));
  }

  return 0;
}