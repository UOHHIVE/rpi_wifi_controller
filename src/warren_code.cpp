#include <cmath>
#include <iostream>

// struct Vec3 {
//   float _x = 0;
//   float _y = 0;
//   float _z = 0;

//   Vec3() = default;
//   Vec3(const float x, const float y, const float z) : _x(x), _y(y), _z(z) {}

//   static float dot(const Vec3 &u, const Vec3 &v) { return u._x * v._x + u._y * v._y + u._z * v._z; }

//   static Vec3 cross(const Vec3 &u, const Vec3 &v) { return {u._y * v._z - u._z * v._y, u._z * v._x - u._x * v._z, u._x * v._y - u._y * v._x}; }

//   friend Vec3 operator+(const Vec3 &u, const Vec3 &v) { return {u._x + v._x, u._y + v._y, u._z + v._z}; }

//   friend Vec3 operator-(const Vec3 &u, const Vec3 &v) { return {u._x - v._x, u._y - v._y, u._z - v._z}; }

//   friend Vec3 operator*(const float s, const Vec3 &v) { return {s * v._x, s * v._y, s * v._z}; }

//   Vec3 unit() const {
//     float magSqr = _x * _x + _y * _y + _z * _z;
//     if (magSqr > 0) {
//       float recip = 1.0f / (float)sqrt(magSqr);
//       return {_x * recip, _y * recip, _z * recip};
//     }
//     return {0, 0, 0};
//   }
// };

// struct Vec4 {
//   float _w = 0;
//   float _x = 0;
//   float _y = 0;
//   float _z = 0;

//   Vec4(const float w, const float x, const float y, const float z) : _w(w), _x(x), _y(y), _z(z) {}
// };

// void rotate_vector_by_quaternion(const Vec3 &v, const Vec4 &q, Vec3 &result) {
//   // Extract the vector part of the quaternion
//   Vec3 u(q._x, q._y, q._z);

//   // Extract the scalar part of the quaternion
//   float s = q._w;

//   // Maths
//   result = 2.0f * Vec3::dot(u, v) * u + (s * s - Vec3::dot(u, u)) * v + 2.0f * s * Vec3::cross(u, v);
// }

int main(int, char **) {
  // input
  // Vec4 robotOrientation(0, -0.258819f, 0, 0.9659258f);  // get from tracker
  Vec4 robotOrientation(0, 0.7071068f, 0, 0.7071068f); // get from tracker
  Vec3 robotPosition(-10, 0, 0);                       // get from tracker
  Vec3 targetPosition(10, 0, 10);                      // get from command

  // calculate world axis
  Vec3 robotAxis(0, 0, 1);
  Vec3 robotDir;
  rotate_vector_by_quaternion(robotAxis, robotOrientation, robotDir);

  Vec3 requiredDir = targetPosition - robotPosition;
  requiredDir = requiredDir.unit();

  float error = 1.0f - fabs(Vec3::dot(robotDir, requiredDir));
  bool turnRight = Vec3::cross(robotDir, requiredDir)._y > 0; // Note this might need reversing

  return 0;
}
