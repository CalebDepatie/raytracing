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

auto lightRay(point startpos, const std::vector<object*>& scene, int bounces, int& total_rays) -> point {
  // auto light = point(5,5,30);
  // auto light_ray = ray(
  //   intersection,
  //   (intersection - light).norm()
  // );
  point colour = point(0,0,0);

  if constexpr(TYPE==distributed) {
    bounces = GRID_SIZE*GRID_SIZE;
  }

  for (int i=0; i<(bounces); i++) {
    point endpos;

    if constexpr(TYPE==distributed) {
      int grid_section = i;
      double increments = 15.0 / GRID_SIZE;
      int x = grid_section%GRID_SIZE;
      int y = grid_section/GRID_SIZE;

      // centred around x=0.0 y=7.5
      endpos = point((x*increments),(y*increments)+7.5, 40);
    } else {

      endpos = point(random_double(-7.5,7.5), random_double(0.0,15.0), 40);
    }

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
      colour = colour + point(0.9, 0.9, 0.9); //point(0.8,0.1,0.8);
      total_rays++;
    }
  }


  return colour/bounces;
}

// returns colour
auto rayCast(ray r, const std::vector<object*>& scene, int bounces, int& total_rays) -> point {
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
    // lighting ray dir
    auto startpos = nearesthit.pos + nearesthit.normal*0.01;
    point light_colour;

    auto random_dir = [](){
      auto p = point(random_double(-1,1), random_double(-1,1), random_double(-1,1));
      return p.norm();
    };

    if (bounces>0) {
      // reflection
      point reflection_colour = point(0,0,0);

      const pos_type fuzz = 0.8;
      const auto reflection = ((r.d - 2*dot(r.d, nearesthit.normal)) * nearesthit.normal)+(random_dir()*fuzz);
      const auto diffuse = nearesthit.normal + random_dir();

      if constexpr(TYPE==distributed) {
        auto light_colour = lightRay(startpos, scene, bounces+1, total_rays);
        bounces = GRID_SIZE*GRID_SIZE;

        for (int bounce = 0; bounce < bounces; bounce++) {
          const auto [ray_x, ray_y] = get_grid_value(bounce);

          auto nr = ray(
            (nearesthit.pos + point(ray_x, ray_y, 0)) - point(0.5,0.5,0),
            (reflection * (1-nearest->diffuse)) + (diffuse * nearest->diffuse)
          );
          reflection_colour = reflection_colour + rayCast(nr, scene, 0, total_rays);
          total_rays++;
        }

        reflection_colour = reflection_colour / bounces;
        reflection_colour = colour*(1.0-nearest->specular) + reflection_colour*nearest->specular;
        colour = (colour + light_colour + reflection_colour) / 3;
        // colour = (colour + reflection_colour + light_colour) / 3;

      } else if constexpr(TYPE==path) {
        // if first intersection
        if (bounces == MAX_RAY_DEPTH_PER_PIXEL) {
          // choose reflection or shadow ray
          const bool shadow = static_cast<bool>(static_cast<int>(random_double(0,2)));
          if (shadow) {
            light_colour = lightRay(startpos, scene, 1, total_rays);

            colour = (colour + light_colour) / 2;
            return colour;
          }
        }
        // reflection

        auto nr = ray(
          nearesthit.pos,
          (reflection * (1-nearest->diffuse)) + (diffuse * nearest->diffuse)
          // nearesthit.normal + (random_dir() * nearest->diffuse).norm()
          // nearesthit.normal + random_dir()
        );
        total_rays++;
        reflection_colour = reflection_colour + rayCast(nr, scene, bounces, total_rays);
        // colour = (colour + reflection_colour) / 2;

        // auto speclight = lightRay(nearesthit.pos+nearesthit.normal*0.02, scene, 1, total_rays);
        // speclight.x = pow(speclight.x, 10.0);
        // speclight.y = pow(speclight.y, 10.0);
        // speclight.z = pow(speclight.z, 10.0);
        // reflection_colour = speclight*0.5 + reflection_colour*0.5;

        colour = colour*(1.0-nearest->specular) + reflection_colour*nearest->specular;
      } else {
        // bounces again correspondes to grids
        point reflection_colour;
        for (int bounce = 0; bounce < bounces; bounce++) {
          const auto [ray_x, ray_y] = get_grid_value(static_cast<int>(random_double(0, (GRID_SIZE*GRID_SIZE)-1)));

          auto nr = ray(
            (nearesthit.pos + point(ray_x, ray_y, 0)) - point(0.5,0.5,0),
            nearesthit.normal + random_dir()
          );
          reflection_colour = reflection_colour + rayCast(nr, scene, bounces-1, total_rays);
          total_rays++;
        }

        reflection_colour = reflection_colour / bounces;
        colour = (colour + reflection_colour + light_colour) / 3;
      }

    } else {
      if constexpr(TYPE==path) {
        colour = (colour + point(0.8, 0.8, 0.8)) / 2;
      }

    }

  }

  return colour;
}
