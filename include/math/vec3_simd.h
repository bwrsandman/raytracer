#pragma once

#include "float_simd.h"
#include "vec3.h"

namespace Raytracer::Math {

template<uint8_t D>
struct vec3_simd
{
  inline vec3_simd()
    : e{ float_simd_t<D>(0.0f), float_simd_t<D>(0.0f), float_simd_t<D>(0.0f) }
  {}
  inline vec3_simd(float_simd_t<D> x, float_simd_t<D> y, float_simd_t<D> z)
    : e{ x, y, z }
  {}
  inline explicit vec3_simd(const vec3 (&scalars)[D]);
  explicit vec3_simd(const vec3 scalars[]);

  float_simd_t<D> e[3];
};

template<>
inline vec3_simd<4>::vec3_simd(const vec3 (&scalars)[4])
  : e{ float_simd_t<4>(
         { scalars[3].x(), scalars[2].x(), scalars[1].x(), scalars[0].x() }),
       float_simd_t<4>(
         { scalars[3].y(), scalars[2].y(), scalars[1].y(), scalars[0].y() }),
       float_simd_t<4>(
         { scalars[3].z(), scalars[2].z(), scalars[1].z(), scalars[0].z() }) }
{}

template<>
inline vec3_simd<4>::vec3_simd(const vec3 scalars[])
  : e{ float_simd_t<4>(
         { scalars[3].x(), scalars[2].x(), scalars[1].x(), scalars[0].x() }),
       float_simd_t<4>(
         { scalars[3].y(), scalars[2].y(), scalars[1].y(), scalars[0].y() }),
       float_simd_t<4>(
         { scalars[3].z(), scalars[2].z(), scalars[1].z(), scalars[0].z() }) }
{}

} // namespace Raytracer::Math
