#include "commons/src/locks/buffer.hpp"
#include "commons/src/locks/lock.hpp"
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

#define TPS 1              // ticks per second
#define MSPT 1000000 / TPS // microseconds per tick

// TODO: make sure buff is right size
// static RWLock<int> STATE;
static Lock<int> STATE;
static BufferLock BUFFER;
static bool test_bool = true;

void tcp_listener() {
  printf("started listening...\n");
  while (1) {
    // TODO: add code to set up the listener stuff

    std::lock_guard<std::mutex> lock(STATE.mtx);
    STATE.inner += 1;

    if (STATE.inner > 69) {
      STATE.inner = 0;
    }
  }
}

int main(void) {

  std::thread p_listener(tcp_listener);

  // define timing stuff
  std::chrono::_V2::system_clock::duration p1;
  int64_t t1;
  int t_delay;

  std::chrono::_V2::system_clock::duration p2 = std::chrono::system_clock::now().time_since_epoch();
  int64_t t2 = std::chrono::duration_cast<std::chrono::microseconds>(p2).count();

  while (true) {

    p1 = p2;
    t1 = t2;

    printf("state=%d (tick delay: %d)\n", STATE.read(), t_delay);
    t_delay = MSPT;

    // TODO: read state
    // TODO: compute state
    // TODO: do movement stuff...

    p2 = std::chrono::system_clock::now().time_since_epoch();
    t2 = std::chrono::duration_cast<std::chrono::microseconds>(p2).count();
    t_delay -= (t2 - t1);

    if (t_delay > 0) { // time keeps going backwards ?? no clue why
      std::this_thread::sleep_for(std::chrono::microseconds(t_delay));
    }
  }

  p_listener.join();
  return 0;
}