#include <cstdio>
#include <string>

#include "core/cli.h"
#include "core/image.h"
#include "core/parallel.h"
#include "student.h"

static const char* USAGE =
  "usage: raytrace [--depth n] [--eps x] [--stage n] [--threads n] [--out file.png] [--probe]";

static void probe(const char* label, const Ray& r, const Sphere& s) {
  float t = 0;
  bool hit = ray_sphere(r, s, t);
  row(label, hit ? format("hit at t = %g", double(t)) : std::string("no hit"));
}

static void touch_tasks() {
  Ray r{{0, 0, 0}, {0, 0, -1}};
  float t = 0;
  vec3 out;
  ray_sphere(r, Sphere{{0, 0, -5}, 1, Material{}}, t);
  reflect(r.d, vec3{0, 0, 1});
  refract(r.d, vec3{0, 0, 1}, 1.5f, out);
}

int main(int argc, char** argv) {
  Args args(argc, argv);
  MAX_DEPTH = args.integer("--depth", 4);
  EPS = args.number("--eps", 1e-3f);
  STAGE = args.integer("--stage", 99);
  int threads = thread_count(args.integer("--threads", 0));
  std::string out = args.text("--out", "out/raytrace.png");
  bool probing = args.flag("--probe");
  args.finish(USAGE);
  touch_tasks();

  if(probing) {
    Sphere sphere{{0, 0, -5}, 1, Material{}};
    probe("o = (0, 0, 0)", Ray{{0, 0, 0}, {0, 0, -1}}, sphere);
    probe("o = (0, 0, 3)", Ray{{0, 0, 3}, {0, 0, -1}}, sphere);
    print_tasks_not_done();
    return 0;
  }

  const int width = 800, height = 600;
  const float fov = PI / 3;
  Scene scene = tiny_scene();
  Image image(width, height);

  stats::reset();
  Timer timer;
  parallel_rows(height, threads, [&](int y) {
    for(int x = 0; x < width; ++x) {
      image.at(x, y) = cast_ray(camera_ray(x, y, width, height, fov), scene, 0);
    }
  });
  double ms = timer.ms();
  stats::Totals totals = stats::totals();
  bool saved = write_png(project_path(out), image, Tone::linear);

  row("image", format("%d x %d, vertical fov 60 deg", width, height));
  row("depth, eps", format("%d, %g", MAX_DEPTH, double(EPS)));
  row("threads", format("%d", threads));
  row("time", format("%.1f ms", ms));
  row("rays / pixel", format("%.3f", double(totals.rays) / (width * height)));
  row("saved", saved ? out : "FAILED to write " + out);
  print_tasks_not_done();
  return saved ? 0 : 1;
}
