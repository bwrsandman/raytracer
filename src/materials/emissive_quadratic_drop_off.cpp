#include "materials/emissive_quadratic_drop_off.h"

#include "hit_record.h"

EmissiveQuadraticDropOff::EmissiveQuadraticDropOff(const vec3& a, float factor)
  : albedo(a)
  , drop_off_factor(factor)
{}

bool
EmissiveQuadraticDropOff::scatter(const Scene& scene,
                                  const Ray& r_in,
                                  const hit_record& rec,
                                  vec3& attenuation,
                                  Ray& scattered) const
{
  attenuation = albedo / (rec.t * rec.t * drop_off_factor);
  return false;
}
