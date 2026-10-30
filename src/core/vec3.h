#pragma once

#include <cmath>

const float PI = 3.14159265358979f;

struct vec3 {
  float x = 0, y = 0, z = 0;

  float& operator[](int i) { return i == 0 ? x : (i == 1 ? y : z); }
  float operator[](int i) const { return i == 0 ? x : (i == 1 ? y : z); }
};

inline vec3 operator+(const vec3& a, const vec3& b) { return {a.x + b.x, a.y + b.y, a.z + b.z}; }
inline vec3 operator-(const vec3& a, const vec3& b) { return {a.x - b.x, a.y - b.y, a.z - b.z}; }
inline vec3 operator-(const vec3& a) { return {-a.x, -a.y, -a.z}; }
inline vec3 operator*(const vec3& a, const vec3& b) { return {a.x * b.x, a.y * b.y, a.z * b.z}; }
inline vec3 operator*(const vec3& a, float s) { return {a.x * s, a.y * s, a.z * s}; }
inline vec3 operator*(float s, const vec3& a) { return a * s; }
inline vec3 operator/(const vec3& a, float s) { return {a.x / s, a.y / s, a.z / s}; }
inline vec3& operator+=(vec3& a, const vec3& b) { return a = a + b; }
inline vec3& operator*=(vec3& a, const vec3& b) { return a = a * b; }
inline vec3& operator*=(vec3& a, float s) { return a = a * s; }
inline vec3& operator/=(vec3& a, float s) { return a = a / s; }

inline float dot(const vec3& a, const vec3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
inline vec3 cross(const vec3& a, const vec3& b) {
  return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}
inline float length(const vec3& a) { return std::sqrt(dot(a, a)); }
inline vec3 normalize(const vec3& a) { return a / length(a); }

inline float luminance(const vec3& c) { return 0.2126f * c.x + 0.7152f * c.y + 0.0722f * c.z; }

inline void make_basis(const vec3& n, vec3& t, vec3& b) {
  float sign = std::copysign(1.0f, n.z);
  float a = -1.0f / (sign + n.z);
  float c = n.x * n.y * a;
  t = {1.0f + sign * n.x * n.x * a, sign * c, -sign * n.x};
  b = {c, sign + n.y * n.y * a, -n.y};
}
