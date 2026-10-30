#pragma once

#include <array>
#include <vector>

#include "core/ray.h"
#include "core/vec3.h"

struct Material {
  vec3 diffuse;
  float albedo[4] = {1, 0, 0, 0};
  float specular_exponent = 0;
  float ior = 1;
};

struct Sphere {
  vec3 center;
  float radius = 1;
  Material material;
};

struct Light {
  vec3 position;
  float intensity = 1;
};

struct Hit {
  float t = 0;
  vec3 p;
  vec3 n;
  int tri = -1;
  const Material* material = nullptr;
};

struct AABB {
  vec3 lo, hi;
};

struct BVHNode {
  AABB box;
  int first = 0;
  int count = 0;
  int right = 0;
};

struct BVH {
  std::vector<BVHNode> nodes;
  std::vector<int> order;
};

struct Mesh {
  std::vector<vec3> positions;
  std::vector<vec3> normals;
  std::vector<std::array<int, 3>> tris;
  std::vector<int> material;
  Material surface;
  BVH bvh;
  Sphere bound;
};

struct Scene {
  std::vector<Sphere> spheres;
  std::vector<Light> lights;
  std::vector<Mesh> meshes;
  bool floor = false;
  vec3 background{0.2f, 0.7f, 0.8f};
  bool use_bvh = false;
};

bool scene_intersect(const Ray& r, const Scene& scene, Hit& hit);

Scene tiny_scene();

struct PtMaterial {
  vec3 albedo;
  vec3 emission;
};

struct PtLight {
  vec3 corner, e1, e2;
  vec3 emission;

  float area() const { return length(cross(e1, e2)); }
  vec3 normal() const { return normalize(cross(e1, e2)); }
};

struct PtScene {
  Mesh mesh;
  std::vector<PtMaterial> materials;
  PtLight light;
};

struct PtOptions {
  int max_depth = 64;
  bool nee = true;
  bool rr = true;
};

bool pt_intersect(const Ray& r, const PtScene& scene, float t_max, Hit& hit);

const PtMaterial& pt_material(const PtScene& scene, const Hit& hit);
