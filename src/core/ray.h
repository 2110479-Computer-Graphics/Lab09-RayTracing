#pragma once

#include "core/vec3.h"

struct Ray {
  vec3 o;
  vec3 d;

  vec3 at(float t) const { return o + d * t; }
};
