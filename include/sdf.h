#pragma once

#include "math/vec3.h"

/// Signed distance function for primitives using the following as a reference
/// http://iquilezles.org/www/articles/distfunctions/distfunctions.htm
namespace Raytracer::sdf {
using Raytracer::Math::vec3;

inline float
intersect(float a, float b)
{
  return std::max(a, b);
}
/// aka union but union is a keyword in c++
inline float
combine(float a, float b)
{
  return std::min(a, b);
}
inline float
difference(float a, float b)
{
  return std::max(a, -b);
}

inline float
sphere(const vec3& position, float radius)
{
  return position.length() - radius;
}

inline float
box(const vec3& position, const vec3& side_length)
{
  vec3 d = std::abs(position) - side_length;
  auto max_side = std::clamp(d.x(), 0.0f, std::max(d.y(), d.z()));
  return (std::max(d, 0.0f) + vec3(max_side, max_side, max_side)).length();
}
inline float
cylinder(const vec3& position, float radius, float length)
{
  vec3 d =
    std::abs(vec3(
      vec3(position.x(), position.z(), 0.0f).length(), position.y(), 0.0f)) -
    vec3(radius, length, 0.0f);
  return std::min(std::max(d.x(), d.y()), 0.0f) + std::max(d, 0.0f).length();
}
} // namespace Raytracer::sdf
