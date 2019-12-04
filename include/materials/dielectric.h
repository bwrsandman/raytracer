#pragma once

#include "materials/material.h"

struct Dielectric : public Material
{
  explicit Dielectric(const vec3& a, float ri, float ni);
  void fill_type_data(const Scene& scene,
                      RayPayload& payload,
                      const vec3& texture_coordinates) const override;

  float ref_idx;
  float ni;
  vec3 albedo;
};
