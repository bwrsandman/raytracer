#pragma once 

class vec3;
class Ray;
struct hit_record;

class Material
{
public:
  virtual bool scatter(const Ray& r_in,
                       const hit_record& rec,
                       vec3& attenuation,
                       Ray& scattered) const = 0;
};

class Lambertian : public Material
{
public:
  Lambertian(const vec3& a)
    : albedo(a)
  {}
  virtual bool scatter(const Ray& r_in,
                       const hit_record& rec,
                       vec3& attenuation,
                       Ray& scattered) const
  {
    vec3 target = rec.p + rec.normal + random_in_unit_sphere();
    scattered = Ray(rec.p, target - rec.p);
    attenuation = albedo;
    return true;
  }

  vec3 albedo;
};

class Metal : public Material
{
public:
  Metal(const vec3& a)
    : albedo(a)
  {}
  virtual bool scatter(const Ray& r_in,
                       const hit_record& rec,
                       vec3& attenuation,
                       Ray& scattered) const
  {
    vec3 reflected = reflect(unit_vector(r_in.direction()), rec.normal);
    scattered = Ray(rec.p, reflected);
    attenuation = albedo;
    return (dot(scattered.direction(), rec.normal) > 0);
  }
  vec3 albedo;
};


//class Dielectric : public Material
//{
//public:
//  Dielectric(float ri)
//    : ref_idx(ri)
//  {}
//  virtual bool scatter(const Ray& r_in,
//                       const hit_record& rec,
//                       vec3& attenuation,
//                       Ray& scattered) const
//  {
//    vec3 outward_normal;
//    vec3 reflected = reflect(r_in.direction(), rec.normal);
//    float ni_over_nt;
//    attenuation = vec3(1.0, 1.0, 1.0);
//    vec3 refracted;
//
//	float reflect_prob;
//    float cosine;
//
//	// outside in
//    if (dot(r_in.direction(), rec.normal) > 0) {
//      outward_normal = -rec.normal;
//      ni_over_nt = ref_idx;
//      cosine =
//        ref_idx * dot(r_in.direction(), rec.normal) / r_in.direction().length();
//    } else { // inside out
//      outward_normal = rec.normal;
//      ni_over_nt = 1.0 / ref_idx;
//      cosine = -dot(r_in.direction(), rec.normal) / r_in.direction().length();
//    }
//
//	// Snell
//    if (refract(r_in.direction(), outward_normal, ni_over_nt, refracted)) {
//      reflect_prob = schlick(cosine, ref_idx);
//    } else {
//      reflect_prob = 1.0;
//    }
//
//	// pathtracer methode (one or the other, not both)
//    if (random_double() < reflect_prob) {
//      scattered = ray(rec.p, reflected);
//    } else {
//      scattered = ray(rec.p, refracted);
//    }
//
//    return true;
//  }
//
//  float ref_idx;
//};