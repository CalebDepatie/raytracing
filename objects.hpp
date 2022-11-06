#pragma once
#include "point.hpp"
#include "ray.hpp"

struct hit {
  bool intersect;
  pos_type depth;
  point pos;
  point normal;
};

class object {
public:
  point colour;
  // object();
  virtual ~object() {};
  object(point c) : colour(c) {};
  virtual hit intersect(ray r) const = 0;
};

// x, y, z, corresponds to centre
class sphere : public object {
public:
  point centre;
  pos_type radius;
  sphere(point c, pos_type r, point colour)
    : object(colour), centre(c), radius(r) {};
  ~sphere() {};

  hit intersect(ray r) const;
  pos_type f(point p);
};

class plane : public object  {
public:
  point vertex;
  point normal;

  plane(point v, point n, point colour) : object(colour), vertex(v), normal(n) {};
  ~plane() {};

  hit intersect(ray r) const;
};
