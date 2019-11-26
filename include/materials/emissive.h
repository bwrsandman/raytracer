#pragma once

#include "material.h"

#include "vec3.h"

struct Emissive : public Material
{
  explicit Emissive(const vec3& a);
  bool scatter(const Scene& scene,
               const Ray& r_in,
               const hit_record& rec,
               vec3& attenuation,
               Ray& scattered) const override;

  vec3 albedo;
};
