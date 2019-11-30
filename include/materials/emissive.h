#pragma once

#include "material.h"

#include "vec3.h"

struct Emissive : public Material
{
  explicit Emissive(const vec3& a);
  void fill_type_data(RayPayload& payload) const override;

  vec3 albedo;
};
