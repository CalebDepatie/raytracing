#pragma once

#include <vector>
#include <iostream>
#include <cstdlib>
#include <cmath>

#include "point.hpp"

class object;

struct ray {
  point e;
  point d;

  ray(point start, point dir) : e(start), d(dir) {};
  point p(pos_type t);
};

auto rayDir(pos_type fov, pos_type x, pos_type y) -> ray;
auto rayCast(ray r, const std::vector<object*>& scene, int bounces, int& total_rays) -> point;
