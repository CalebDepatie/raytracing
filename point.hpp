#pragma once

#include <iostream>

using pos_type = double;

struct point {
  pos_type x;
  pos_type y;
  pos_type z;

  constexpr point() {};
  constexpr point(pos_type x, pos_type y, pos_type z) : x(x), y(y), z(z) {};

  pos_type length_squared() const;
  pos_type length() const;
  point norm() const;

  friend point operator+(point p1, point p2);
  friend point operator+(point p, pos_type scalar);
  friend point operator-(point p1, point p2);
  friend point operator-(point p, pos_type scalar);
  friend point operator*(point p1, point p2);
  friend point operator*(point p, pos_type scalar);
  friend point operator/(point p1, point p2);
  friend point operator/(point p, pos_type scalar);

  friend std::ostream& operator<<(std::ostream &out, const point& p);
};

pos_type dot(const point p1, const point p2);
point cross(const point p1, const point p2);
