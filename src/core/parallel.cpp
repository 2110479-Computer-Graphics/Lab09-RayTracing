#include "core/parallel.h"

#include <algorithm>
#include <atomic>
#include <thread>
#include <vector>

#include "core/stats.h"

int thread_count(int requested) {
  if(requested > 0) {
    return requested;
  }
  return std::max(1u, std::thread::hardware_concurrency());
}

void parallel_rows(int height, int threads, const std::function<void(int)>& row) {
  std::atomic<int> next{0};
  auto work = [&]() {
    for(int y = next++; y < height; y = next++) {
      row(y);
    }
    stats::flush();
  };
  std::vector<std::thread> pool;
  for(int i = 1; i < threads; ++i) {
    pool.emplace_back(work);
  }
  work();
  for(std::thread& t : pool) {
    t.join();
  }
}
