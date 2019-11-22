#include "materials/metal.h"

#include "ray.h"

#include "hit_record.h"

Metal::Metal(const vec3& a)
  : albedo(a)
{}

bool
Metal::scatter(const Scene& scene,
               const Ray& r_in,
               const hit_record& rec,
               vec3& attenuation,
               Ray& scattered) const
{
  vec3 reflected = reflect(unit_vector(r_in.direction), rec.normal);
  scattered = Ray(rec.p, reflected);
  attenuation = albedo;
  return (dot(scattered.direction, rec.normal) > 0);
}
