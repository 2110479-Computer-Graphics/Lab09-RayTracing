#pragma once

#include <string>

#include "core/scene.h"

Mesh make_torus_knot(int segments, int sides);

bool load_obj(const std::string& path, Mesh& mesh, std::string& error);

void place(Mesh& mesh, float size, const vec3& center);

void compute_normals(Mesh& mesh);
