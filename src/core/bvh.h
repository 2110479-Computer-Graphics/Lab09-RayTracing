#pragma once

#include "core/scene.h"

enum class Split {
  median,
  sah
};

struct BVHStats {
  int nodes = 0;
  int leaves = 0;
  int depth = 0;
  double ms = 0;
};

const int BVH_MAX_DEPTH = 60;

BVHStats build_bvh(Mesh& mesh, Split split);

AABB empty_box();
AABB grow(const AABB& box, const vec3& p);
AABB merge(const AABB& a, const AABB& b);
float surface_area(const AABB& box);
