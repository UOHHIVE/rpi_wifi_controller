#include <cstdint>

struct vec3 {
  int x;
  int y;
  int z;
};

struct BotState {
  uint64_t id;
  vec3 current_pos;
  vec3 current_rot;
  vec3 target_pos;
  bool halted;
};

// class BotState {
// public:
//   uint64_t id;
//   vec3 current_pos;
//   vec3 current_rot;
//   vec3 target_pos;
//   bool halted;

// protected:
// private:
// };