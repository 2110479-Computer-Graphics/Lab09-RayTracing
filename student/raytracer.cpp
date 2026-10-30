#include "student.h"

#include <algorithm>
#include <cmath>

// TASK 1a: the ray from the camera through the centre of pixel (x, y).
Ray camera_ray(int x, int y, int width, int height, float fov) {
  task_not_done("1a");
  return Ray{vec3{0, 0, 0}, vec3{0, 0, -1}};
}

// TASK 1b: the smallest t > EPS where r meets the sphere (EQ-10.02).
bool ray_sphere(const Ray& r, const Sphere& s, float& t) {
  task_not_done("1b");
  return false;
}

// TASK 1c: d reflected about the unit normal n (EQ-10.04).
vec3 reflect(const vec3& d, const vec3& n) {
  task_not_done("1c");
  return vec3{0, 0, 0};
}

// TASK 1c: d refracted through a surface whose normal n points out of the object.
// false on total internal reflection.
bool refract(const vec3& d, const vec3& n, float ior, vec3& out) {
  task_not_done("1c");
  return false;
}

// TASK 1d: the colour seen along r: Phong, shadows, reflection, refraction (Whitted).
vec3 cast_ray(const Ray& r, const Scene& scene, int depth) {
  task_not_done("1d");
  return scene.background;
}
