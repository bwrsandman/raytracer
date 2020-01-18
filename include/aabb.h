#pragma once

#include "math/vec3.h"
#if !__EMSCRIPTEN__
#include "math/vec3_simd.h"
#endif

namespace Raytracer {
#if !__EMSCRIPTEN__
using Raytracer::Math::bool_simd_t;
using Raytracer::Math::float_simd_t;
using Raytracer::Math::vec3_simd;
#endif
using Raytracer::Math::vec3;

struct Ray;
struct hit_record;
#if !__EMSCRIPTEN__
template<uint8_t D>
struct RaySimd;
#endif

struct Aabb
{
  vec3 min, max;

  static bool hit(const Aabb& box, const Ray& r, float t_min, float t_max);
#if !__EMSCRIPTEN__
  template<uint8_t D>
  static bool_simd_t<D> hit(const Aabb& box,
                            const RaySimd<D>& r,
                            float_simd_t<D> t_min,
                            float_simd_t<D> t_max);
#endif
};

static_assert(sizeof(Aabb) == 24,
              "axis-aligned bounding box is not minimal size");

#if !__EMSCRIPTEN__
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
#endif

} // namespace Raytracer
