#include <cstdio>
#include <string>
#include <vector>

#include "core/bvh.h"
#include "core/cli.h"
#include "core/image.h"
#include "core/mesh.h"
#include "core/parallel.h"
#include "student.h"

static const char* USAGE =
  "usage: mesh [--tris 1k|2k|4k|16k|64k|256k] [--obj file.obj] [--flat] [--glass] [--bvh] [--split sah|median]\n"
  "            [--depth n] [--eps x] [--threads n] [--out file.png] [--sweep]";

struct KnotSize {
  const char* name;
  int segments, sides;
};
static const KnotSize KNOTS[] = {{"1k", 64, 8},    {"2k", 64, 16},   {"4k", 128, 16},
                                 {"16k", 256, 32}, {"64k", 512, 64}, {"256k", 1024, 128}};
static const int BRUTE_FORCE_LIMIT = 20000;

static const KnotSize* knot_size(const std::string& name) {
  for(const KnotSize& k : KNOTS) {
    if(name == k.name) {
      return &k;
    }
  }
  return nullptr;
}

static bool touch_tasks(bool with_bvh) {
  Mesh m;
  m.positions = {{-1, -1, -5}, {1, -1, -5}, {0, 1, -5}};
  m.tris = {{0, 1, 2}};
  build_bvh(m, Split::sah);
  Ray r{{0, 0, 0}, {0, 0, -1}};
  float t = 0, u = 0, v = 0;
  Hit hit;
  vec3 out;
  ray_triangle(r, m.positions[0], m.positions[1], m.positions[2], t, u, v);
  intersect_mesh(r, m, 1e30f, hit);
  ray_sphere(r, Sphere{{0, 0, -5}, 1, Material{}}, t);
  reflect(r.d, vec3{0, 0, 1});
  refract(r.d, vec3{0, 0, 1}, 1.5f, out);
  if(with_bvh) {
    ray_aabb(r, vec3{1e30f, 1e30f, -1}, m.bvh.nodes[0].box, 1e30f, t);
    intersect_bvh(r, m, 1e30f, hit);
  }
  stats::reset();
  return with_bvh && !is_task_not_done("2d") && !is_task_not_done("2c");
}

static std::string per_ray(uint64_t count, uint64_t rays) {
  return format("%.1f", rays ? double(count) / double(rays) : 0.0);
}

static void sweep(bool bvh_done, int threads) {
  const int width = 320, height = 240;
  std::printf("  sweep          primary rays only, %d x %d, the knot alone\n", width, height);
  std::printf("  %9s  %-12s %10s %11s %11s %10s\n", "triangles", "method", "build ms", "render ms",
              "tests/ray", "nodes/ray");
  for(const KnotSize& k : KNOTS) {
    Scene scene;
    scene.meshes.push_back(make_torus_knot(k.segments, k.sides));
    place(scene.meshes[0], 7, vec3{0, 0, -10});
    int triangles = int(scene.meshes[0].tris.size());
    for(int method = 0; method < 3; ++method) {
      const char* names[] = {"brute force", "bvh median", "bvh sah"};
      double build_ms = 0;
      if(method == 0 && triangles > BRUTE_FORCE_LIMIT) {
        std::printf("  %9d  %-12s %10s %11s   skipped (slow): extrapolate from the rows above\n", triangles,
                    names[method], "-", "-");
        continue;
      }
      if(method > 0) {
        build_ms = build_bvh(scene.meshes[0], method == 1 ? Split::median : Split::sah).ms;
        if(!bvh_done) {
          std::printf("  %9d  %-12s %10.1f %11s   TASK 2c/2d not done\n", triangles, names[method], build_ms, "-");
          continue;
        }
      }
      scene.use_bvh = method > 0;
      stats::reset();
      Timer timer;
      parallel_rows(height, threads, [&](int y) {
        for(int x = 0; x < width; ++x) {
          Hit hit;
          scene_intersect(camera_ray(x, y, width, height, PI / 3), scene, hit);
        }
      });
      double ms = timer.ms();
      stats::Totals totals = stats::totals();
      std::printf("  %9d  %-12s %10.1f %11.1f %11s %10s\n", triangles, names[method], build_ms, ms,
                  per_ray(totals.triangle_tests, totals.rays).c_str(), per_ray(totals.node_visits, totals.rays).c_str());
    }
  }
}

int main(int argc, char** argv) {
  Args args(argc, argv);
  MAX_DEPTH = args.integer("--depth", 4);
  EPS = args.number("--eps", 1e-3f);
  int threads = thread_count(args.integer("--threads", 0));
  std::string tris = args.text("--tris", "1k");
  std::string obj = args.text("--obj", "");
  bool flat = args.flag("--flat");
  bool glass = args.flag("--glass");
  bool want_bvh = args.flag("--bvh");
  std::string split_name = args.text("--split", "sah");
  std::string out = args.text("--out", "out/mesh.png");
  bool sweeping = args.flag("--sweep");
  args.finish(USAGE);
  const KnotSize* knot = knot_size(tris);
  if(!knot || (split_name != "sah" && split_name != "median")) {
    std::fprintf(stderr, "error: bad --tris or --split\n%s\n", USAGE);
    return 2;
  }

  bool bvh_done = touch_tasks(want_bvh || sweeping);
  if(sweeping) {
    sweep(bvh_done, threads);
    print_tasks_not_done();
    return 0;
  }

  Scene scene = tiny_scene();
  Mesh mesh;
  std::string what;
  if(obj.empty()) {
    mesh = make_torus_knot(knot->segments, knot->sides);
    what = "torus knot";
  } else {
    std::string error;
    if(!load_obj(project_path(obj), mesh, error)) {
      std::fprintf(stderr, "error: %s\n", error.c_str());
      return 1;
    }
    what = obj;
  }
  place(mesh, 6, vec3{-7, 4.5f, -18});
  if(flat) {
    mesh.normals.clear();
  }
  mesh.surface = glass ? scene.spheres[1].material : Material{{0.8f, 0.55f, 0.2f}, {0.7f, 0.5f, 0.1f, 0.0f}, 60, 1.0f};

  row("mesh", format("%s, %d triangles, %s normals%s", what.c_str(), int(mesh.tris.size()), flat ? "flat" : "smooth",
                     glass ? ", glass" : ""));
  if(want_bvh && bvh_done) {
    BVHStats st = build_bvh(mesh, split_name == "sah" ? Split::sah : Split::median);
    row("bvh", format("%s split: %d nodes, %d leaves, depth %d, built in %.1f ms", split_name.c_str(), st.nodes,
                      st.leaves, st.depth, st.ms));
    scene.use_bvh = true;
  } else {
    row("intersect", want_bvh ? "TASK 2c/2d not done: brute force" : "brute force (add --bvh for the BVH)");
  }
  scene.meshes.push_back(mesh);

  const int width = 800, height = 600;
  Image image(width, height);
  stats::reset();
  Timer timer;
  parallel_rows(height, threads, [&](int y) {
    for(int x = 0; x < width; ++x) {
      image.at(x, y) = cast_ray(camera_ray(x, y, width, height, PI / 3), scene, 0);
    }
  });
  double ms = timer.ms();
  stats::Totals totals = stats::totals();
  bool saved = write_png(project_path(out), image, Tone::linear);

  row("threads", format("%d", threads));
  row("time", format("%.1f ms", ms));
  row("rays / pixel", format("%.3f", double(totals.rays) / (width * height)));
  row("tests / ray", per_ray(totals.triangle_tests, totals.rays) + " triangle tests");
  row("nodes / ray", per_ray(totals.node_visits, totals.rays) + " BVH nodes visited");
  row("saved", saved ? out : "FAILED to write " + out);
  print_tasks_not_done();
  return saved ? 0 : 1;
}
