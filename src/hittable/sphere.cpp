#include "hittable/sphere.h"

#include "hit_record.h"
#include "ray.h"

Sphere::Sphere(vec3 cen, float r, uint16_t m)
  : center(cen)
  , radius(r)
  , mat_id(m)
{}

Sphere::~Sphere() = default;

bool
Sphere::hit(const Ray& r, float t_min, float t_max, hit_record& rec) const
{
  vec3 oc = r.origin - center;
  float a = dot(r.direction, r.direction);
  float b = dot(oc, r.direction);
  float c = dot(oc, oc) - radius * radius;
  float discriminant = b * b - a * c;
  if (discriminant > 0) {
    float temp = (-b - sqrt(discriminant)) / a;
    if (temp >= t_max || temp <= t_min) {
      temp = (-b + sqrt(discriminant)) / a;
      if (temp >= t_max || temp <= t_min) {
        return false;
      }
    }
    rec.t = temp;
    rec.p = r.point_at_parameter(rec.t);
    rec.normal = (rec.p - center) / radius;
    rec.uv[0] =
      0.5f + std::atan2(-rec.normal.z(), -rec.normal.x()) * M_1_PIf32 * 0.5f;
    rec.uv[1] = 0.5f - std::asin(-rec.normal.y()) * M_1_PIf32;
    rec.mat_id = mat_id;
    return true;
  }
  return false;
}
