#pragma once

#include "material.h"

#include "vec3.h"

struct Emissive : public Material
{
  explicit Emissive(const vec3& a);
  void fill_type_data(const Scene& scene,
                      RayPayload& payload,
                      const float (&texture_coordinates)[2]) const override;

  vec3 albedo;
};
