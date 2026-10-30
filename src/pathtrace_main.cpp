#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

#include "core/cli.h"
#include "core/cornell.h"
#include "core/image.h"
#include "core/parallel.h"
#include "student.h"

static const char* USAGE =
  "usage: pathtrace [--spp n] [--depth n] [--no-nee] [--no-rr] [--eps x] [--seed n] [--sweep] [--threads n] [--out file.png|.pfm]";

static const int SIZE = 256;
static const float EXPOSURE = 0.85f;
static const char* REFERENCE = "reference/pathtrace_ref.pfm";

static void touch_tasks(const PtScene& scene) {
  Rng rng(0, 0);
  float t = 0, u = 0, v = 0;
  Hit seen;
  ray_triangle(Ray{{0, 0, 0}, {0, 0, -1}}, vec3{-1, -1, -5}, vec3{1, -1, -5}, vec3{0, 1, -5}, t, u, v);
  pt_intersect(cornell_camera_ray(128, 128, SIZE, SIZE), scene, 1e30f, seen);
  vec3 throughput{1, 1, 1};
  Hit hit;
  hit.p = vec3{278, 1, 278};
  hit.n = vec3{0, 1, 0};
  hit.tri = 4;
  cosine_sample_hemisphere(vec3{0, 1, 0}, 0.5f, 0.5f);
  sample_light(hit, scene, rng);
  russian_roulette(throughput, 5, rng);
  path_trace(cornell_camera_ray(128, 128, SIZE, SIZE), scene, rng, PtOptions{});
  stats::reset();
}

static Image render(const PtScene& scene, int spp, const PtOptions& opt, int threads, int seed) {
  Image image(SIZE, SIZE);
  parallel_rows(SIZE, threads, [&](int y) {
    for(int x = 0; x < SIZE; ++x) {
      vec3 sum{0, 0, 0};
      for(int s = 0; s < spp; ++s) {
        Rng rng(uint64_t(y * SIZE + x), uint64_t(s) + (uint64_t(seed) << 32));
        float px = float(x) + rng.next(), py = float(y) + rng.next();
        sum += path_trace(cornell_camera_ray(px, py, SIZE, SIZE), scene, rng, opt);
      }
      image.at(x, y) = sum / float(spp);
    }
  });
  return image;
}

static double mean_luminance(const Image& image) {
  double sum = 0;
  for(const vec3& c : image.px) {
    sum += std::isfinite(luminance(c)) ? double(luminance(c)) : 0.0;
  }
  return sum / double(image.px.size());
}

static double rmse(const Image& image, const Image& reference) {
  return masked_rmse(image, reference, 1.0f);
}

int main(int argc, char** argv) {
  Args args(argc, argv);
  int spp = args.integer("--spp", 16);
  PtOptions opt;
  opt.max_depth = args.integer("--depth", 64);
  opt.nee = !args.flag("--no-nee");
  opt.rr = !args.flag("--no-rr");
  EPS = args.number("--eps", EPS);
  int seed = args.integer("--seed", 0);
  bool sweeping = args.flag("--sweep");
  int threads = thread_count(args.integer("--threads", 0));
  std::string out = args.text("--out", "out/pathtrace.png");
  args.finish(USAGE);
  if(spp < 1 || opt.max_depth < 1 || seed < 0) {
    std::fprintf(stderr, "error: --spp and --depth must be at least 1, --seed at least 0\n");
    return 2;
  }

  PtScene scene = cornell_box();
  touch_tasks(scene);
  Image reference;
  bool have_reference = read_pfm(project_path(REFERENCE), reference) && reference.w == SIZE && reference.h == SIZE;

  row("scene", format("Cornell box, %d triangles, %d x %d", int(scene.mesh.tris.size()), SIZE, SIZE));
  row("options", format("depth %d, next event estimation %s, Russian roulette %s, eps %g", opt.max_depth,
                        opt.nee ? "on" : "off", opt.rr ? "on" : "off", double(EPS)));
  row("threads", format("%d", threads));
  if(!have_reference) {
    row("reference", std::string(REFERENCE) + " not found or not 256 x 256: RMSE skipped");
  }

  if(sweeping) {
    std::printf("  %6s %11s %10s %16s   (RMSE without the lamp's pixels)\n", "spp", "time ms", "RMSE",
                "RMSE x sqrt(spp)");
    std::vector<double> xs, ys;
    for(int n : {1, 4, 16, 64, 256}) {
      Timer timer;
      Image image = render(scene, n, opt, threads, seed);
      double ms = timer.ms();
      if(!have_reference) {
        std::printf("  %6d %11.1f %10s %16s\n", n, ms, "-", "-");
        continue;
      }
      double e = rmse(image, reference);
      std::printf("  %6d %11.1f %10.4f %16.4f\n", n, ms, e, e * std::sqrt(double(n)));
      if(e > 0 && std::isfinite(e)) {
        xs.push_back(std::log(double(n)));
        ys.push_back(std::log(e));
      }
    }
    if(xs.size() >= 2) {
      double mx = 0, my = 0, sxy = 0, sxx = 0;
      for(size_t i = 0; i < xs.size(); ++i) {
        mx += xs[i] / double(xs.size());
        my += ys[i] / double(ys.size());
      }
      for(size_t i = 0; i < xs.size(); ++i) {
        sxy += (xs[i] - mx) * (ys[i] - my);
        sxx += (xs[i] - mx) * (xs[i] - mx);
      }
      row("slope", format("%.3f  (log RMSE against log spp)", sxy / sxx));
    }
    print_tasks_not_done();
    return 0;
  }

  stats::reset();
  Timer timer;
  Image image = render(scene, spp, opt, threads, seed);
  double ms = timer.ms();
  stats::Totals totals = stats::totals();
  bool pfm = out.size() > 4 && out.substr(out.size() - 4) == ".pfm";
  bool saved = pfm ? write_pfm(project_path(out), image) : write_png(project_path(out), image, Tone::filmic, EXPOSURE);

  row("spp", format("%d", spp));
  row("time", format("%.1f ms", ms));
  row("rays / sample", format("%.2f", double(totals.rays) / (double(SIZE) * SIZE * spp)));
  if(have_reference) {
    row("mean", format("%.4f  (reference %.4f)", mean_luminance(image), mean_luminance(reference)));
    int used = 0;
    double e = masked_rmse(image, reference, 1.0f, &used);
    row("RMSE", format("%.4f  against the 4096 spp reference, on the %.1f%% of the pixels away from the lamp", e,
                       100.0 * used / (double(SIZE) * SIZE)));
  } else {
    row("mean", format("%.4f", mean_luminance(image)));
  }
  row("saved", saved ? out : "FAILED to write " + out);
  print_tasks_not_done();
  return saved ? 0 : 1;
}
