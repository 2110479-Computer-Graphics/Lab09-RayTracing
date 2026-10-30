#include "student.h"

#include <algorithm>
#include <cmath>

// TASK 2a: Moller-Trumbore (DEF-10.01): does r hit the triangle at t > EPS? Write t, u, v.
bool ray_triangle(const Ray& r, const vec3& v0, const vec3& v1, const vec3& v2,
                  float& t, float& u, float& v) {
  task_not_done("2a");
  return false;
}

// TASK 2b: the closest triangle of m below t_max, testing every one (count_triangle_test()).
// Fill hit.t, hit.tri and hit.n.
bool intersect_mesh(const Ray& r, const Mesh& m, float t_max, Hit& hit) {
  task_not_done("2b");
  return false;
}

// TASK 2c (optional): the slab test (EQ-10.03). inv_d is 1 / r.d; write the entry distance.
bool ray_aabb(const Ray& r, const vec3& inv_d, const AABB& box, float t_max, float& t_entry) {
  task_not_done("2c");
  return false;
}

// TASK 2d (optional): TASK 2b through m.bvh (count_node_visit() for every node).
bool intersect_bvh(const Ray& r, const Mesh& m, float t_max, Hit& hit) {
  task_not_done("2d");
  return false;
}
