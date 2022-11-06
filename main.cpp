#include <vector>
#include <iostream>
#include <cstdlib>
#include <cmath>

#include "ray.hpp"
#include "objects.hpp"
#include "EasyBMP.hpp"
#include "common.hpp"

// ray tracing in one weekend consulted for path tracing
// https://raytracing.github.io/

auto createScene() -> std::vector<object*>;
// auto random_double() -> double;
// auto random_double(double min, double max) -> double;
// auto toRad(pos_type deg) -> pos_type;

auto main() -> int {
  // set up the scene
  const auto scene = createScene();

  point image[WIDTH][HEIGHT] = {};

  // tracing
  if constexpr(TYPE == path) {


  } else if constexpr(TYPE == distributed) {
    for (int x = 0; x<WIDTH; x++) {
      for (int y = 0; y<HEIGHT; y++) {
        // scatter within pixel
        point pixel = point(0,0,0);
        for (int ray_i = 0; ray_i < INITIAL_RAYS_PER_PIXEL; ray_i++) {

          const ray r = rayDir(90.0, x+random_double(-0.5,0.5), y+random_double(-0.5,0.5));
          pixel = pixel + rayCast(r, scene, MAX_RAY_DEPTH_PER_PIXEL);
        }

        image[x][y] = (pixel/INITIAL_RAYS_PER_PIXEL)*255;

      }
    }
  } else if constexpr(TYPE == test) {
    // shoot 1 ray per pixel for intersection testing
    for (int x = 0; x<WIDTH; x++) {
      for (int y = 0; y<HEIGHT; y++) {

        const ray r = rayDir(90.0, x, y);

        // todo: async
        image[x][y] = rayCast(r, scene, MAX_RAY_DEPTH_PER_PIXEL)*255;

      }
    }
  }

  // save image
  auto output = EasyBMP::Image(WIDTH, HEIGHT, "output.bmp");
  for (int x = 0; x<WIDTH; x++) {
    for (int y = 0; y<HEIGHT; y++) {
      output.SetPixel(x, y, EasyBMP::RGBColor(image[x][y].x, image[x][y].y, image[x][y].z));
    }
  }
  output.Write();

  // cleanup
  for (size_t i = 0; i<scene.size(); i++)
    delete scene[i];

  return 0;
}

auto createScene() -> std::vector<object*> {
  std::vector<object*> scene = std::vector<object*>();
  scene.emplace_back(new plane({
    point(0,0,-3),
    point(0,0,1),
    point(0.5, 0.5, 0.5)
  }));
  scene.emplace_back(new sphere(
    point(0,10,0),
    5.0,
    point(1, 0, 0)
  ));
  scene.emplace_back(new sphere(
    point(15,20,-1),
    3.0,
    point(0, 1, 0)
  ));
  scene.emplace_back(new sphere(
    point(-10,15,0),
    5.0,
    point(0, 0, 1)
  ));

  return scene;
}
