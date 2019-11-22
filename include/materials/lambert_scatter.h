#pragma once

#include "material.h"

#include "vec3.h"

class LambertianScatter : public Material
{
public:
  explicit LambertianScatter(const vec3& a);
  bool scatter(const Scene& scene,
               const Ray& r_in,
               const hit_record& rec,
               vec3& attenuation,
               Ray& scattered) const override;

private:
  vec3 albedo;
};
