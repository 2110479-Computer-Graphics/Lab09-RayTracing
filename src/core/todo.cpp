#include "core/todo.h"

#include <cstdio>
#include <mutex>
#include <set>

static std::mutex lock;
static std::set<std::string> not_done;

void task_not_done(const char* task) {
  thread_local const char* seen[32];
  thread_local int count = 0;
  for(int i = 0; i < count; ++i) {
    if(seen[i] == task) {
      return;
    }
  }
  {
    std::lock_guard<std::mutex> guard(lock);
    not_done.insert(task);
  }
  if(count < 32) {
    seen[count++] = task;
  }
}

bool is_task_not_done(const char* task) {
  std::lock_guard<std::mutex> guard(lock);
  return not_done.count(task) > 0;
}

std::vector<std::string> tasks_not_done() {
  std::lock_guard<std::mutex> guard(lock);
  return std::vector<std::string>(not_done.begin(), not_done.end());
}

void print_tasks_not_done() {
  for(const std::string& task : tasks_not_done()) {
    std::printf("  TASK %s not done\n", task.c_str());
  }
}
