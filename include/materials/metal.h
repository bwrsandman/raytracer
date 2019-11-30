#pragma once

#include "material.h"

#include "vec3.h"

struct Ray;
struct hit_record;

struct Metal : public Material
{
  explicit Metal(const vec3& a);
  void fill_type_data(RayPayload& payload) const override;

  vec3 albedo;
};
