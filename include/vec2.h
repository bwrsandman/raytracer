#pragma once

struct vec2
{
  constexpr vec2() noexcept
    : e{ 0.0f, 0.0f }
  {}
  constexpr vec2(float e0, float e1) noexcept
    : e{ e0, e1 }
  {}
  inline vec2 operator+(const vec2& rhs) const
  {
    return vec2(e[0] + rhs.e[0], e[1] + rhs.e[1]);
  }
  inline vec2 operator-(const vec2& rhs) const
  {
    return vec2(e[0] - rhs.e[0], e[1] - rhs.e[1]);
  }
  inline vec2 operator*(float rhs) const
  {
    return vec2(e[0] * rhs, e[1] * rhs);
  }

  float e[2];
};
