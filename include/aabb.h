#pragma once

#include "math/vec3.h"
#include "math/vec3_simd.h"

namespace Raytracer {
using Raytracer::Math::bool_simd_t;
using Raytracer::Math::float_simd_t;
using Raytracer::Math::vec3;
using Raytracer::Math::vec3_simd;

struct Ray;
template<uint8_t D>
struct RaySimd;
struct hit_record;

struct Aabb
{
  vec3 min, max;

  static bool hit(const Aabb& box, const Ray& r, float t_min, float t_max);
  template<uint8_t D>
  static bool_simd_t<D> hit(const Aabb& box,
                            const RaySimd<D>& r,
                            float_simd_t<D> t_min,
                            float_simd_t<D> t_max);
};

static_assert(sizeof(Aabb) == 24,
              "axis-aligned bounding box is not minimal size");

template<uint8_t D>
struct AabbSimd
{
  vec3_simd<D> min, max;

  static bool_simd_t<D> hit(const AabbSimd& box,
                            const Ray& r,
                            float_simd_t<D> t_min,
                            float_simd_t<D> t_max);
};

static_assert(sizeof(AabbSimd<4>) == 0x60,
              "quad axis-aligned bounding box is not minimal size");
static_assert(sizeof(AabbSimd<8>) == 0xC0,
              "oct axis-aligned bounding box is not minimal size");

} // namespace Raytracer
