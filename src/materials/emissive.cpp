#include "materials/emissive.h"

#include "hit_record.h"

Emissive::Emissive(const vec3& a)
  : albedo(a)
{}

bool
Emissive::scatter(const Scene& scene,
                  const Ray& r_in,
                  const hit_record& rec,
                  vec3& attenuation,
                  Ray& scattered) const
{
  attenuation = albedo;
  return false;
}
