#pragma once

#include "vec3.h"

struct mat3
{
  constexpr mat3(float e00,
                 float e10,
                 float e20,
                 float e01,
                 float e11,
                 float e21,
                 float e02,
                 float e12,
                 float e22) noexcept
    : m{ e00, e10, e20, e01, e11, e21, e02, e12, e22 }
  {}

  float m[9];
};

inline vec3
dot(const mat3& m, const vec3& v)
{
  return vec3(m.m[0] * v.e[0] + m.m[3] * v.e[1] + m.m[6] * v.e[2],
              m.m[1] * v.e[0] + m.m[4] * v.e[1] + m.m[7] * v.e[2],
              m.m[2] * v.e[0] + m.m[5] * v.e[1] + m.m[8] * v.e[2]);
}
