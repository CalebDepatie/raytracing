#include <vector>
#include <array>
#include <string>
#include <iostream>
#include <memory>
#include <cstdlib>
#include <cmath>

#include "ray.hpp"
#include "objects.hpp"
#include "EasyBMP.hpp"
#include "common.hpp"

// ray tracing in one weekend consulted for path tracing
// https://raytracing.github.io/

using array_t = std::unique_ptr<std::array<std::array<point, HEIGHT>, WIDTH>>;

auto createScene() -> std::vector<std::shared_ptr<object>>;
auto saveImage(array_t image) -> void;
auto pathTrace(std::vector<std::shared_ptr<object>> scene) -> array_t;
auto distTrace(std::vector<std::shared_ptr<object>> scene) -> array_t;

auto main() -> int {
  // set up the scene
  const auto scene = createScene();
  array_t image;

  // tracing
  if constexpr(TYPE == path) {
    image = pathTrace(scene);

  } else if constexpr(TYPE == distributed) {
    image = distTrace(scene);

  } else if constexpr(TYPE == test) {
    image = std::make_unique<std::array<std::array<point, HEIGHT>, WIDTH>>();

    // shoot 1 ray per pixel for intersection testing
    for (int x = 0; x<WIDTH; x++) {
      for (int y = 0; y<HEIGHT; y++) {

        const ray r = rayDir(90.0, x, y);
        (*image)[x][y] = rayCast(r, scene, MAX_RAY_DEPTH_PER_PIXEL)*255;
      }
    }
  }

  saveImage(std::move(image));

  return 0;
}

auto pathTrace(std::vector<std::shared_ptr<object>> scene) -> array_t {
  auto image = std::make_unique<std::array<std::array<point, HEIGHT>, WIDTH>>();

  if constexpr(EXEC==seq) {

    for (int x = 0; x<WIDTH; x++) {
      for (int y = 0; y<HEIGHT; y++) {
        point pixel = point(0,0,0);

        // scatter within pixel
        for (int ray_i = 0; ray_i < INITIAL_RAYS_PER_PIXEL; ray_i++) {

          const ray r = rayDir(90.0, x+random_double(-0.5,0.5), y+random_double(-0.5,0.5));
          pixel = pixel + rayCast(r, scene, MAX_RAY_DEPTH_PER_PIXEL);
        }

        (*image)[x][y] = (pixel/(INITIAL_RAYS_PER_PIXEL))*255;
      }
    }

  } else if constexpr(EXEC==openmp) {
    // NYI

  } else if constexpr(EXEC==opencl) {
    // NYI
  }

  return image;
}

auto distTrace(std::vector<std::shared_ptr<object>> scene) -> array_t {
  auto image = std::make_unique<std::array<std::array<point, HEIGHT>, WIDTH>>();

  if constexpr(EXEC==seq) {
    for (int x = 0; x<WIDTH; x++) {
      for (int y = 0; y<HEIGHT; y++) {
        point pixel = point(0,0,0);

        // scatter within pixel grid
        for (int ray_i = 0; ray_i < GRID_SIZE*GRID_SIZE; ray_i++) {

          const auto [ray_x, ray_y] = get_grid_value(ray_i);

          const ray r = rayDir(90.0, (x+ray_x)-0.5, (y+ray_y)-0.5);
          pixel = pixel + rayCast(r, scene, MAX_RAY_DEPTH_PER_PIXEL);
        }

        (*image)[x][y] = (pixel/(GRID_SIZE*GRID_SIZE))*255;
      }
    }

  } else if constexpr(EXEC==openmp) {
    // NYI
  } else if constexpr(EXEC==opencl) {
    // NYI
  }

  return image;
}

auto saveImage(array_t image) -> void {
  auto output = EasyBMP::Image(WIDTH, HEIGHT, "output.bmp");

  for (int x = 0; x<WIDTH; x++) {
    for (int y = 0; y<HEIGHT; y++) {
      output.SetPixel(x, y, EasyBMP::RGBColor((*image)[x][y].x,
                                              (*image)[x][y].y,
                                              (*image)[x][y].z));
    }
  }

  output.Write();
}

auto createScene() -> std::vector<std::shared_ptr<object>> {
  auto scene = std::vector<std::shared_ptr<object>>();

  scene.push_back(std::make_shared<plane>(
    point(0,0,-3),
    point(0,0,1),
    point(0.5, 0.5, 0.5),
    0.0,
    1.0
  ));
  scene.push_back(std::make_shared<sphere>(
    point(0,12,0),
    5.0,
    point(1, 0, 0),
    0.5,
    1.0
  ));
  scene.push_back(std::make_shared<sphere>(
    point(15,20,-1),
    3.0,
    point(0, 1, 0),
    0.9, // 0.5,
    0.5  // 0.9
  ));
  scene.push_back(std::make_shared<sphere>(
    point(-10,15,0),
    5.0,
    point(0, 0, 1),
    0.3,
    0.5
  ));

  scene.push_back(std::make_shared<sphere>(
    point(-5,10,-2),
    1.0,
    point(0.3, 0.3, 1),
    0.3,
    1.0
  ));
  scene.push_back(std::make_shared<sphere>(
    point(3,5,-2),
    1.0,
    point(0.3, 1, 0.3),
    0.3,
    0.8
  ));
  scene.push_back(std::make_shared<sphere>(
    point(-7,8,-2),
    1.0,
    point(0.5, 0.7, 1),
    0.8,
    1.0
  ));
  scene.push_back(std::make_shared<sphere>(
    point(-1,3,-2),
    1.0,
    point(0.9, 0.3, 1),
    0.3,
    0.3
  ));
  scene.push_back(std::make_shared<sphere>(
    point(13,17,-2),
    1.0,
    point(0.6, 0.5, 1),
    0.5,
    1.0
  ));

  return scene;
}
