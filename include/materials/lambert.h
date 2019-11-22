#pragma once

#include "material.h"

#include "vec3.h"

class Lambertian : public Material
{
public:
  explicit Lambertian(const vec3& a);
  bool scatter(const Ray& r_in,
               const hit_record& rec,
               vec3& attenuation,
               Ray& scattered) const override;

  vec3 albedo;
};
