#include "core/bvh.h"

#include <algorithm>
#include <limits>
#include <numeric>

#include "core/cli.h"

AABB empty_box() {
  const float inf = std::numeric_limits<float>::infinity();
  return AABB{{inf, inf, inf}, {-inf, -inf, -inf}};
}

AABB grow(const AABB& box, const vec3& p) {
  return AABB{{std::min(box.lo.x, p.x), std::min(box.lo.y, p.y), std::min(box.lo.z, p.z)},
              {std::max(box.hi.x, p.x), std::max(box.hi.y, p.y), std::max(box.hi.z, p.z)}};
}

AABB merge(const AABB& a, const AABB& b) { return grow(grow(a, b.lo), b.hi); }

float surface_area(const AABB& box) {
  vec3 e = box.hi - box.lo;
  if(e.x < 0) {
    return 0;
  }
  return 2 * (e.x * e.y + e.y * e.z + e.z * e.x);
}

namespace {

const int LEAF_SIZE = 4;
const int BINS = 16;
const float COST_TRAVERSE = 1.0f;
const float COST_INTERSECT = 1.5f;

struct Builder {
  Mesh& mesh;
  Split split;
  std::vector<AABB> boxes;
  std::vector<vec3> centroids;
  BVHStats stats;

  bool best_bin(int first, int count, int axis, float lo, float hi, const AABB& parent, float& cost, int& bin) {
    int bin_count[BINS] = {};
    AABB bin_box[BINS];
    for(AABB& b : bin_box) {
      b = empty_box();
    }
    for(int i = first; i < first + count; ++i) {
      int tri = mesh.bvh.order[size_t(i)];
      int b = std::min(BINS - 1, int(BINS * (centroids[size_t(tri)][axis] - lo) / (hi - lo)));
      ++bin_count[b];
      bin_box[b] = merge(bin_box[b], boxes[size_t(tri)]);
    }
    float right_area[BINS] = {};
    int right_count[BINS] = {};
    AABB right = empty_box();
    for(int b = BINS - 1, n = 0; b > 0; --b) {
      right = merge(right, bin_box[b]);
      n += bin_count[b];
      right_area[b] = surface_area(right);
      right_count[b] = n;
    }
    bool found = false;
    AABB left = empty_box();
    float parent_area = surface_area(parent);
    for(int b = 1, n = 0; b < BINS; ++b) {
      left = merge(left, bin_box[b - 1]);
      n += bin_count[b - 1];
      if(n == 0 || right_count[b] == 0) {
        continue;
      }
      float c = COST_TRAVERSE + (surface_area(left) / parent_area) * float(n) * COST_INTERSECT +
                (right_area[b] / parent_area) * float(right_count[b]) * COST_INTERSECT;
      if(!found || c < cost) {
        found = true;
        cost = c;
        bin = b;
      }
    }
    return found;
  }

  int partition(int first, int count, const AABB& box) {
    AABB cbox = empty_box();
    for(int i = first; i < first + count; ++i) {
      cbox = grow(cbox, centroids[size_t(mesh.bvh.order[size_t(i)])]);
    }
    vec3 extent = cbox.hi - cbox.lo;
    int longest = extent.x >= extent.y && extent.x >= extent.z ? 0 : (extent.y >= extent.z ? 1 : 2);
    if(!(extent[longest] > 0)) {
      return first;
    }
    auto begin = mesh.bvh.order.begin() + first, end = begin + count;
    if(split == Split::sah) {
      float best = 0;
      int best_axis = -1, best_bin_index = 0;
      for(int axis = 0; axis < 3; ++axis) {
        float cost = 0;
        int bin = 0;
        if(extent[axis] > 0 && best_bin(first, count, axis, cbox.lo[axis], cbox.hi[axis], box, cost, bin) &&
           (best_axis < 0 || cost < best)) {
          best = cost;
          best_axis = axis;
          best_bin_index = bin;
        }
      }
      if(best_axis >= 0) {
        float lo = cbox.lo[best_axis], hi = cbox.hi[best_axis];
        auto middle = std::partition(begin, end, [&](int tri) {
          return std::min(BINS - 1, int(BINS * (centroids[size_t(tri)][best_axis] - lo) / (hi - lo))) < best_bin_index;
        });
        return int(middle - mesh.bvh.order.begin());
      }
    }
    auto middle = begin + count / 2;
    std::nth_element(begin, middle, end, [&](int a, int b) {
      return centroids[size_t(a)][longest] < centroids[size_t(b)][longest];
    });
    return int(middle - mesh.bvh.order.begin());
  }

  int build(int first, int count, int depth) {
    int index = int(mesh.bvh.nodes.size());
    mesh.bvh.nodes.push_back(BVHNode{});
    AABB box = empty_box();
    for(int i = first; i < first + count; ++i) {
      box = merge(box, boxes[size_t(mesh.bvh.order[size_t(i)])]);
    }
    mesh.bvh.nodes[size_t(index)].box = box;
    stats.nodes++;
    stats.depth = std::max(stats.depth, depth);

    int middle = count <= LEAF_SIZE || depth >= BVH_MAX_DEPTH ? first : partition(first, count, box);
    if(middle <= first || middle >= first + count) {
      mesh.bvh.nodes[size_t(index)].first = first;
      mesh.bvh.nodes[size_t(index)].count = count;
      stats.leaves++;
      return index;
    }
    build(first, middle - first, depth + 1);
    int right = build(middle, first + count - middle, depth + 1);
    mesh.bvh.nodes[size_t(index)].right = right;
    return index;
  }
};

}

BVHStats build_bvh(Mesh& mesh, Split split) {
  Timer timer;
  Builder builder{mesh, split, {}, {}, {}};
  for(const auto& tri : mesh.tris) {
    vec3 a = mesh.positions[size_t(tri[0])], b = mesh.positions[size_t(tri[1])], c = mesh.positions[size_t(tri[2])];
    builder.boxes.push_back(grow(grow(grow(empty_box(), a), b), c));
    builder.centroids.push_back((a + b + c) / 3);
  }
  mesh.bvh.nodes.clear();
  mesh.bvh.order.resize(mesh.tris.size());
  std::iota(mesh.bvh.order.begin(), mesh.bvh.order.end(), 0);
  if(!mesh.tris.empty()) {
    builder.build(0, int(mesh.tris.size()), 0);
  }
  builder.stats.ms = timer.ms();
  return builder.stats;
}
