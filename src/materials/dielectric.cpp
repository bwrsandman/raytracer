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
  vec3 outward_normal = rec.normal;
  vec3 reflected = reflect(r_in.direction, rec.normal);
  float ni_over_nt;
  float ni = 1.f, nt = ref_idx; // NEW
  attenuation = vec3(1.0, 1.0, 1.0);
  vec3 refracted;

  float reflect_prob;
  float cosine;

  

  // refracted using Snells and Fresnels law
  if (refract(r_in.direction, outward_normal, 1.0f, ref_idx, refracted)) {
    attenuation = fresnel_rate(r_in.direction, outward_normal, 1.0f, ref_idx);
  }

  if (std::isnan(refracted.x())) {
    refracted = vec3(1.f, 1.f, 1.f);
  }

   scattered[0] = Ray(rec.p, reflected);
   scattered[1] = Ray(rec.p, refracted);

  // attenuation = vec3(0.5f, 0.5f, 0.5f);


  //if (dot(r_in.direction, rec.normal) > 0) {
  //  outward_normal = -rec.normal;
  //  ni_over_nt = ref_idx;
  //  cosine =
  //    ref_idx * dot(r_in.direction, rec.normal) / r_in.direction.length();
  //} else {
  //  outward_normal = rec.normal;
  //  ni_over_nt = 1.0 / ref_idx;
  //  cosine = -dot(r_in.direction, rec.normal) / r_in.direction.length();
  //}

  //// refracted using Snells law and Schlicks approximation
  //if (refract_old(r_in.direction, outward_normal, ni_over_nt, refracted)) {

  //  // attenuation =
  //  reflect_prob = schlick(cosine, ref_idx);
  //} else {
  //  reflect_prob = 1.0f;
  //}
  //attenuation = vec3(reflect_prob, reflect_prob, reflect_prob);

  //if (random_double() < reflect_prob) {
  //  scattered[0] = Ray(rec.p, reflected);
  //} else {
  //  scattered[0] = Ray(rec.p, refracted);
  //}


  

  return true;
}
