#include "core/image.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <filesystem>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

static float finite_or_zero(float v) {
  if(std::isnan(v)) {
    return 0;
  }
  return std::min(std::max(v, 0.0f), 1e6f);
}

static float filmic(float v) {
  float mapped = std::min(1.0f, v * (2.51f * v + 0.03f) / (v * (2.43f * v + 0.59f) + 0.14f));
  return std::sqrt(std::max(0.0f, mapped));
}

static void make_parent(const std::string& path) {
  std::filesystem::path parent = std::filesystem::path(path).parent_path();
  if(!parent.empty()) {
    std::error_code ignored;
    std::filesystem::create_directories(parent, ignored);
  }
}

bool write_png(const std::string& path, const Image& img, Tone tone, float exposure) {
  std::vector<uint8_t> bytes(img.px.size() * 3);
  for(size_t i = 0; i < img.px.size(); ++i) {
    vec3 c = vec3{finite_or_zero(img.px[i].x), finite_or_zero(img.px[i].y), finite_or_zero(img.px[i].z)} * exposure;
    if(tone == Tone::linear) {
      float brightest = std::max(1.0f, std::max(c.x, std::max(c.y, c.z)));
      c = c / brightest;
    } else {
      c = {filmic(c.x), filmic(c.y), filmic(c.z)};
    }
    for(int k = 0; k < 3; ++k) {
      bytes[i * 3 + k] = uint8_t(std::lround(255 * c[k]));
    }
  }
  make_parent(path);
  return stbi_write_png(path.c_str(), img.w, img.h, 3, bytes.data(), img.w * 3) != 0;
}

bool write_pfm(const std::string& path, const Image& img) {
  make_parent(path);
  FILE* f = std::fopen(path.c_str(), "wb");
  if(!f) {
    return false;
  }
  std::fprintf(f, "PF\n%d %d\n-1.0\n", img.w, img.h);
  bool ok = true;
  for(int y = img.h - 1; y >= 0; --y) {
    ok = ok && std::fwrite(&img.at(0, y), sizeof(vec3), size_t(img.w), f) == size_t(img.w);
  }
  return std::fclose(f) == 0 && ok;
}

bool read_pfm(const std::string& path, Image& img) {
  FILE* f = std::fopen(path.c_str(), "rb");
  if(!f) {
    return false;
  }
  char magic[3] = {};
  int w = 0, h = 0;
  float scale = 0;
  bool ok = std::fscanf(f, "%2s %d %d %f", magic, &w, &h, &scale) == 4 && std::string(magic) == "PF" &&
            w > 0 && h > 0 && scale < 0 && std::fgetc(f) != EOF;
  if(ok) {
    img = Image(w, h);
    for(int y = h - 1; ok && y >= 0; --y) {
      ok = std::fread(&img.at(0, y), sizeof(vec3), size_t(w), f) == size_t(w);
    }
  }
  std::fclose(f);
  return ok;
}

double masked_rmse(const Image& img, const Image& ref, float max_luminance, int* pixels) {
  double sum = 0;
  int used = 0;
  for(int y = 0; y < ref.h; ++y) {
    for(int x = 0; x < ref.w; ++x) {
      bool bright = false;
      for(int dy = -1; dy <= 1; ++dy) {
        for(int dx = -1; dx <= 1; ++dx) {
          int nx = std::min(std::max(x + dx, 0), ref.w - 1), ny = std::min(std::max(y + dy, 0), ref.h - 1);
          bright = bright || luminance(ref.at(nx, ny)) > max_luminance;
        }
      }
      if(!bright) {
        vec3 d = img.at(x, y) - ref.at(x, y);
        sum += double(dot(d, d));
        ++used;
      }
    }
  }
  if(pixels) {
    *pixels = used;
  }
  return used > 0 ? std::sqrt(sum / (3.0 * used)) : 0.0;
}
