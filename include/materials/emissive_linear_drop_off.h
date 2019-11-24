#pragma once

#include "material.h"

#include "vec3.h"

class EmissiveLinearDropOff : public Material
{
public:
  EmissiveLinearDropOff(const vec3& a, float factor);
  bool scatter(const Scene& scene,
               const Ray& r_in,
               const hit_record& rec,
               vec3& attenuation,
               Ray& scattered) const override;

  const vec3 albedo;
  const float drop_off_factor;
};
