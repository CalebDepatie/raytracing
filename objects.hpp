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
  pos_type specular = 0.5;
  pos_type diffuse = 1.0;
  // object();
  constexpr virtual ~object() {};
  constexpr object(point c) : colour(c) {};
  constexpr object(point c, pos_type spec, pos_type dif)
    : colour(c), specular(spec), diffuse(dif) {};
  virtual hit intersect(ray r) const = 0;
};

// x, y, z, corresponds to centre
class sphere : public object {
public:
  point centre;
  pos_type radius;
  constexpr sphere(point c, pos_type r, point colour)
    : object(colour), centre(c), radius(r) {};
  constexpr sphere(point c, pos_type r, point colour, pos_type spec, pos_type dif)
    : object(colour, spec, dif), centre(c), radius(r) {};
  constexpr ~sphere() {};

  hit intersect(ray r) const;
  pos_type f(point p);
};

class plane : public object  {
public:
  point vertex;
  point normal;

  constexpr plane(point v, point n, point colour)
    : object(colour), vertex(v), normal(n) {};
    constexpr plane(point v, point n, point colour, pos_type spec, pos_type dif)
      : object(colour, spec, dif), vertex(v), normal(n) {};
  constexpr ~plane() {};

  hit intersect(ray r) const;
};
