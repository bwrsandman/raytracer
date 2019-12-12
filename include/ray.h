#pragma once

#include "math/vec3.h"
#include "math/vec3_simd.h"

namespace Raytracer {
using Raytracer::Math::float_simd_t;
using Raytracer::Math::vec3;
using Raytracer::Math::vec3_simd;

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

static_assert(sizeof(Ray) == 24, "ray is not minimal size");

template<uint8_t D>
struct RaySimd
{
  inline vec3_simd<D> point_at_parameter(float_simd_t<D> t) const
  {
    return origin + t * direction;
  }

  vec3_simd<D> origin;
  vec3_simd<D> direction;
};

static_assert(sizeof(RaySimd<4>) == 0x60, "quad ray is not minimal size");
static_assert(sizeof(RaySimd<8>) == 0xC0, "oct ray is not minimal size");

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
