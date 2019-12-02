#pragma once

#include "vec2.h"
#include "vec3.h"

struct RayPayload;
struct Scene;

struct Material
{
  virtual ~Material() = default;
  virtual void fill_type_data(const Scene& scene,
                              RayPayload& payload,
                              const vec2& texture_coordinates) const = 0;
};
