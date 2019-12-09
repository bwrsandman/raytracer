#pragma once

#include "mat4.h"

namespace Raytracer::Math {
struct quat
{
  float x, y, z, w;

  quat inverse() const
  {
    auto norm = x * x + y * y + z * z + w * w;
    return { -x / norm, -y / norm, -z / norm, w / norm };
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
} // namespace Raytracer::Math
