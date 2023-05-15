#include "objects.hpp"
#include <cmath>

[[nodiscard]]
pos_type sphere::f(point p) {
  return pow(this->centre.x-p.x, 2) + pow(this->centre.y-p.y,2) + pow(this->centre.z-p.z,2) - pow(this->radius,2);
}

[[nodiscard]]
hit sphere::intersect(ray r) const {
  const point oc = r.e - this->centre;
  const pos_type a = dot(r.d, r.d);
  const pos_type b = 2.0 * dot(oc, r.d);
  const pos_type c = dot(oc, oc) - (this->radius*this->radius);
  const pos_type d = b*b - 4*a*c;

  if (d < 0) {
    return hit{.intersect=false};
  }

  const pos_type t = (-b - sqrt(d)) / (2.0*a);

  point pos = r.p(t);

  return hit{
    .intersect = t>0,
    .depth = t,
    .pos = pos,
    .normal = (pos - this->centre).norm()
  };
}

[[nodiscard]]
hit plane::intersect(ray r) const {
  const pos_type t = dot(this->vertex - r.e, this->normal) / dot(r.d, this->normal);

  // return t;
  auto pos = r.p(t);

  return hit{
    .intersect = t>0,
    .depth = t,
    .pos = pos,
    .normal = this->normal.norm()
  };
}
