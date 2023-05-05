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

typedef struct rayHit {
  int intersect;
  float depth;
  float3 pos;
  float3 norm;
} rayHit;

// -- Helper Functions --
rayHit intersect(Obj object, Ray r) {
  rayHit new_hit;

  if (object.type == 0) { // plane
    // in this case, params refers to the plane normal
    float t = dot(object.pos - r.origin, object.params) /
              dot(r.direction, object.params);

    float3 intersection_pos = r.origin + (r.direction * t);

    new_hit.intersect = t>0 ? 1 : 0;
    new_hit.depth = t;
    new_hit.pos = intersection_pos,
    new_hit.norm = normalize(object.params);

  } else if (object.type == 1) { // sphere
    // in this case, params[0] is the sphere radius
    float3 oc = r.origin - object.pos;
    float a = dot(r.direction, r.direction);
    float b = 2.0 * dot(oc, r.direction);
    float c = dot(oc, oc) - (object.params.x * object.params.x);
    float d = b*b - 4*a*c;

    if (d < 0) {
      new_hit.intersect = 0;

    } else {
      float t = (-b - sqrt(d)) / (2.0*a);

      float3 intersection_pos = r.origin + (r.direction * t);

      new_hit.intersect = t>0 ? 1 : 0;
      new_hit.depth = t;
      new_hit.pos = intersection_pos,
      new_hit.norm = normalize(intersection_pos - object.pos);
    }
  }

  return new_hit;
}

float rand(int seed) {
  unsigned int t = (unsigned int)seed ^ ((unsigned int) seed << 11);
  float mid_val = (float) ((t * 0x5DEECE66DL + 0xBL) & ((1L << 48) - 1));
  return fmod(mid_val, 10000.0f) / 10000.0f;
}

float bounded_rand(float min, float max, int seed) {
  return min + (max-min) * rand(seed);
}

float3 random_dir(int seed) {
  //float3 p = (float3)(bounded_rand(-1,1, seed), bounded_rand(-1,1, seed), bounded_rand(-1,1, seed));
  float3 p = (0.5 * (seed >> 14), -0.5 * (seed << 8), 0.2 * (seed >> 4));
  return normalize(p);
}

// -- Main Path Tracing --

__kernel void pathTrace(
  __global Obj* scene,
  __global Material* mats,
  int sceneLen,
  __global Ray* rays,
  int raysPerPixel,
  int seed,
  __global float3* image
) {
  int id = get_global_id(0);
  float3 colour = (float3)(0.1,0.1,0.2);

  for (int ray_i=0; ray_i<raysPerPixel; ray_i++) {
    const int ray_id = id * raysPerPixel + ray_i;

    Ray cur_ray = rays[ray_id];

    rayHit nearest_hit;
    int nearest_obj_i = -1;

    // check intersections
    for (int i=0; i<sceneLen; i++) {
      Obj cur_obj = scene[i];

      rayHit cur_hit = intersect(cur_obj, cur_ray);

      if (cur_hit.intersect == 1) {
        if (nearest_obj_i == -1 || cur_hit.depth < nearest_hit.depth) {
          nearest_obj_i = i;
          nearest_hit = cur_hit;
        }
      }
    }

    Ray next_ray = cur_ray;

    // determining colour

    if (nearest_obj_i != -1) {
      Obj nearest_obj = scene[nearest_obj_i];
      Material nearest_mat = mats[nearest_obj_i];

      //float3 startpos = nearest_hit.pos + nearest_hit.norm*0.01f;

      // relfection rays
      const float fuzz = 0.8;
      const float3 reflection = ((cur_ray.direction -
        2*dot(cur_ray.direction, nearest_hit.norm))
        * nearest_hit.norm)+(random_dir(seed)*fuzz);

      const float3 diffuse = nearest_hit.norm + random_dir(seed);

      next_ray.origin = nearest_hit.pos;
      next_ray.direction = normalize((reflection * (1-nearest_mat.diff)) + (diffuse * nearest_mat.diff));

      colour += (nearest_mat.colour*255)*(1.0f-nearest_mat.spec);// + reflection_colour*nearest_ptr->specular;

    }

    rays[id] = next_ray;
  }

  // average it out
  colour /= raysPerPixel;

  // update pixel
  if (image[id][0] == 0 && image[id][1] == 0 && image[id][2] == 0) {
    image[id] = colour;

  } else {
    image[id] = (colour + image[id]) / 2;
  }

}
