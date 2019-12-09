#pragma once

#include "math/vec3.h"

namespace Raytracer {
using Raytracer::Math::vec3;

struct Ray
{
  Ray()
    : origin()
    , direction()
  {}
  Ray(const vec3& a, const vec3& b)
    : origin(a)
    , direction(b)
  {}
  inline vec3 point_at_parameter(float t) const
  {
    return origin + t * direction;
  }

  vec3 origin;
  vec3 direction;
};

struct RayPayload
{
  RayPayload()
    : type(Type::NoHit)
  {}
  enum class Type
  {
    NoHit,
    Dielectric,
    Emissive,
    Lambert,
    Metal,
  };

  union
  {
    vec3 attenuation;
    vec3 emission;
  };

  struct
  {
    float ni;
    float nt;
  } dielectric;
  float distance;
  vec3 normal;
  vec3 tangent;
  Type type;
};
} // namespace Raytracer
