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

template<>
inline vec3_simd<8>::vec3_simd(const vec3 (&scalars)[8])
  : e{ float_simd_t<8>({ scalars[7].x(),
                         scalars[6].x(),
                         scalars[5].x(),
                         scalars[4].x(),
                         scalars[3].x(),
                         scalars[2].x(),
                         scalars[1].x(),
                         scalars[0].x() }),
       float_simd_t<8>({ scalars[7].y(),
                         scalars[6].y(),
                         scalars[5].y(),
                         scalars[4].y(),
                         scalars[3].y(),
                         scalars[2].y(),
                         scalars[1].y(),
                         scalars[0].y() }),
       float_simd_t<8>({ scalars[7].z(),
                         scalars[6].z(),
                         scalars[5].z(),
                         scalars[4].z(),
                         scalars[3].z(),
                         scalars[2].z(),
                         scalars[1].z(),
                         scalars[0].z() }) }
{}

template<>
inline vec3_simd<8>::vec3_simd(const vec3 scalars[])
  : e{ float_simd_t<8>({ scalars[7].x(),
                         scalars[6].x(),
                         scalars[5].x(),
                         scalars[4].x(),
                         scalars[3].x(),
                         scalars[2].x(),
                         scalars[1].x(),
                         scalars[0].x() }),
       float_simd_t<8>({ scalars[7].y(),
                         scalars[6].y(),
                         scalars[5].y(),
                         scalars[4].y(),
                         scalars[3].y(),
                         scalars[2].y(),
                         scalars[1].y(),
                         scalars[0].y() }),
       float_simd_t<8>({ scalars[7].z(),
                         scalars[6].z(),
                         scalars[5].z(),
                         scalars[4].z(),
                         scalars[3].z(),
                         scalars[2].z(),
                         scalars[1].z(),
                         scalars[0].z() }) }
{}

} // namespace Raytracer::Math
