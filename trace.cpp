#include "trace.hpp"

#include "common.hpp"
#include "Structures/objects.hpp"
#include "Structures/ray.hpp"

auto lightRay(point startpos, const std::vector<std::shared_ptr<object>> scene, int bounces) -> point {
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
    }
  }

  return colour/bounces;
}

// returns colour
auto rayCast(ray r, const std::vector<std::shared_ptr<object>> scene, int bounces) -> point {
  auto nearest = std::weak_ptr<object>();
  hit nearesthit;
  pos_type depth = 0;
  point colour = point(0.1,0.1,0.2);

  //Loop through every object
  for (auto& obj : scene) {
    auto objhit = obj->intersect(r);
    //Check if ray intersected
    if (objhit.intersect) {
      //Check if this new intersection is the closest to to the eye
      //If this is the first iteration, nearest==false and this should run regardless
      std::shared_ptr<object> nearest_ptr = nearest.lock();
      if (!nearest_ptr || objhit.depth < depth) {
        //New nearest
        depth = objhit.depth;
        nearest = obj;
        nearesthit = objhit;
      }
    }
  }

  std::shared_ptr<object> nearest_ptr = nearest.lock();
  if (nearest_ptr && nearesthit.depth >= 0.001) {
    //Object base colour
    colour = nearest_ptr->colour;

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
        auto light_colour = lightRay(startpos, scene, bounces+1);
        bounces = GRID_SIZE*GRID_SIZE;

        for (int bounce = 0; bounce < bounces; bounce++) {
          const auto [ray_x, ray_y] = get_grid_value(bounce);

          auto nr = ray(
            (nearesthit.pos + point(ray_x, ray_y, 0)) - point(0.5,0.5,0),
            (reflection * (1-nearest_ptr->diffuse)) + (diffuse * nearest_ptr->diffuse)
          );
          reflection_colour += rayCast(nr, scene, 0);
        }

        reflection_colour = reflection_colour / bounces;
        reflection_colour = colour*(1.0-nearest_ptr->specular)
                            + reflection_colour*nearest_ptr->specular;

        colour = (colour + light_colour + reflection_colour) / 3;

      } else if constexpr(TYPE==path) {
        // if first intersection
        if (bounces == MAX_RAY_DEPTH_PER_PIXEL) {
          // choose reflection or shadow ray
          const bool shadow = static_cast<bool>(static_cast<int>(random_double(0,2)));
          if (shadow) {
            light_colour = lightRay(startpos, scene, 1);

            colour = (colour + light_colour) / 2;
            return colour;
          }
        }

        // reflection
        auto nr = ray(
          nearesthit.pos,
          (reflection * (1-nearest_ptr->diffuse)) + (diffuse * nearest_ptr->diffuse)
        );

        reflection_colour += rayCast(nr, scene, bounces-1);

        colour = colour*(1.0-nearest_ptr->specular) + reflection_colour*nearest_ptr->specular;
      } else {
        // test tracing section

        // bounces again correspondes to grids
        point reflection_colour;

        if constexpr(EXEC == openmp) {

          #pragma omp parallel for reduction(pointAdd : reflection_colour)
          for (int bounce = 0; bounce < bounces; bounce++) {
            const auto [ray_x, ray_y] = get_grid_value(static_cast<int>(random_double(0, (GRID_SIZE*GRID_SIZE)-1)));

            auto nr = ray(
              (nearesthit.pos + point(ray_x, ray_y, 0)) - point(0.5,0.5,0),
              nearesthit.normal + random_dir()
            );

            reflection_colour += rayCast(nr, scene, bounces-1);
          }

        } else {

          for (int bounce = 0; bounce < bounces; bounce++) {
            const auto [ray_x, ray_y] = get_grid_value(static_cast<int>(random_double(0, (GRID_SIZE*GRID_SIZE)-1)));

            auto nr = ray(
              (nearesthit.pos + point(ray_x, ray_y, 0)) - point(0.5,0.5,0),
              nearesthit.normal + random_dir()
            );

            reflection_colour += rayCast(nr, scene, bounces-1);
          }
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
