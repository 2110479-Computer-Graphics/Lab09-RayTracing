#include "student.h"

#include <algorithm>
#include <cmath>

// TASK 3a: a unit direction around n with density cos(theta) / pi.
vec3 cosine_sample_hemisphere(const vec3& n, float u1, float u2) {
  task_not_done("3a");
  return n;
}

// TASK 3b: the light reaching hit straight from one random point on the lamp.
vec3 sample_light(const Hit& hit, const PtScene& scene, Rng& rng) {
  task_not_done("3b");
  return vec3{0, 0, 0};
}

// TASK 3c: Russian roulette (EQ-10.08): false stops the path.
bool russian_roulette(vec3& throughput, int bounce, Rng& rng) {
  task_not_done("3c");
  return true;
}

// TASK 3d: the light arriving along r, following one path.
vec3 path_trace(const Ray& r, const PtScene& scene, Rng& rng, const PtOptions& opt) {
  task_not_done("3d");
  return vec3{0, 0, 0};
}
