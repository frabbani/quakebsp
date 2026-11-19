#include "vector.h"

#include <cmath>

#define QUANT(f) static_cast<int>((f) * 1e5)

bool Vector2::operator<(const Vector2& rhs) const {
  int lhsX = QUANT(x);
  int rhsX = QUANT(rhs.x);
  if (lhsX < rhsX)
    return true;
  else if (lhsX > rhsX)
    return false;
  else {
    int lhsY = QUANT(y);
    int rhsY = QUANT(rhs.y);
    if (lhsY < rhsY)
      return true;
    else if (lhsY > rhsY)
      return false;
  }
  return false;
}

bool Vector2::operator>(const Vector2& rhs) const { return rhs < *this; }

bool Vector2::operator==(const Vector2& rhs) const {
  return fabsf(x - rhs.x) < tol && fabsf(y - rhs.y) < tol;
}

bool Vector2::equals(const Vector2& rhs, float tolerence) const {
  return fabsf(x - rhs.x) < tolerence && fabsf(y - rhs.y) < tolerence;
}

Vector2 Vector2::operator+(const Vector2& rhs) const {
  Vector2 r;
  r.x = x + rhs.x;
  r.y = y + rhs.y;
  return r;
}

Vector2 Vector2::operator-(const Vector2& rhs) const {
  Vector2 r;
  r.x = x - rhs.x;
  r.y = y - rhs.y;
  return r;
}

Vector2 Vector2::operator*(float s) const {
  Vector2 r;
  r.x = x * s;
  r.y = y * s;
  return r;
}

void Vector2::operator+=(const Vector2& rhs) {
  x += rhs.x;
  y += rhs.y;
}

void Vector2::operator-=(const Vector2& rhs) {
  x -= rhs.x;
  y -= rhs.y;
}

void Vector2::operator*=(float s) {
  x *= s;
  y *= s;
}

float Vector2::dot(const Vector2& rhs) const { return x * rhs.x + y * rhs.y; }

void Vector2::normalize() { *this = this->normalized(); }

float Vector2::length() const {
  float s = x * x + y * y;
  if (s < tolSq) return 0.0f;
  return sqrtf(s);
}

Vector2 Vector2::normalized() const {
  float l = length();
  if (l == 0.0f) {
    return Vector2(NaN, NaN);
  }
  return (*this) * (1.0f / l);
}

bool Vector2::isValid() const { return (fabsf(x) != NaN) && (fabsf(y) != NaN); }

Vector3 Vector3::cross(const Vector3& rhs) const {
  Vector3 r;
  r.xyz[0] = (xyz[1] * rhs.xyz[2] - xyz[2] * rhs.xyz[1]);
  r.xyz[1] = -(xyz[0] * rhs.xyz[2] - xyz[2] * rhs.xyz[0]);
  r.xyz[2] = (xyz[0] * rhs.xyz[1] - xyz[1] * rhs.xyz[0]);
  return r;
}

float Vector3::dot(const Vector3& rhs) const {
  return x * rhs.x + y * rhs.y + z * rhs.z;
}

Vector3 Vector3::operator+(const Vector3& rhs) const {
  Vector3 r;
  r.x = x + rhs.x;
  r.y = y + rhs.y;
  r.z = z + rhs.z;
  return r;
}

Vector3 Vector3::operator-(const Vector3& rhs) const {
  Vector3 r;
  r.x = x - rhs.x;
  r.y = y - rhs.y;
  r.z = z - rhs.z;
  return r;
}

Vector3 Vector3::operator*(float s) const {
  Vector3 r;
  r.x = x * s;
  r.y = y * s;
  r.z = z * s;
  return r;
}

void Vector3::operator+=(const Vector3& rhs) {
  x += rhs.x;
  y += rhs.y;
  z += rhs.z;
}

void Vector3::operator-=(const Vector3& rhs) {
  x -= rhs.x;
  y -= rhs.y;
  z -= rhs.z;
}

void Vector3::operator*=(float s) {
  x *= s;
  y *= s;
  z *= s;
}

void Vector3::normalize() { *this = this->normalized(); }

float Vector3::length() const {
  float s = x * x + y * y + z * z;
  if (s < tolSq) return 0.0f;
  return sqrtf(s);
}

Vector3 Vector3::normalized() const {
  float l = length();
  if (l == 0.0f) {
    return Vector3(NaN, NaN, NaN);
  }
  return (1.0f / l) * *this;
}

bool Vector3::isValid() const {
  return (fabsf(x) != NaN) && (fabsf(y) != NaN) && (fabsf(z) != NaN);
}

bool Vector3::equals(const Vector3& rhs, float tolerence) const {
  return fabsf(x - rhs.x) < tolerence && fabsf(y - rhs.y) < tolerence &&
         fabsf(z - rhs.z) < tolerence;
}

bool Vector3::operator<(const Vector3& rhs) const {
  int lhsX = QUANT(x);
  int rhsX = QUANT(rhs.x);
  if (lhsX < rhsX)
    return true;
  else if (lhsX > rhsX)
    return false;
  else {
    int lhsY = QUANT(y);
    int rhsY = QUANT(rhs.y);
    if (lhsY < rhsY)
      return true;
    else if (lhsY > rhsY)
      return false;
    else {
      int lhsZ = QUANT(z);
      int rhsZ = QUANT(rhs.z);
      if (lhsZ < rhsZ)
        return true;
      else if (lhsZ > rhsZ)
        return false;
    }
  }
  return false;
}

bool Vector3::operator>(const Vector3& rhs) const { return rhs < *this; }
bool Vector3::operator==(const Vector3& rhs) const {
  int lhsX = QUANT(x);
  int rhsX = QUANT(rhs.x);
  if (lhsX != rhsX) return false;
  int lhsY = QUANT(y);
  int rhsY = QUANT(rhs.y);
  if (lhsY != rhsY) return false;
  int lhsZ = QUANT(z);
  int rhsZ = QUANT(rhs.z);
  if (lhsZ != rhsZ) return false;
  return true;
}