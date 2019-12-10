#pragma once

#include "materials/material.h"

#include "math/vec3.h"

namespace Raytracer::Materials {
struct Metal : public Material
{
  Metal(const vec3& a,
        uint16_t albedo_texture_id = std::numeric_limits<uint16_t>::max(),
        uint16_t normal_texture_id = std::numeric_limits<uint16_t>::max());
  void fill_type_data(const Scene& scene,
                      RayPayload& payload,
                      const vec3& texture_coordinates) const override;

  vec3 albedo;
  uint16_t albedo_texture_id;
  uint16_t normal_texture_id;
};
} // namespace Raytracer::Materials
