#ifndef RAYTRACER_PLANE_T_H
#define RAYTRACER_PLANE_T_H

#include "ray_t.h"

struct plane_t
{
  vec3 min;
  vec3 max;
  vec3 normal;
  uint32_t mat_id;
};

#ifdef __cplusplus
static void
plane_serialize(const plane_t& plane,
                vec4& data0,
                vec4& data1,
                vec4& data2,
                uvec4& data3)
{
  // TODO mat_id
  data0.e[0] = plane.min.e[0];
  data0.e[1] = plane.min.e[1];
  data0.e[2] = plane.min.e[2];
  data1.e[0] = plane.max.e[0];
  data1.e[1] = plane.max.e[1];
  data1.e[2] = plane.max.e[2];
  data2.e[0] = plane.normal.e[0];
  data2.e[1] = plane.normal.e[1];
  data2.e[2] = plane.normal.e[2];
  data3.e[0] = plane.mat_id;
}

#else
#include "hit_record_t.h"

void
plane_deserialize(vec4 data0,
                  vec4 data1,
                  vec4 data2,
                  uvec4 data3,
                  out plane_t plane)
{
  plane.min = data0.xyz;
  plane.max = data1.xyz;
  plane.normal = data2.xyz;
  plane.mat_id = data3.x;
}

void
plane_hit(ray_t ray,
          plane_t plane,
          float t_min,
          float t_max,
          out hit_record_t rec)
{
  int index_one = 0, index_two = 0, axis = 0;
  float t;

  if (plane.normal.x != 0.f) { // yz plane

    index_one = 1;
    index_two = 2;
    axis = 0;

  } else if (plane.normal.y != 0.f) { // xz plane

    index_one = 0;
    index_two = 2;
    axis = 1;

  } else { // xy plane

    index_one = 0;
    index_two = 1;
    axis = 2;
  }
  t = (plane.min[axis] - ray.origin[axis]) / ray.direction[axis];

  vec4 point = ray_point_at(ray, t);
  vec2 point2 = vec2(point[index_one], point[index_two]);
  vec2 min2 = vec2(plane.min[index_one], plane.min[index_two]);
  vec2 max2 = vec2(plane.max[index_one], plane.max[index_two]);

  if (t < t_min || t > t_max || point2.x < min2.x || point2.x > max2.x ||
      point2.y < min2.y || point2.y > max2.y) {
    rec.status = HIT_RECORD_STATUS_MISS;
    return;
  }

  vec2 uv = (point2 - min2) / (max2 - min2);

  rec.tangent[axis] = 0.0f;
  rec.tangent[index_one] = uv.x;
  rec.tangent[index_two] = uv.y;
  rec.tangent = normalize(rec.tangent);

  rec.t = t;
  rec.uv = uv;
  rec.mat_id = plane.mat_id;
  rec.position = ray_point_at(ray, t).xyz;
  rec.normal = plane.normal;
  rec.status = HIT_RECORD_STATUS_HIT;
}
#endif // __cplusplus

#endif // RAYTRACER_PLANE_T_H
