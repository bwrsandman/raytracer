#include "materials/emissive_linear_drop_off.h"

#include "hit_record.h"

EmissiveLinearDropOff::EmissiveLinearDropOff(const vec3& a, float factor)
  : albedo(a)
  , drop_off_factor(factor)
{}

bool
EmissiveLinearDropOff::scatter(const Scene& scene,
                               const Ray& r_in,
                               const hit_record& rec,
                               vec3& attenuation,
                               Ray (&scattered)[2]) const
{
  attenuation = albedo / (rec.t * drop_off_factor);
  return false;
}
