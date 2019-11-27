#pragma once

#include "material.h"

#include "vec3.h"

struct EmissiveLinearDropOff : public Material
{
  EmissiveLinearDropOff(const vec3& a, float factor);
  bool scatter(const Scene& scene,
               const Ray& r_in,
               const hit_record& rec,
               vec3& attenuation,
               Ray (&scattered)[2]) const override;

  vec3 albedo;
  float drop_off_factor;
};
