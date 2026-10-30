#include "core/mesh.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>

#include "core/bvh.h"

static vec3 knot_curve(float u) {
  const float p = 2, q = 3;
  float cs = std::cos(q / p * u);
  return vec3{(2 + cs) * 0.5f * std::cos(u), (2 + cs) * 0.5f * std::sin(u), 0.5f * std::sin(q / p * u)};
}

Mesh make_torus_knot(int segments, int sides) {
  const float tube = 0.4f;
  Mesh mesh;
  for(int i = 0; i < segments; ++i) {
    float u = 4 * PI * float(i) / float(segments);
    vec3 p1 = knot_curve(u), p2 = knot_curve(u + 0.01f);
    vec3 t = p2 - p1, n = p2 + p1;
    vec3 b = normalize(cross(t, n));
    n = normalize(cross(b, t));
    for(int j = 0; j < sides; ++j) {
      float v = 2 * PI * float(j) / float(sides);
      vec3 around = n * -std::cos(v) + b * std::sin(v);
      mesh.positions.push_back(p1 + around * tube);
      mesh.normals.push_back(around);
    }
  }
  for(int i = 0; i < segments; ++i) {
    for(int j = 0; j < sides; ++j) {
      int a = i * sides + j;
      int b = ((i + 1) % segments) * sides + j;
      int c = ((i + 1) % segments) * sides + (j + 1) % sides;
      int d = i * sides + (j + 1) % sides;
      mesh.tris.push_back({a, b, d});
      mesh.tris.push_back({b, c, d});
    }
  }
  return mesh;
}

static int face_index(const std::string& token, int vertex_count) {
  int index = 0;
  try {
    index = std::stoi(token.substr(0, token.find('/')));
  } catch(...) {
    return -1;
  }
  index = index < 0 ? vertex_count + index : index - 1;
  return index >= 0 && index < vertex_count ? index : -1;
}

bool load_obj(const std::string& path, Mesh& mesh, std::string& error) {
  std::ifstream file(path);
  if(!file) {
    error = "cannot open " + path;
    return false;
  }
  Mesh result;
  std::string line;
  for(int number = 1; std::getline(file, line); ++number) {
    std::istringstream in(line);
    std::string kind;
    in >> kind;
    if(kind == "v") {
      vec3 p;
      in >> p.x >> p.y >> p.z;
      result.positions.push_back(p);
    } else if(kind == "f") {
      std::vector<int> face;
      for(std::string token; in >> token;) {
        int index = face_index(token, int(result.positions.size()));
        if(index < 0) {
          error = path + ":" + std::to_string(number) + ": bad vertex '" + token + "'";
          return false;
        }
        face.push_back(index);
      }
      for(size_t k = 2; k < face.size(); ++k) {
        result.tris.push_back({face[0], face[k - 1], face[k]});
      }
    }
  }
  if(result.tris.empty()) {
    error = path + " has no triangles";
    return false;
  }
  compute_normals(result);
  mesh = result;
  return true;
}

void place(Mesh& mesh, float size, const vec3& center) {
  AABB box = empty_box();
  for(const vec3& p : mesh.positions) {
    box = grow(box, p);
  }
  vec3 extent = box.hi - box.lo;
  float scale = size / std::max(extent.x, std::max(extent.y, extent.z));
  vec3 middle = (box.lo + box.hi) * 0.5f;
  mesh.bound = Sphere{center, 0, Material{}};
  for(vec3& p : mesh.positions) {
    p = center + (p - middle) * scale;
    mesh.bound.radius = std::max(mesh.bound.radius, length(p - center) * 1.001f);
  }
}

void compute_normals(Mesh& mesh) {
  mesh.normals.assign(mesh.positions.size(), vec3{0, 0, 0});
  for(const auto& tri : mesh.tris) {
    vec3 a = mesh.positions[size_t(tri[0])], b = mesh.positions[size_t(tri[1])], c = mesh.positions[size_t(tri[2])];
    vec3 n = cross(b - a, c - a);
    for(int k : tri) {
      mesh.normals[size_t(k)] += n;
    }
  }
  for(vec3& n : mesh.normals) {
    float l = length(n);
    n = l > 0 ? n / l : vec3{0, 1, 0};
  }
}
