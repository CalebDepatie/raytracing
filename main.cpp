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

constexpr auto createScene() -> std::vector<object*>;

auto main() -> int {
  // set up the scene
  const auto scene = createScene();

  point image[WIDTH][HEIGHT] = {};

  // tracing
  if constexpr(TYPE == path) {
    for (int x = 0; x<WIDTH; x++) {
      for (int y = 0; y<HEIGHT; y++) {
        point pixel = point(0,0,0);
        int total_rays = 0;

        // scatter within pixel
        for (int ray_i = 0; ray_i < INITIAL_RAYS_PER_PIXEL; ray_i++) {

          const ray r = rayDir(90.0, x+random_double(-0.5,0.5), y+random_double(-0.5,0.5));
          pixel = pixel + rayCast(r, scene, MAX_RAY_DEPTH_PER_PIXEL, total_rays);
        }

        image[x][y] = (pixel/(INITIAL_RAYS_PER_PIXEL))*255;

      }
    }
  } else if constexpr(TYPE == distributed) {
    for (int x = 0; x<WIDTH; x++) {
      for (int y = 0; y<HEIGHT; y++) {
        point pixel = point(0,0,0);
        int total_rays = 0;

        // scatter within pixel grid
        for (int ray_i = 0; ray_i < GRID_SIZE*GRID_SIZE; ray_i++) {

          const auto [ray_x, ray_y] = get_grid_value(ray_i);//get_grid_value(static_cast<int>(random_double(0, (GRID_SIZE*GRID_SIZE)-1)));
          // std::cout << ray_i << " " << x << " " << y << std::endl;
          // const ray r = rayDir(90.0, x+random_double(-0.5,0.5), y+random_double(-0.5,0.5));
          const ray r = rayDir(90.0, (x+ray_x)-0.5, (y+ray_y)-0.5);
          pixel = pixel + rayCast(r, scene, MAX_RAY_DEPTH_PER_PIXEL, total_rays);
        }

        // std::cout << total_rays << std::endl;
        image[x][y] = (pixel/(GRID_SIZE*GRID_SIZE))*255;

      }
      std::cout << "line " << x << " complete" << std::endl;
    }
  } else if constexpr(TYPE == test) {
    // shoot 1 ray per pixel for intersection testing
    for (int x = 0; x<WIDTH; x++) {
      for (int y = 0; y<HEIGHT; y++) {

        const ray r = rayDir(90.0, x, y);
        int total_rays = 0;
        // todo: async
        image[x][y] = rayCast(r, scene, MAX_RAY_DEPTH_PER_PIXEL, total_rays)*255;

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

constexpr auto createScene() -> std::vector<object*> {
  std::vector<object*> scene = std::vector<object*>();
  scene.emplace_back(new plane({
    point(0,0,-3),
    point(0,0,1),
    point(0.5, 0.5, 0.5),
    0.0,
    1.0
  }));
  scene.emplace_back(new sphere(
    point(0,12,0),
    5.0,
    point(1, 0, 0),
    0.5,
    1.0
  ));
  scene.emplace_back(new sphere(
    point(15,20,-1),
    3.0,
    point(0, 1, 0),
    0.9, // 0.5,
    0.5  // 0.9
  ));
  scene.emplace_back(new sphere(
    point(-10,15,0),
    5.0,
    point(0, 0, 1),
    0.3,
    0.5
  ));

  scene.emplace_back(new sphere(
    point(-5,10,-2),
    1.0,
    point(0.3, 0.3, 1),
    0.3,
    1.0
  ));
  scene.emplace_back(new sphere(
    point(3,5,-2),
    1.0,
    point(0.3, 1, 0.3),
    0.3,
    0.8
  ));
  scene.emplace_back(new sphere(
    point(-7,8,-2),
    1.0,
    point(0.5, 0.7, 1),
    0.8,
    1.0
  ));
  scene.emplace_back(new sphere(
    point(-1,3,-2),
    1.0,
    point(0.9, 0.3, 1),
    0.3,
    0.3
  ));
  scene.emplace_back(new sphere(
    point(13,17,-2),
    1.0,
    point(0.6, 0.5, 1),
    0.5,
    1.0
  ));

  return scene;
}
