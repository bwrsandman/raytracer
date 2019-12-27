#ifndef RAYTRACER_SPHERE_T_H
#define RAYTRACER_SPHERE_T_H

#include "ray_t.h"

struct sphere_t
{
  vec4 center;
  float radius;
  uint mat_id;
};

#ifdef __cplusplus
static void
sphere_serialize(const sphere_t& sphere, vec4& data0, uvec4& data1)
{
  data0.e[0] = sphere.center.e[0];
  data0.e[1] = sphere.center.e[1];
  data0.e[2] = sphere.center.e[2];
  data0.e[3] = sphere.radius;
  data1.e[0] = sphere.mat_id;
}

#else
#include "hit_record_t.h"

void
sphere_deserialize(vec4 data0, uvec4 data1, out sphere_t sphere)
{
  // TODO mat_id
  sphere.center.xyz = data0.xyz;
  sphere.center.w = 1.0f;
  sphere.radius = data0.w;
  sphere.mat_id = data1.x;
}

void
sphere_hit(ray_t ray,
           sphere_t sphere,
           float t_min,
           float t_max,
           out hit_record_t rec)
{
  vec3 oc = ray.origin.xyz - sphere.center.xyz;
  float a = dot(ray.direction.xyz, ray.direction.xyz);
  float b = dot(oc, ray.direction.xyz);
  float c = dot(oc, oc) - sphere.radius * sphere.radius;
  float discriminant = b * b - a * c;
  if (discriminant > 0) {
    float temp = (-b - sqrt(discriminant)) / a;
    if (temp >= t_max || temp <= t_min) {
      temp = (-b + sqrt(discriminant)) / a;
      if (temp >= t_max || temp <= t_min) {
        rec.status = HIT_RECORD_STATUS_MISS;
        return;
      }
    }
    rec.t = temp;
    rec.p = ray_point_at(ray, rec.t).xyz;
    rec.normal = (rec.p - sphere.center.xyz) / sphere.radius;
    rec.tangent = normalize(cross(rec.normal, vec3(0, 1, 0)));
    rec.uv.x = 0.5f + atan(-rec.normal.z, rec.normal.x) * M_1_2PI;
    rec.uv.y = 0.5f - asin(-rec.normal.y) * M_1_PI;
    rec.mat_id = sphere.mat_id;
    rec.status = HIT_RECORD_STATUS_HIT;
    return;
  }
  rec.status = HIT_RECORD_STATUS_MISS;
}
#endif // __cplusplus

#endif // RAYTRACER_SPHERE_T_H
