#pragma once

#include "core/scene.h"

PtScene cornell_box();

Ray cornell_camera_ray(float px, float py, int w, int h);
