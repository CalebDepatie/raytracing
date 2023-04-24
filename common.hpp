#include <random>
#include <tuple>

constexpr int WIDTH = 128*4;
constexpr int HEIGHT = 128*4;

constexpr int MAX_RAY_DEPTH_PER_PIXEL = 8; // per path is more accurate
constexpr int INITIAL_RAYS_PER_PIXEL = 128;
constexpr int GRID_SIZE = 8;

enum trace_type {
  test,
  path,
  distributed,
};

enum exec_type {
  seq,
  openmp,
  opencl
};

constexpr trace_type TYPE = distributed;
constexpr exec_type EXEC = openmp;

inline constexpr auto get_grid_value(int grid_section) -> std::tuple<double, double> {
  // [[assume(grid_section < GRID_SIZE*GRID_SIZE)]]
  // if a pixel is split into an n by n grid, return the bounds
  double increments = 1.0 / GRID_SIZE;
  int x = grid_section%GRID_SIZE;
  int y = grid_section/GRID_SIZE;

  return std::make_tuple(increments*x,increments*y);
}

inline auto random_double() -> double {
  static std::uniform_real_distribution<double> distribution(0.0, 1.0);
  static std::mt19937 generator;
  return distribution(generator);
}

inline auto random_double(double min, double max) -> double {
  // [[assume(min < max)]]
  return min + (max-min)*random_double();
}

inline constexpr auto toRad(double deg) -> double {
  return deg * 2.0 * 3.14159265358979323 / 360.0;
}
