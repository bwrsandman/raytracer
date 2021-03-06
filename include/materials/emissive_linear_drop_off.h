#pragma once

#include "materials/material.h"

#include "math/vec3.h"

namespace Raytracer::Materials {
struct EmissiveLinearDropOff : public Material
{
  EmissiveLinearDropOff(const vec3& a, float factor);
  void fill_type_data(const Scene& scene,
                      RayPayload& payload,
                      const vec3& texture_coordinates) const override;

  vec3 albedo;
  float drop_off_factor;
};
} // namespace Raytracer::Materials
