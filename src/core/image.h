#pragma once

#include <string>
#include <vector>

#include "core/vec3.h"

struct Image {
  int w = 0, h = 0;
  std::vector<vec3> px;

  Image() = default;
  Image(int width, int height) : w(width), h(height), px(size_t(width) * size_t(height)) {}

  vec3& at(int x, int y) { return px[size_t(y) * size_t(w) + size_t(x)]; }
  const vec3& at(int x, int y) const { return px[size_t(y) * size_t(w) + size_t(x)]; }
};

enum class Tone {
  linear,
  filmic
};

bool write_png(const std::string& path, const Image& img, Tone tone, float exposure = 1);

double masked_rmse(const Image& img, const Image& ref, float max_luminance, int* pixels = nullptr);

bool write_pfm(const std::string& path, const Image& img);
bool read_pfm(const std::string& path, Image& img);
