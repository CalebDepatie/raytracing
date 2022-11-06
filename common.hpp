#include <random>

constexpr int WIDTH = 128*4;
constexpr int HEIGHT = 128*4;

constexpr int MAX_RAY_DEPTH_PER_PIXEL = 4;
constexpr int INITIAL_RAYS_PER_PIXEL = 4;

enum trace_type {
  test,
  path,
  distributed,
};

constexpr trace_type TYPE = distributed;

inline auto random_double() -> double {
  static std::uniform_real_distribution<double> distribution(0.0, 1.0);
  static std::mt19937 generator;
  return distribution(generator);
}

inline auto random_double(double min, double max) -> double {
  return min + (max-min)*random_double();
}

inline auto toRad(pos_type deg) -> pos_type {
  return deg * 2.0 * 3.14159265358979323 / 360.0;
}
