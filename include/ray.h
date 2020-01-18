#pragma once

#include <array>

#include "math/vec3.h"
#if !__EMSCRIPTEN__
#include "math/vec3_simd.h"
#endif

namespace Raytracer {
#if !__EMSCRIPTEN__
using Raytracer::Math::float_simd_t;
using Raytracer::Math::vec3_simd;
#endif
using Raytracer::Math::vec3;

struct Ray
{
  constexpr Ray() noexcept
    : origin()
    , direction()
  {}
  constexpr Ray(const vec3& a, const vec3& b) noexcept
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

#if !__EMSCRIPTEN__
template<uint8_t D>
struct RaySimd
{
  inline vec3_simd<D> point_at_parameter(float_simd_t<D> t) const
  {
    return origin + t * direction;
  }

  template<uint8_t index>
  inline constexpr static Ray get_scalar(const RaySimd& vector)
  {
    static_assert(index < D, "scalar index out of bounds");
    return { vec3_simd<D>::get_scalar<index>(vector.origin),
             vec3_simd<D>::get_scalar<index>(vector.direction) };
  }
  inline constexpr static std::array<Ray, D> get_scalars(const RaySimd& vector)
  {
    auto origins = vec3_simd<D>::get_scalars(vector.origin);
    auto directions = vec3_simd<D>::get_scalars(vector.direction);
    std::array<Ray, D> result;
    for (uint8_t i = 0; i < D; ++i) {
      result[i].origin = origins[i];
      result[i].direction = directions[i];
    }
    return result;
  }

  vec3_simd<D> origin;
  vec3_simd<D> direction;
};

static_assert(sizeof(RaySimd<4>) == 0x60, "quad ray is not minimal size");
static_assert(sizeof(RaySimd<8>) == 0xC0, "oct ray is not minimal size");
#endif

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
  uint32_t bvh_hits;
};
} // namespace Raytracer
