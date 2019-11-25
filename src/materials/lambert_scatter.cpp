#include "materials/lambert_scatter.h"

#include "ray.h"

#include "hit_record.h"

LambertianScatter::LambertianScatter(const vec3& a)
  : albedo(a)
{}

bool
LambertianScatter::scatter(const Scene& scene,
                           const Ray& r_in,
                           const hit_record& rec,
                           vec3& attenuation,
                           Ray (&scattered)[2]) const
{
  vec3 target = rec.p + rec.normal + random_in_unit_sphere();
  scattered[0] = Ray(rec.p, target - rec.p);
  attenuation = albedo;
  return true;
}
