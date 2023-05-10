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

bool isEqual(float3 a, float3 b) {
  return a.x == b.x && a.y == b.y && a.z == b.z;
}

// -- Main Path Tracing --

const float3 dead_ray = (float3)(-1.0,-1.0,-1.0);

__kernel void pathTrace(
  __global Obj* scene,
  __global Material* mats,
  int sceneLen,
  __global Ray* rays,
  int raysPerPixel,
  __global float3* jitter,
  __global int* shadow,
  __global float3* image,
  int iter,
  int max_depth
) {
  int id = get_global_id(0);
  float3 colour = (float3)(0.0,0.0,0.0);
  bool firstIter = iter == 0;
  int raysCount = 0;

  for (int ray_i=0; ray_i<raysPerPixel; ray_i++) {
    const int ray_id = id * raysPerPixel + ray_i;

    Ray cur_ray = rays[ray_id];

    if (isEqual(cur_ray.origin, dead_ray)) {
      continue;
    }

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

      if (shadow[ray_i]) { // lighting ray

        // light ray
        float3 light_colour = nearest_mat.colour;
        float3 light_start = nearest_hit.pos + nearest_hit.norm*0.01f;
        float3 light_end = jitter[ray_id]; // [-1, 1]
        light_end.x *= 7.5f;
        light_end.y = fabs(light_end.y) * 15.0f;
        light_end.z = 40.0f;

        Ray light_ray;
        light_ray.origin = light_start;
        light_ray.direction = normalize(light_end - light_start);

        bool hitlight = true;
        for (int i=0; i<sceneLen; i++) {
          Obj cur_obj = scene[i];

          rayHit cur_hit = intersect(cur_obj, light_ray);

          if (cur_hit.intersect == 1) {
            hitlight = false;

            break;
          }
        }

        if (hitlight) {
          light_colour += (float3)(0.9f, 0.9f, 0.9f);
        }

        light_colour /= 2;
        colour += light_colour;
        raysCount++;

        // light runs once
        next_ray.origin = dead_ray;

      } else {

        // relfection rays
        const float fuzz = 0.8;
        const float3 reflection = ((cur_ray.direction -
          2*dot(cur_ray.direction, nearest_hit.norm))
          * nearest_hit.norm)+(normalize(jitter[ray_id])*fuzz);

        const float3 diffuse = nearest_hit.norm + normalize(jitter[ray_id]);

        next_ray.origin = nearest_hit.pos;
        next_ray.direction = normalize((reflection * (1-nearest_mat.diff))
          + (diffuse * nearest_mat.diff));

        // if its the first one, we're looking directly at the object
        float3 reflection_colour;
        if (firstIter) {
          reflection_colour = nearest_mat.colour*(1.0f-nearest_mat.spec);

        } else {
          reflection_colour = (image[id]/255)*(1.0f-nearest_mat.spec) + (nearest_mat.colour) * (nearest_mat.spec);
        }

        colour += reflection_colour;
        raysCount++;
      }
    } else { // hits nothing
      colour += (float3)(0.1,0.1,0.2);
      raysCount++;

      if (!firstIter) {
        colour += (float3)(0.8,0.8,0.8);
        raysCount++;
      }

      next_ray.origin = dead_ray;
    }

    rays[ray_id] = next_ray;
  }

  colour /= raysCount;

  // update pixel
  // float c = 0.89;
  // float weight = exp(-(log(iter+c)/log((float)max_depth)));

  // account for only dead rays
  if (isEqual(colour, (float3)(0.0,0.0,0.0))) {
    return;
  }

  float weight = max_depth-iter / max_depth;

  image[id] = ((colour*255) + image[id]) / 2;


  // image[id] = ((colour*255)*weight + image[id]*exp(-1/log((float)max_depth))) / (1 + weight*exp(-1/log((float)max_depth)));
}
