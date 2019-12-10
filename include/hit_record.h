#pragma once

#include <cstdint>

#include "math/vec2.h"
#include "math/vec3.h"

namespace Raytracer {
using Raytracer::Math::vec3;
struct hit_record
{
  float t;
  vec3 p;
  vec3 normal;
  vec3 tangent;
  vec3 uv;
  uint16_t mat_id;
};
} // namespace Raytracer
