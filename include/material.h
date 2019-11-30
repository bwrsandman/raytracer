#pragma once

#include "vec3.h"

struct RayPayload;
struct Scene;

struct Material
{
  virtual ~Material() = default;
  virtual void fill_type_data(const Scene& scene,
                              RayPayload& payload,
                              const float (&texture_coordinates)[2]) const = 0;
};
