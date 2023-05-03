#pragma once

#include <CL/opencl.hpp>

struct Ray {
  cl_float3 origin;
  cl_float3 direction;
};

struct Material {
  cl_float3 colour;
  cl_float spec;
  cl_float diff;
};

struct Obj {
  cl_float3 pos;
  Material mat;
  int type;
  cl_float3 params;
};
