#pragma once

#include <cstdint>

void count_ray();
void count_triangle_test();
void count_node_visit();

namespace stats {

struct Totals {
  uint64_t rays = 0;
  uint64_t triangle_tests = 0;
  uint64_t node_visits = 0;
};

void reset();
void flush();
Totals totals();

}
