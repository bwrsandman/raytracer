#include "materials/dielectric.h"

#include "hit_record.h"
#include "ray.h"
#include "vec3.h"

Dielectric::Dielectric(float ri)
  : ref_idx(ri)
{}

bool
Dielectric::scatter(const Ray& r_in,
                    const hit_record& rec,
                    vec3& attenuation,
                    Ray& scattered) const
{
  vec3 outward_normal;
  vec3 reflected = reflect(r_in.direction(), rec.normal);
  float ni_over_nt;
  attenuation = vec3(1.0, 1.0, 1.0);
  vec3 refracted;

  float reflect_prob;
  float cosine;

  // outside in
  if (dot(r_in.direction(), rec.normal) > 0) {
    outward_normal = -rec.normal;
    ni_over_nt = ref_idx;
    cosine =
      ref_idx * dot(r_in.direction(), rec.normal) / r_in.direction().length();
  }
  // inside out
  else {
    outward_normal = rec.normal;
    ni_over_nt = 1.0 / ref_idx;
    cosine = -dot(r_in.direction(), rec.normal) / r_in.direction().length();
  }

  // Snell
#if 0
    if (refract(r_in.direction(), outward_normal, ni_over_nt, refracted))
    {
        reflect_prob = schlick(cosine, ref_idx);
    }
    else
#endif
  {
    reflect_prob = 1.0;
  }

  // pathtracer methode (one or the other, not both)
  if (random_double() < reflect_prob) {
    scattered = Ray(rec.p, reflected);
  } else {
    scattered = Ray(rec.p, refracted);
  }

  return true;
}
