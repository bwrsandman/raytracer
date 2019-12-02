#pragma once

#include "materials/material.h"

#include "vec3.h"

struct Emissive : public Material
{
  explicit Emissive(const vec3& a);
  void fill_type_data(const Scene& scene,
                      RayPayload& payload,
                      const vec2& texture_coordinates) const override;

  vec3 albedo;
};
