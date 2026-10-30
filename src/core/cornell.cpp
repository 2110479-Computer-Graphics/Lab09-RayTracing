#include "core/cornell.h"

#include <cmath>

static void add_quad(PtScene& scene, vec3 corner, vec3 u, vec3 v, int material) {
  int first = int(scene.mesh.positions.size());
  scene.mesh.positions.insert(scene.mesh.positions.end(), {corner, corner + u, corner + u + v, corner + v});
  scene.mesh.tris.push_back({first, first + 1, first + 2});
  scene.mesh.tris.push_back({first, first + 2, first + 3});
  scene.mesh.material.insert(scene.mesh.material.end(), {material, material});
}

static void add_box(PtScene& scene, vec3 size, float degrees, vec3 offset, int material) {
  float c = std::cos(degrees * PI / 180), s = std::sin(degrees * PI / 180);
  auto at = [&](float x, float y, float z) { return vec3{c * x + s * z, y, -s * x + c * z} + offset; };
  auto side = [&](vec3 p) { return at(p.x, p.y, p.z) - at(0, 0, 0); };
  vec3 x{size.x, 0, 0}, y{0, size.y, 0}, z{0, 0, size.z};
  add_quad(scene, at(0, 0, 0), side(y), side(x), material);
  add_quad(scene, at(0, 0, size.z), side(x), side(y), material);
  add_quad(scene, at(0, 0, 0), side(z), side(y), material);
  add_quad(scene, at(size.x, 0, 0), side(y), side(z), material);
  add_quad(scene, at(0, size.y, 0), side(z), side(x), material);
  add_quad(scene, at(0, 0, 0), side(x), side(z), material);
}

PtScene cornell_box() {
  PtScene scene;
  scene.materials = {
    {{0.73f, 0.73f, 0.73f}, {0, 0, 0}},
    {{0.65f, 0.05f, 0.05f}, {0, 0, 0}},
    {{0.12f, 0.45f, 0.15f}, {0, 0, 0}},
    {{0, 0, 0}, {15, 15, 15}},
  };
  scene.light = PtLight{{343, 554, 332}, {-130, 0, 0}, {0, 0, -105}, {15, 15, 15}};

  add_quad(scene, {555, 0, 0}, {0, 555, 0}, {0, 0, 555}, 1);
  add_quad(scene, {0, 0, 0}, {0, 0, 555}, {0, 555, 0}, 2);
  add_quad(scene, {0, 0, 0}, {555, 0, 0}, {0, 0, 555}, 0);
  add_quad(scene, {555, 555, 555}, {-555, 0, 0}, {0, 0, -555}, 0);
  add_quad(scene, {0, 0, 555}, {555, 0, 0}, {0, 555, 0}, 0);
  add_quad(scene, scene.light.corner, scene.light.e1, scene.light.e2, 3);
  add_box(scene, {165, 165, 165}, -18, {130, 0, 65}, 0);
  add_box(scene, {165, 330, 165}, 15, {265, 0, 295}, 0);
  return scene;
}

Ray cornell_camera_ray(float px, float py, int w, int h) {
  const vec3 eye{278, 278, -800};
  float scale = std::tan(20 * PI / 180);
  float sx = (2 * px / float(w) - 1) * scale * float(w) / float(h);
  float sy = (1 - 2 * py / float(h)) * scale;
  return Ray{eye, normalize(vec3{-sx, sy, 1})};
}
