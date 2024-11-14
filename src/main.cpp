#include "commons/src/locks/buffer.hpp"
#include "commons/src/locks/rw_lock.hpp"
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

// TODO: make sure buff is right size
static BufferLock BUFFER;
static RWLock<int> STATE;

void tcp_listener() {
  while (1) {
    printf("Listening...");

    // TODO: add code to set up the listener stuff
  }
}

void test_thread() {
  for (int i = 0; i < 1000; i++) {
    int current_state = STATE.locking_read();
    STATE.write(current_state + 1);
  }

  // auto current = BUFFER.read();
  // printf(current);
}

int main(void) {
  std::thread p1(test_thread);
  std::thread p2(test_thread);

  p1.join();
  p2.join();

  printf("%d", STATE.read());
}

int main2(void) {

  // TODO: spawn listener

  // define timing stuff
  std::chrono::_V2::system_clock::duration p1;
  int64_t t1;
  int t_delay;

  std::chrono::_V2::system_clock::duration p2 = std::chrono::system_clock::now().time_since_epoch();
  int64_t t2 = std::chrono::duration_cast<std::chrono::microseconds>(p2).count();

  while (true) {

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