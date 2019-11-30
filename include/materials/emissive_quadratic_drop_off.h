#pragma once

#include "material.h"

#include "vec3.h"

struct EmissiveQuadraticDropOff : public Material
{
  EmissiveQuadraticDropOff(const vec3& a, float factor);
  void fill_type_data(RayPayload& payload) const override;

  vec3 albedo;
  float drop_off_factor;
};
