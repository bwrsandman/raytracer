#pragma once

#include "material.h"

struct Dielectric : public Material
{
  explicit Dielectric(float ri, float ni);
  void fill_type_data(const Scene& scene,
                      RayPayload& payload,
                      const float (&texture_coordinates)[2]) const override;

  float ref_idx;
  float ni;
};
