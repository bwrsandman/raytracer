#pragma once

#include "material.h"

struct Dielectric : public Material
{
  explicit Dielectric(float ri, float ni);
  void fill_type_data(RayPayload& payload) const;

  float ref_idx;
  float ni;
};
