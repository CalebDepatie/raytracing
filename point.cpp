#include "point.hpp"
#include <cmath>

pos_type point::length_squared() const {
  return this->x*this->x + this->y*this->y + this->z*this->z;
}

pos_type point::length() const {
  return sqrt(this->length_squared());
}

point point::norm() const {
  return *this / this->length();
}

point& point::operator+=(const point& p) {
  this->x += p.x;
  this->y += p.y;
  this->z += p.z;

  return *this;
}

point operator+(point p1, point p2) {
  return point(p1.x+p2.x, p1.y+p2.y, p1.z+p2.z);
}

point operator+(point p, pos_type scalar) {
  return point(p.x+scalar, p.y+scalar, p.z+scalar);
}

point operator-(point p1, point p2) {
  return point(p1.x-p2.x, p1.y-p2.y, p1.z-p2.z);
}

point operator-(point p, pos_type scalar) {
  return point(p.x-scalar, p.y-scalar, p.z-scalar);
}

point operator*(point p1, point p2) {
  return point(p1.x*p2.x, p1.y*p2.y, p1.z*p2.z);
}

point operator*(point p, pos_type scalar) {
  return point(p.x*scalar, p.y*scalar, p.z*scalar);
}

point operator/(point p1, point p2) {
  return point(p1.x/p2.x, p1.y/p2.y, p1.z/p2.z);
}

point operator/(point p, pos_type scalar) {
  return point(p.x/scalar, p.y/scalar, p.z/scalar);
}

std::ostream& operator<<(std::ostream &out, const point& p) {
  return out << p.x << " " << p.y << " " << p.z;
}

pos_type dot(const point p1, const point p2) {
  return (p1.x*p2.x) + (p1.y*p2.y) + (p1.z*p2.z);
}

point cross(const point p1, const point p2) {
  const point n1 = p1.norm();
  const point n2 = p2.norm();
  return point(
    n1.y*n2.z - n1.z*n2.y,
    n1.z*n2.x - n1.x*n2.z,
    n1.x*n2.y - n1.y*n2.x
  );
}
