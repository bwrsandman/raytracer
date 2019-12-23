#ifndef RAYTRACER_SPHERE_T_H
#define RAYTRACER_SPHERE_T_H

#include "ray_t.h"

struct sphere_t
{
  vec4 center;
  float radius;
  int mat_id;
};

void
sphere_hit(ray_t ray,
           sphere_t sphere,
           float t_min,
           float t_max,
           out hit_record_t rec)
{
  vec4 oc = ray.origin - sphere.center;
  float a = dot(ray.direction, ray.direction);
  float b = dot(oc, ray.direction);
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

#endif // RAYTRACER_SPHERE_T_H
