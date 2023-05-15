#pragma once

#include "point.hpp"

class object;

struct ray {
  point e;
  point d;

  ray(point start, point dir) : e(start), d(dir) {};
  point p(pos_type t);
};

auto rayDir(pos_type fov, pos_type x, pos_type y) -> ray;
