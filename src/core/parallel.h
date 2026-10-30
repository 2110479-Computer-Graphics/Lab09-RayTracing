#pragma once

#include <functional>

int thread_count(int requested);

void parallel_rows(int height, int threads, const std::function<void(int)>& row);
