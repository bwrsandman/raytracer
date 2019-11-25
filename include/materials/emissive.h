#pragma once

#include "material.h"

#include "vec3.h"

class Emissive : public Material
{
public:
  explicit Emissive(const vec3& a);
  bool scatter(const Scene& scene,
               const Ray& r_in,
               const hit_record& rec,
               vec3& attenuation,
               Ray (&scattered)[2]) const override;

  const vec3 albedo;
};
