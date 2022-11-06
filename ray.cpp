#include "ray.hpp"
#include "objects.hpp"
#include "point.hpp"
#include "common.hpp"

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

auto lightRay(point startpos, const std::vector<object*>& scene, int bounces) -> point {
  // auto light = point(5,5,30);
  // auto light_ray = ray(
  //   intersection,
  //   (intersection - light).norm()
  // );
  point colour = point(0,0,0);

  for (int i=0; i<bounces; i++) {
    auto endpos = point(random_double(-20,20),random_double(-20,20), 30);
    // std::cout << endpos << std::endl;

    auto light_ray = ray(
      startpos,
      (endpos-startpos).norm()
    );

    bool hitlight = true;
    for (auto& obj : scene) {
      auto objhit = obj->intersect(light_ray);

      if (objhit.intersect) {
        hitlight = false;
        break;
      }
    }

    if (hitlight) {
      colour = colour + point(0.8,0.1,0.8);
    }
  }

  return colour/bounces;
}

// returns colour
auto rayCast(ray r, const std::vector<object*>& scene, int bounces) -> point {
  object* nearest = 0;
  hit nearesthit;
  pos_type depth = 0;
  point colour = point(0.1,0.1,0.2);

  //Loop through every object
  for (auto& obj : scene) {
    auto objhit = obj->intersect(r);
    //Check if ray intersected
    if (objhit.intersect) {
      //Check if this new intersection is the closest to to the eye
      //If this is the first iteratio, nearest==0 and this should run regardless
      if (nearest==0 || objhit.depth < depth) {
        //New nearest
        depth = objhit.depth;
        nearest = obj;
        nearesthit = objhit;
      }
    }
  }

  if (nearest && nearesthit.depth >= 0.001) {
    //Object base colour
    colour = nearest->colour;
    // lighting ray
    auto startpos = nearesthit.pos + nearesthit.normal;
    auto lightColour = lightRay(nearesthit.pos, scene, bounces+1);

    auto random_dir = [](){
      auto p = point(random_double(-1,1), random_double(-1,1), random_double(-1,1));
      return p.norm();
    };

    // if (bounces>0) {
    //   // reflection
    //   point reflection_colour = point(0,0,0);
    //   for (int bounce = 0; bounce < bounces; bounce++) {
    //     auto nr = ray(
    //       nearesthit.pos,
    //       nearesthit.normal + random_dir()
    //     );
    //     reflection_colour = reflection_colour + rayCast(nr, scene, bounces-1);
    //   }
    //   reflection_colour = reflection_colour / bounces;
    //
    //   colour = (colour + lightColour + reflection_colour) / 3;
    //
    // } else {
      colour = (colour + lightColour) / 2;
    // }

    // if (fabs(fmod(nearesthit.pos.x, 2.0))<0.02 || fabs(fmod(nearesthit.pos.y, 2.0))<0.02) {
    //   colour = point(1,1,0);
    // }
  }

  return colour;
}
