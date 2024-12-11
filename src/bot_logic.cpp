#include "main.hpp"

extern void bot_logic() {
  // define timing stuff
  std::chrono::_V2::system_clock::duration p1;
  std::chrono::_V2::system_clock::duration p2;

  int64_t t1;
  int64_t t2;

  int t_delay;

  logging::log("Starting Ticking", TESTING);

  while (TICK) {
    p1 = std::chrono::high_resolution_clock::now().time_since_epoch();
    t1 = std::chrono::duration_cast<std::chrono::microseconds>(p1).count();

    auto s = STATE.read();

    if (!s.sleep) {
      if (s.aligned) {
        if (misc::in_bound(s.current_pos.x(), s.target_pos.x(), EB_XYZ) xor misc::in_bound(s.current_pos.x(), s.target_pos.x(), EB_XYZ)) {
          logging::catch_debug(TESTING, zumo_movement::stop, "ZUMO: STOP");
          std::lock_guard<std::mutex> lock(STATE.mtx);
          STATE.inner.aligned = false;
        } else if (misc::in_bound(s.current_pos.x(), s.target_pos.x(), EB_XYZ) && misc::in_bound(s.current_pos.x(), s.target_pos.x(), EB_XYZ)) {
          logging::catch_debug(TESTING, zumo_movement::stop, "ZUMO: STOP");
        } else {
          logging::catch_debug(TESTING, zumo_movement::forward, "ZUMO: FORWARD");
        }
      } else {
        // Handle bot's alignment logic...
        // Continue with your logic here
      }
    } else {
      logging::catch_debug(TESTING, zumo_movement::stop, "ZUMO: STOP");
      if (s.sleep > 0) {
        std::this_thread::sleep_for(std::chrono::microseconds(s.sleep));
        std::lock_guard<std::mutex> lock(STATE.mtx);
        STATE.inner.duration = 0;
        STATE.inner.sleep = false;
      }

      // Always re-align after a halt
      if (s.aligned) {
        std::lock_guard<std::mutex> lock(STATE.mtx);
        STATE.inner.aligned = false;
      }
    }

    p2 = std::chrono::high_resolution_clock::now().time_since_epoch();
    t2 = std::chrono::duration_cast<std::chrono::microseconds>(p2).count();
    t_delay = MSPT - (t2 - t1);
    std::this_thread::sleep_for(std::chrono::microseconds(t_delay));
  }
}
