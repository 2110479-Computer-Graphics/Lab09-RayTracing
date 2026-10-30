#pragma once

#include "core/ray.h"
#include "core/rng.h"
#include "core/scene.h"
#include "core/stats.h"
#include "core/todo.h"
#include "core/vec3.h"

extern float EPS;
extern int MAX_DEPTH;
extern int STAGE;

Ray camera_ray(int x, int y, int width, int height, float fov);
bool ray_sphere(const Ray& r, const Sphere& s, float& t);
vec3 reflect(const vec3& d, const vec3& n);
bool refract(const vec3& d, const vec3& n, float ior, vec3& out);
vec3 cast_ray(const Ray& r, const Scene& scene, int depth);

bool ray_triangle(const Ray& r, const vec3& v0, const vec3& v1, const vec3& v2,
                  float& t, float& u, float& v);
bool intersect_mesh(const Ray& r, const Mesh& m, float t_max, Hit& hit);
bool ray_aabb(const Ray& r, const vec3& inv_d, const AABB& box, float t_max,
              float& t_entry);
bool intersect_bvh(const Ray& r, const Mesh& m, float t_max, Hit& hit);

vec3 cosine_sample_hemisphere(const vec3& n, float u1, float u2);
vec3 sample_light(const Hit& hit, const PtScene& scene, Rng& rng);
bool russian_roulette(vec3& throughput, int bounce, Rng& rng);
vec3 path_trace(const Ray& r, const PtScene& scene, Rng& rng, const PtOptions& opt);
