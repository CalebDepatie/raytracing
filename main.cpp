#define CL_HPP_TARGET_OPENCL_VERSION 300

#include <vector>
#include <array>
#include <string>
#include <iostream>
#include <sstream>
#include <memory>
#include <cstdlib>
#include <cmath>
#include <omp.h>
#include <CL/opencl.hpp>

#include "common.hpp"
#include "ray.hpp"
#include "objects.hpp"
#include "EasyBMP.hpp"
#include "clStructs.hpp"

// ray tracing in one weekend consulted for path tracing
// https://raytracing.github.io/

using array_t = std::unique_ptr<std::array<std::array<point, HEIGHT>, WIDTH>>;

auto createScene() -> std::vector<std::shared_ptr<object>>;
auto saveImage(array_t image) -> void;
auto loadKernel(std::string file) -> std::string;
auto checkErr(std::string ctx, cl_int err) -> void;
auto checkBuildErr(cl::Program prog, cl_int err) -> void;
auto pathTrace(std::vector<std::shared_ptr<object>> scene) -> array_t;
auto distTrace(std::vector<std::shared_ptr<object>> scene) -> array_t;
auto pathCL(std::vector<std::shared_ptr<object>> scene) -> array_t;

// openCL globals
cl::Device device;
cl::Context context;

auto main() -> int {
  // set up the scene
  const auto scene = createScene();
  array_t image;

  // setup openCL
  if constexpr(EXEC == opencl) {

    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);

    if (platforms.size() < 1) {
      std::cerr << "No openCL platforms detected, cannot run" << std::endl;
      exit(-1);
    }

    std::vector<cl::Device> devices;
    platforms[0].getDevices(CL_DEVICE_TYPE_ALL, &devices);

    if (devices.size() < 1) {
      std::cerr << "No openCL devices detected, cannot run" << std::endl;
      exit(-1);
    }

    device = devices[0];
    context = cl::Context(device);
  }

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

        // const ray r = rayDir(90.0, x, y);
        // (*image)[x][y] = rayCast(r, scene, MAX_RAY_DEPTH_PER_PIXEL)*255;
      }
    }

    // test openCL
    if constexpr(EXEC == opencl) {
      // do vec_add
      std::string vec_add = loadKernel("./kernels/vec_add.cl");

      cl::Program prog(context, vec_add.c_str());
      cl_int result = prog.build({device}, "");
      checkBuildErr(prog, result);

      cl::Kernel kernel(prog, "vecAdd");

      const int elems = 10;
      float arrayA[elems] = {0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0};
      float arrayB[elems] = {0.5, 1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5, 8.5, 9.5};
      float arrayAB[elems] = {};

      const int bufSize = elems * sizeof(cl_float);

      cl::Buffer bufA(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, bufSize, arrayA);
      cl::Buffer bufB(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, bufSize, arrayB);
      cl::Buffer bufAB(context, CL_MEM_WRITE_ONLY, bufSize);

      kernel.setArg(0, bufA);
      kernel.setArg(1, bufB);
      kernel.setArg(2, bufAB);

      cl::CommandQueue queue(context, device);

      std::size_t global_work_size = elems;
      std::size_t local_work_size = 10;

      result = queue.enqueueNDRangeKernel(kernel, 0, global_work_size, local_work_size);
      checkErr("Could not enqueue Kernel: ", result);

      result = queue.enqueueReadBuffer(bufAB, CL_TRUE, 0, bufSize, arrayAB);
      checkErr("Could not enqueue read: ", result);

      for (int i=0; i<elems; i++)
        std::cout << arrayA[i] << " + " << arrayB[i] << " = " << arrayAB[i] << std::endl;

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

    omp_set_num_threads(12);

    // collapse only valid when width == height
    #pragma omp parallel for collapse(2)
    for (int x = 0; x<WIDTH; x++) {
      for (int y = 0; y<HEIGHT; y++) {
        point pixel = point(0,0,0);

        // scatter within pixel
        #pragma omp parallel for reduction(pointAdd : pixel)
        for (int ray_i = 0; ray_i < INITIAL_RAYS_PER_PIXEL; ray_i++) {

          const ray r = rayDir(90.0, x+random_double(-0.5,0.5), y+random_double(-0.5,0.5));

          pixel += rayCast(r, scene, MAX_RAY_DEPTH_PER_PIXEL);
        }

        //
        (*image)[x][y] = (pixel/(INITIAL_RAYS_PER_PIXEL))*255;
      }
    }

  } else if constexpr(EXEC==opencl) {
    std::string vec_add = loadKernel("./kernels/path.cl");

    cl::Program prog(context, vec_add.c_str());
    cl_int result = prog.build({device}, "");
    checkBuildErr(prog, result);

    cl::Kernel kernel(prog, "pathTrace");

    // setup kernel params
    const int len = WIDTH*HEIGHT;

    cl_float3* imageOut = (cl_float3*) malloc(len * sizeof(cl_float3));

    // cl::Buffer objBuf(context, CL_MEM_READ_ONLY, 0);
    // cl::Buffer matBuf(context, CL_MEM_READ_ONLY, 0);
    // cl::Buffer rayBuf(context, CL_MEM_READ_WRITE, 0);
    cl::Buffer imageBuf(context, CL_MEM_WRITE_ONLY, len*sizeof(cl_float3), NULL, &result);
    checkErr("Could not make output buffer: ", result);

    cl_int sceneLen = 0;
    cl_int imgLen = WIDTH;

    // kernel.setArg(0, objBuf);
    // kernel.setArg(1, matBuf);
    result = kernel.setArg(0, &sceneLen);
    checkErr("Could not set kernel arg: ", result);

    result = kernel.setArg(1, &imgLen);
    checkErr("Could not set kernel arg: ", result);

    // kernel.setArg(4, rayBuf);
    result = kernel.setArg(2, imageBuf);
    checkErr("Could not set kernel arg: ", result);

    // execute tracing
    cl::CommandQueue queue(context, device);

    cl::NDRange global_work_size(HEIGHT,WIDTH);
    cl::NDRange local_work_size(1,1);

    result = queue.enqueueNDRangeKernel(kernel, 0, global_work_size, local_work_size);
    checkErr("Could not enqueue Kernel: ", result);

    // read and paste image
    result = queue.enqueueReadBuffer(imageBuf, CL_TRUE, 0, len*sizeof(cl_float3), imageOut);
    checkErr("Could not enqueue read: ", result);

    for (int row=0; row<WIDTH; row++) {
      for (int col=0; col<HEIGHT; col++) {
        int index = row * imgLen + col;
        (*image)[row][col].x = imageOut[index].s[0];
        (*image)[row][col].y = imageOut[index].s[1];
        (*image)[row][col].z = imageOut[index].s[2];
      }
    }

    free(imageOut);
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

    omp_set_num_threads(12);

    // collapse only valid when width == height
    #pragma omp parallel for collapse(2)
    for (int x = 0; x<WIDTH; x++) {
      for (int y = 0; y<HEIGHT; y++) {
        point pixel = point(0,0,0);

        // scatter within pixel grid
        #pragma omp parallel for reduction(pointAdd : pixel)
        for (int ray_i = 0; ray_i < GRID_SIZE*GRID_SIZE; ray_i++) {

          const auto [ray_x, ray_y] = get_grid_value(ray_i);

          const ray r = rayDir(90.0, (x+ray_x)-0.5, (y+ray_y)-0.5);
          pixel += rayCast(r, scene, MAX_RAY_DEPTH_PER_PIXEL);
        }

        (*image)[x][y] = (pixel/(GRID_SIZE*GRID_SIZE))*255;
      }
    }

  } else if constexpr(EXEC==opencl) {
    // NYI
  }

  return image;
}

auto checkErr(std::string ctx, cl_int err) -> void {
  if (err) {
    std::cerr << ctx << err << std::endl;
    exit(-1);
  }
}

auto checkBuildErr(cl::Program prog, cl_int err) -> void {
  if (err) {
    std::cerr << "Could not build program: " << err << std::endl;
    char build_log[4096];
    prog.getBuildInfo(device, CL_PROGRAM_BUILD_LOG, build_log);

    std::cerr << "Build log: " << build_log << std::endl;

    exit(-1);
  }
}

auto loadKernel(std::string file) -> std::string {
  std::ifstream kernel_s(file);
  std::stringstream buf;
  buf << kernel_s.rdbuf();
  std::string kernel_src = buf.str();

  return kernel_src;
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
