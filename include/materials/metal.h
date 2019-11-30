#pragma once

#include "material.h"

#include "vec3.h"

struct Ray;
struct hit_record;

struct Metal : public Material
{
  explicit Metal(const vec3& a);
  void fill_type_data(const Scene& scene,
                      RayPayload& payload,
                      float (&texture_coordinates)[2]) const override;

  vec3 albedo;
};
