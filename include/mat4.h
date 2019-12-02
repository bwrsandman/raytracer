#pragma once

struct mat4
{
  constexpr mat4(float e00,
                 float e10,
                 float e20,
                 float e30,
                 float e01,
                 float e11,
                 float e21,
                 float e31,
                 float e02,
                 float e12,
                 float e22,
                 float e32,
                 float e03,
                 float e13,
                 float e23,
                 float e33) noexcept
    : m{ e00, e10, e20, e30, e01, e11, e21, e31,
         e02, e12, e22, e32, e03, e13, e23, e33 }
  {}

  float m[16];
};

inline mat4
dot(const mat4& lhs, const mat4& rhs)
{
  return {
    lhs.m[0] * rhs.m[0] + lhs.m[4] * rhs.m[0] + lhs.m[8] * rhs.m[0] +
      lhs.m[12] * rhs.m[0], // 0,0
    lhs.m[0] * rhs.m[1] + lhs.m[4] * rhs.m[1] + lhs.m[8] * rhs.m[1] +
      lhs.m[12] * rhs.m[1], // 1,0
    lhs.m[0] * rhs.m[2] + lhs.m[4] * rhs.m[2] + lhs.m[8] * rhs.m[2] +
      lhs.m[12] * rhs.m[2], // 2,0
    lhs.m[0] * rhs.m[3] + lhs.m[4] * rhs.m[3] + lhs.m[8] * rhs.m[3] +
      lhs.m[12] * rhs.m[3], // 2,0

    lhs.m[1] * rhs.m[4] + lhs.m[5] * rhs.m[4] + lhs.m[9] * rhs.m[4] +
      lhs.m[13] * rhs.m[4], // 0,1
    lhs.m[1] * rhs.m[5] + lhs.m[5] * rhs.m[5] + lhs.m[9] * rhs.m[5] +
      lhs.m[13] * rhs.m[5], // 1,1
    lhs.m[1] * rhs.m[6] + lhs.m[5] * rhs.m[6] + lhs.m[9] * rhs.m[6] +
      lhs.m[13] * rhs.m[6], // 2,1
    lhs.m[1] * rhs.m[7] + lhs.m[5] * rhs.m[7] + lhs.m[9] * rhs.m[7] +
      lhs.m[13] * rhs.m[7], // 2,1

    lhs.m[2] * rhs.m[8] + lhs.m[6] * rhs.m[8] + lhs.m[10] * rhs.m[8] +
      lhs.m[14] * rhs.m[8], // 0,2
    lhs.m[2] * rhs.m[9] + lhs.m[6] * rhs.m[9] + lhs.m[10] * rhs.m[9] +
      lhs.m[14] * rhs.m[9], // 1,2
    lhs.m[2] * rhs.m[10] + lhs.m[6] * rhs.m[10] + lhs.m[10] * rhs.m[10] +
      lhs.m[14] * rhs.m[10], // 2,2
    lhs.m[2] * rhs.m[11] + lhs.m[6] * rhs.m[11] + lhs.m[10] * rhs.m[11] +
      lhs.m[14] * rhs.m[11], // 2,2

    lhs.m[3] * rhs.m[12] + lhs.m[7] * rhs.m[12] + lhs.m[11] * rhs.m[12] +
      lhs.m[15] * rhs.m[12], // 0,3
    lhs.m[3] * rhs.m[13] + lhs.m[7] * rhs.m[13] + lhs.m[11] * rhs.m[13] +
      lhs.m[15] * rhs.m[13], // 1,3
    lhs.m[3] * rhs.m[14] + lhs.m[7] * rhs.m[14] + lhs.m[11] * rhs.m[14] +
      lhs.m[15] * rhs.m[14], // 2,3
    lhs.m[3] * rhs.m[15] + lhs.m[7] * rhs.m[15] + lhs.m[11] * rhs.m[15] +
      lhs.m[15] * rhs.m[15], // 2,3
  };
}
