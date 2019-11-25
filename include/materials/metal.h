#pragma once

#include "material.h"

#include "vec3.h"

struct Ray;
struct hit_record;

class Metal : public Material
{
public:
  explicit Metal(const vec3& a);
  bool scatter(const Scene& scene,
               const Ray& r_in,
               const hit_record& rec,
               vec3& attenuation,
               Ray (&scattered)[2]) const override;
  vec3 albedo;
};
