#pragma once

#include "material.h"

#include "vec3.h"

struct Lambert : public Material
{
  explicit Lambert(
    const vec3& a,
    uint16_t albedo_texture_id = std::numeric_limits<uint16_t>::max(),
    uint16_t normal_texture_id = std::numeric_limits<uint16_t>::max());
  void fill_type_data(const Scene& scene,
                      RayPayload& payload,
                      const float (&texture_coordinates)[2]) const override;

  vec3 albedo;
  uint16_t albedo_texture_id;
  uint16_t normal_texture_id;
};
