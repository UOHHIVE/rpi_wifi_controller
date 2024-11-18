#include "commons/src/locks/buffer.hpp"
#include "commons/src/locks/lock.hpp"
#include "commons/src/locks/rw_lock.hpp"
#include "commons/src/netcode/netcode.cpp"
#include "commons/src/zumo.h"
#include <chrono>
#include <stdio.h>
#include <sys/time.h>
#include <thread>

// URL:
// https://stackoverflow.com/questions/158585/how-do-you-add-a-timed-delay-to-a-c-program
using namespace std::this_thread;     // sleep_for, sleep_until
using namespace std::chrono_literals; // ns, us, ms, s, h, etc.
using std::chrono::high_resolution_clock;
using std::chrono::system_clock;

using namespace netcode;

#define TPS 120            // ticks per second
#define MSPT 1000000 / TPS // microseconds per tick

// TODO: make sure buff is right size
// static RWLock<int> STATE;
static Lock<int> STATE;
static char BUFFER[1024];
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

void netcode_test() {

  ConnManager::cm_connect("10.140.10.31", 6009);

  char *hello = "Hello from client";
  ConnManager::cm_write(hello);

  ConnManager::cm_read(BUFFER);
  printf("BUFFER: %s\n", BUFFER);

  ConnManager::cm_disconnect();
}

int main(void) {

  std::thread p_listener(tcp_listener);
  std::thread p_test(netcode_test);

  // define timing stuff
  // std::chrono::_V2::system_clock::duration p1;
  // std::chrono::_V2::system_clock::duration p2;

  // int64_t t1;
  // int64_t t2;

  // int t_delay;

  // while (true) {

  //   p1 = std::chrono::high_resolution_clock::now().time_since_epoch();
  //   t1 = std::chrono::duration_cast<std::chrono::microseconds>(p1).count();

  //   // TODO: read state
  //   // TODO: compute state
  //   // TODO: do movement stuff...

  //   printf("tick \n");
  //   printf("state=%d (tick delay: %d)\n", STATE.read(), t_delay);

  //   p2 = std::chrono::high_resolution_clock::now().time_since_epoch();
  //   t2 = std::chrono::duration_cast<std::chrono::microseconds>(p2).count();

  //   t_delay = MSPT - (t2 - t1);
  //   std::this_thread::sleep_for(std::chrono::microseconds(t_delay));
  // }

  p_listener.join();
  p_test.join();
  return 0;
}