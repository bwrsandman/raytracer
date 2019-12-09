#pragma once

#include "materials/material.h"

#include "math/vec3.h"

namespace Raytracer::Materials {
struct Emissive : public Material
{
  explicit Emissive(const vec3& a);
  void fill_type_data(const Scene& scene,
                      RayPayload& payload,
                      const vec3& texture_coordinates) const override;

  vec3 albedo;
};
} // namespace Raytracer::Materials
