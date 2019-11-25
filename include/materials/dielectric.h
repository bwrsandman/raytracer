#pragma once

#include "material.h"

class Dielectric : public Material
{
public:
  explicit Dielectric(float ri);
  bool scatter(const Scene& scene,
               const Ray& r_in,
               const hit_record& rec,
               vec3& attenuation,
               Ray(& scattered)[2]) const override;

  float ref_idx;
};
