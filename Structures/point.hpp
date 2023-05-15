#pragma once

#include <iostream>
#include <CL/opencl.hpp>

using pos_type = double;

struct point {
  pos_type x;
  pos_type y;
  pos_type z;

  constexpr point() : x(0.0), y(0.0), z(0.0) {};
  constexpr point(pos_type x, pos_type y, pos_type z) : x(x), y(y), z(z) {};

  pos_type length_squared() const;
  pos_type length() const;
  point norm() const;
  cl_float3 toFloat3() const;

  point& operator+=(const point& p);

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

#pragma omp declare reduction(pointAdd : point : omp_out += omp_in)
