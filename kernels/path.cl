// -- Types --

typedef struct Ray {
  float3 origin;
  float3 direction;
} Ray;

typedef struct Material {
  float3 colour;
  float spec;
  float diff;
} Material;

typedef struct Obj {
  float3 pos;
  int type;
  float3 params;
} Obj;

// -- Helper Functions --

// -- Main Path Tracing --

__kernel void pathTrace(
  //__global Obj* scene,
  //__global Material* mats,
  __constant int* sceneLen,
  __constant int* imgLen,
  //__global Ray* rays,
  __global float* image_x,
  __global float* image_y,
  __global float* image_z
) {
  int row = get_global_id(0);
  int col = get_global_id(1);
  int index = row * (*imgLen) + col;

  //Ray cur_ray = rays[index];

  // check intersections

  // update ray and pixel
  image_x[index] = 0.5;
  image_y[index] = 0.5;
  image_z[index] = 0.5;
}
