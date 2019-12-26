#pragma once

namespace Raytracer::Math {
struct vec4
{
  constexpr vec4() noexcept
    : e{ 0.0f, 0.0f, 0.0f, 0.0f }
  {}
  constexpr vec4(float e0, float e1, float e2, float e3) noexcept
    : e{ e0, e1, e2, e3 }
  {}

  inline vec4 operator*(float f) const
  {
    return vec4(f * e[0], f * e[1], f * e[2], f * e[3]);
  }

  inline vec4 operator+(const vec4& other) const
  {
    return vec4(e[0] + other.e[0],
                e[1] + other.e[1],
                e[2] + other.e[2],
                e[3] + other.e[3]);
  }

  float e[4];
};

} // namespace Raytracer::Math
