#pragma once

#include <CL/opencl.hpp>

struct cl_Ray {
  cl_float3 origin;
  cl_float3 direction;
};

struct cl_Material {
  cl_float3 colour;
  cl_float spec;
  cl_float diff;
};

struct cl_Obj {
  cl_float3 pos;
  int type;
  cl_float3 params;
};
