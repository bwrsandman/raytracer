#pragma once

#include "material.h"

#include "vec3.h"

struct EmissiveLinearDropOff : public Material
{
  EmissiveLinearDropOff(const vec3& a, float factor);
  void fill_type_data(const Scene& scene,
                      RayPayload& payload,
                      const float (&texture_coordinates)[2]) const override;

  vec3 albedo;
  float drop_off_factor;
};
