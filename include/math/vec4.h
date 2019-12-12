#pragma once

#include "vec3.h"

namespace Raytracer::Math {
struct vec4
{
  constexpr vec4() noexcept
    : e{ 0.0f, 0.0f, 0.0f, 0.0f }
  {}
  constexpr vec4(float e0, float e1, float e2, float e3) noexcept
    : e{ e0, e1, e2, e3 }
  {}

  inline float x() const { return e[0]; }
  inline float y() const { return e[1]; }
  inline float z() const { return e[2]; }
  inline float w() const { return e[3]; }

  float e[4];

  inline void vec4::normalize_special()
  {
    float k = 1.0 / sqrt(e[0] * e[0] + e[1] * e[1] + e[2] * e[2] );
    e[0] *= k;
    e[1] *= k;
    e[2] *= k;
  }

  explicit operator vec3() const { return vec3(e[1], e[2], e[3]); }
};
} // namespace Raytracer::Math
