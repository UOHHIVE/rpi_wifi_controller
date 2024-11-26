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
#define TICK false

// TODO: make sure buff is right size
static Lock<int> STATE;
static char BUFFER[1024];

void tcp_listener() {
  printf("started listening...\n");

  // ConnManager::cm_connect("10.140.10.61", 6009);
  Socket s = Socket("10.140.10.61", 6009);

  uint32_t test = 69;
  s._send(reinterpret_cast<char *>(&test), sizeof(test));

  char *hello = "Hello from client";
  s._send(hello, sizeof(hello));

  s._read(BUFFER, 1024);
  printf("BUFFER: %s\n", BUFFER);

  while (1) {
    // TODO: start listening

    // std::lock_guard<std::mutex> lock(STATE.mtx);
    // STATE.inner += 1;

    // if (STATE.inner >= 70) {
    //   STATE.inner = 0;
    // }

    // s._send(reinterpret_cast<char *>(&STATE.inner), sizeof(STATE.inner));
  }

  sleep_for(5s);
  s._disconnect();
}

int main(void) {

  std::thread p_listener(tcp_listener);

  // define timing stuff
  std::chrono::_V2::system_clock::duration p1;
  std::chrono::_V2::system_clock::duration p2;

  int64_t t1;
  int64_t t2;

  int t_delay;

  while (TICK) {

    p1 = std::chrono::high_resolution_clock::now().time_since_epoch();
    t1 = std::chrono::duration_cast<std::chrono::microseconds>(p1).count();

    // TODO: read state
    // TODO: compute state
    // TODO: do movement stuff...

    printf("tick \n");
    printf("state=%d (tick delay: %d)\n", STATE.read(), t_delay);

    p2 = std::chrono::high_resolution_clock::now().time_since_epoch();
    t2 = std::chrono::duration_cast<std::chrono::microseconds>(p2).count();

    t_delay = MSPT - (t2 - t1);
    std::this_thread::sleep_for(std::chrono::microseconds(t_delay));
  }

  p_listener.join();
  return 0;
}