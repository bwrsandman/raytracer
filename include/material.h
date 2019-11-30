#pragma once

#include "vec3.h"

struct RayPayload;

class Material
{
public:
  virtual ~Material() = default;
  virtual void fill_type_data(RayPayload& payload) const = 0;
};
