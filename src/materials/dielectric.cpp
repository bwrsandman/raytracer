#include "materials/dielectric.h"

#include "hit_record.h"
#include "ray.h"

Dielectric::Dielectric(float ri)
  : ref_idx(ri)
{}

bool
Dielectric::scatter(const Scene& scene,
                    const Ray& r_in,
                    const hit_record& rec,
                    vec3& attenuation,
                    Ray (&scattered)[2]) const
{
  vec3 outward_normal;
  vec3 reflected = reflect(r_in.direction, rec.normal);
  float ni_over_nt;
  float ni = 1.f, nt = ref_idx;
  attenuation = vec3(0.0, 0.0, 0.0);
  vec3 refracted;

  float reflect_prob, reflect_rate, refract_rate;
  float cosine;

  // outside in
  if (dot(r_in.direction, rec.normal) > 0) {
    outward_normal = -rec.normal;
    ni_over_nt = ref_idx;
    //cosine = ref_idx * dot(r_in.direction, rec.normal) / r_in.direction.length();
  }
  // inside out
  else {
    outward_normal = rec.normal;
    ni_over_nt = 1.0 / ref_idx;
    std::swap(ni, nt);
    //cosine = -dot(r_in.direction, rec.normal) / r_in.direction.length();
  }

  // refracted using Snells law
  if (refract(r_in.direction, outward_normal, ni_over_nt, refracted)) {
    
    // Calculate with Fresnels law
    attenuation = fresnel_rate(r_in.direction, outward_normal, ni, nt);
  }

  scattered[0] = Ray(rec.p, reflected);
  scattered[1] = Ray(rec.p, refracted);

  return true;
}
