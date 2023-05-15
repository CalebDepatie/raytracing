#include <iostream>
#include <cstdlib>
#include <cmath>

#include "ray.hpp"
#include "point.hpp"
#include "../common.hpp"

[[nodiscard]]
point ray::p(pos_type t) {
  // [[assume(t>0)]]
  return this->e + (this->d * t);
}

auto rayDir(pos_type fov, pos_type x, pos_type y) -> ray {
  pos_type halfFov = tan(toRad(90.0 - fov*0.5));
  pos_type ypart = HEIGHT * 0.5 * halfFov;

  return ray(
    point(0,0,0),
    point(x-(WIDTH/2.0), ypart, -(y-HEIGHT/2.0))
  );
}
