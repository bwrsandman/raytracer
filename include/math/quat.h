#pragma once

#include "mat4.h"

namespace Raytracer::Math {
struct quat
{
  float w, x, y, z;

  quat inverse() const
  {
    auto norm = x * x + y * y + z * z + w * w;
    return { w / norm, -x / norm, -y / norm, -z / norm };
  }

  quat normalize() const
  {
    float norm = sqrt(x * x + y * y + z * z + w * w);
    return { w / norm, x / norm, y / norm, z / norm };
  }

  
  /// Conversion between quaternion and pre-multiplied rotation matrix
  explicit operator mat4() const
  {
    return { 1.0f - 2.0f * y * y - 2.0f * z * z,
             2.0f * x * y - 2.0f * z * w,
             2.0f * x * z + 2.0f * y * w,
             0.0f,

             2.0f * x * y + 2.0f * z * w,
             1.0f - 2.0f * x * x - 2.0f * z * z,
             2.0f * y * z - 2.0f * x * w,
             0.0f,

             2.0f * x * z - 2.0f * y * w,
             2.0f * y * z + 2.0f * x * w,
             1.0f - 2.0f * x * x - 2.0f * y * y,
             0.0f,

             0.0f,
             0.0f,
             0.0f,
             1.0f };
  }
};

inline quat operator*(const quat& q1, const quat& q2)
{
  float w = (q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z);
  float x = (q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y);
  float y = (q1.w * q2.y - q1.x * q2.z + q1.y * q2.w + q1.z * q2.x);
  float z = (q1.w * q2.z + q1.x * q2.y - q1.y * q2.x + q1.z * q2.w);

  return  { w, x, y, z };
}
} // namespace Raytracer::Math
