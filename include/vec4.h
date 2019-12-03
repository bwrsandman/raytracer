#pragma once

struct vec4
{
  constexpr vec4() noexcept
    : e{ 0.0f, 0.0f, 0.0f, 0.0f }
  {}
  constexpr vec4(float e0, float e1, float e2, float e3) noexcept
    : e{ e0, e1, e2, e3 }
  {}

  float e[4];
};
