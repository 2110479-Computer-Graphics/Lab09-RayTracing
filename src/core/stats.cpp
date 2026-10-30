#include "core/stats.h"

#include <atomic>

static thread_local stats::Totals local;
static std::atomic<uint64_t> total_rays{0}, total_triangle_tests{0}, total_node_visits{0};

void count_ray() { ++local.rays; }
void count_triangle_test() { ++local.triangle_tests; }
void count_node_visit() { ++local.node_visits; }

namespace stats {

void reset() {
  local = Totals{};
  total_rays = 0;
  total_triangle_tests = 0;
  total_node_visits = 0;
}

void flush() {
  total_rays += local.rays;
  total_triangle_tests += local.triangle_tests;
  total_node_visits += local.node_visits;
  local = Totals{};
}

Totals totals() {
  flush();
  return Totals{total_rays, total_triangle_tests, total_node_visits};
}

}
