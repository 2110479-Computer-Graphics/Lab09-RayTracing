#include "core/scene.h"

#include <cmath>
#include <limits>

#include "core/stats.h"
#include "student.h"

float EPS = 1e-3f;
int MAX_DEPTH = 4;
int STAGE = 99;

static const float INF = std::numeric_limits<float>::infinity();

static bool floor_intersect(const Ray& r, float t_max, Hit& hit) {
  static const Material light{{0.3f, 0.3f, 0.3f}, {1, 0, 0, 0}, 0, 1};
  static const Material dark{{0.3f, 0.2f, 0.1f}, {1, 0, 0, 0}, 0, 1};
  if(std::fabs(r.d.y) < 1e-3f) {
    return false;
  }
  float t = -(r.o.y + 4) / r.d.y;
  vec3 p = r.at(t);
  if(t <= EPS || t >= t_max || std::fabs(p.x) >= 10 || p.z >= -10 || p.z <= -30) {
    return false;
  }
  hit.t = t;
  hit.n = vec3{0, 1, 0};
  hit.tri = -1;
  int square = int(std::floor(0.5f * p.x)) + int(std::floor(0.5f * p.z));
  hit.material = (square & 1) ? &dark : &light;
  return true;
}

bool scene_intersect(const Ray& r, const Scene& scene, Hit& hit) {
  count_ray();
  float nearest = INF;
  for(const Sphere& s : scene.spheres) {
    float t = 0;
    if(ray_sphere(r, s, t) && t < nearest) {
      nearest = t;
      hit.t = t;
      hit.n = (r.at(t) - s.center) / s.radius;
      hit.tri = -1;
      hit.material = &s.material;
    }
  }
  if(scene.floor && floor_intersect(r, nearest, hit)) {
    nearest = hit.t;
  }
  for(const Mesh& m : scene.meshes) {
    float t_bound = 0;
    if(m.bound.radius > 0 && !ray_sphere(r, m.bound, t_bound)) {
      continue;
    }
    Hit mesh_hit;
    bool found = scene.use_bvh ? intersect_bvh(r, m, nearest, mesh_hit) : intersect_mesh(r, m, nearest, mesh_hit);
    if(found && mesh_hit.t < nearest) {
      nearest = mesh_hit.t;
      hit = mesh_hit;
      hit.material = &m.surface;
    }
  }
  if(nearest == INF) {
    return false;
  }
  hit.p = r.at(hit.t);
  return true;
}

Scene tiny_scene() {
  const Material ivory{{0.4f, 0.4f, 0.3f}, {0.6f, 0.3f, 0.1f, 0.0f}, 50, 1.0f};
  const Material glass{{0.6f, 0.7f, 0.8f}, {0.0f, 0.5f, 0.1f, 0.8f}, 125, 1.5f};
  const Material red_rubber{{0.3f, 0.1f, 0.1f}, {0.9f, 0.1f, 0.0f, 0.0f}, 10, 1.0f};
  const Material mirror{{1.0f, 1.0f, 1.0f}, {0.0f, 10.0f, 0.8f, 0.0f}, 1425, 1.0f};

  Scene scene;
  scene.spheres = {
    {{-3.0f, 0.0f, -16.0f}, 2, ivory},
    {{-1.0f, -1.5f, -12.0f}, 2, glass},
    {{1.5f, -0.5f, -18.0f}, 3, red_rubber},
    {{7.0f, 5.0f, -18.0f}, 4, mirror},
  };
  scene.lights = {
    {{-20, 20, 20}, 1.5f},
    {{30, 50, -25}, 1.8f},
    {{30, 20, 30}, 1.7f},
  };
  scene.floor = true;
  return scene;
}

bool pt_intersect(const Ray& r, const PtScene& scene, float t_max, Hit& hit) {
  count_ray();
  Hit mesh_hit;
  if(!intersect_mesh(r, scene.mesh, t_max, mesh_hit) || !(mesh_hit.t < t_max)) {
    return false;
  }
  hit = mesh_hit;
  hit.p = r.at(hit.t);
  return true;
}

const PtMaterial& pt_material(const PtScene& scene, const Hit& hit) {
  static const PtMaterial missing{{0, 0, 0}, {0, 0, 0}};
  if(hit.tri < 0 || hit.tri >= int(scene.mesh.material.size())) {
    return missing;
  }
  return scene.materials[size_t(scene.mesh.material[size_t(hit.tri)])];
}
