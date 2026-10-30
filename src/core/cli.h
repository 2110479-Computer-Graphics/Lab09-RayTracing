#pragma once

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <set>
#include <string>

inline std::string project_path(const std::string& relative) {
  bool absolute = !relative.empty() && (relative[0] == '/' || (relative.size() > 1 && relative[1] == ':'));
  return absolute ? relative : std::string(LAB09_ROOT) + "/" + relative;
}

template <typename... T>
std::string format(const char* fmt, T... args) {
  char buffer[256];
  std::snprintf(buffer, sizeof buffer, fmt, args...);
  return buffer;
}

inline void row(const std::string& label, const std::string& value) {
  std::printf("  %-15s%s\n", label.c_str(), value.c_str());
}

struct Timer {
  std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

  double ms() const {
    return std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
  }
};

class Args {
public:
  Args(int argc, char** argv) {
    for(int i = 1; i < argc; ++i) {
      std::string name = argv[i];
      if(name.rfind("--", 0) != 0) {
        unknown.insert(name);
        continue;
      }
      bool has_value = i + 1 < argc && std::string(argv[i + 1]).rfind("--", 0) != 0;
      values[name] = has_value ? argv[++i] : "";
    }
  }

  bool flag(const std::string& name) {
    used.insert(name);
    return values.count(name) > 0;
  }

  std::string text(const std::string& name, const std::string& fallback) {
    return flag(name) ? values[name] : fallback;
  }

  float number(const std::string& name, float fallback) {
    if(!flag(name)) {
      return fallback;
    }
    char* end = nullptr;
    float value = std::strtof(values[name].c_str(), &end);
    if(values[name].empty() || *end != '\0') {
      fail(name + " needs a number, got '" + values[name] + "'");
    }
    return value;
  }

  int integer(const std::string& name, int fallback) {
    float value = number(name, float(fallback));
    if(value != float(int(value))) {
      fail(name + " needs a whole number, got '" + values[name] + "'");
    }
    return int(value);
  }

  void finish(const char* usage) {
    for(const auto& entry : values) {
      if(!used.count(entry.first)) {
        unknown.insert(entry.first);
      }
    }
    if(!unknown.empty()) {
      fail("unknown option " + *unknown.begin() + "\n" + usage);
    }
  }

private:
  std::map<std::string, std::string> values;
  std::set<std::string> used, unknown;

  [[noreturn]] static void fail(const std::string& message) {
    std::fprintf(stderr, "error: %s\n", message.c_str());
    std::exit(2);
  }
};
