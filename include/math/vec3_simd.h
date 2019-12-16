#pragma once

#include <array>

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

  inline void make_unit_vector()
  {
    auto mag2 = e[0] * e[0];
    mag2 = e[1].multiply_add(e[1], mag2);
    mag2 = e[2].multiply_add(e[2], mag2);
    auto k = mag2.reciprocal_sqrt();
    e[0] = e[0] * k;
    e[1] = e[1] * k;
    e[2] = e[2] * k;
  }

  template<uint8_t index>
  inline constexpr static vec3 get_scalar(const vec3_simd& vector)
  {
    static_assert(index < D, "scalar index out of bounds");
    return vec3{
      float_simd_t<D>::get_scalar<index>(vector.e[0]),
      float_simd_t<D>::get_scalar<index>(vector.e[1]),
      float_simd_t<D>::get_scalar<index>(vector.e[2]),
    };
  }
  inline constexpr static std::array<vec3, D> get_scalars(
    const vec3_simd& vector)
  {
    auto e0 = float_simd_t<D>::get_scalars(vector.e[0]);
    auto e1 = float_simd_t<D>::get_scalars(vector.e[1]);
    auto e2 = float_simd_t<D>::get_scalars(vector.e[2]);
    std::array<vec3, D> result;
    for (uint8_t i = 0; i < D; ++i) {
      result[i].e[0] = e0[i];
      result[i].e[1] = e1[i];
      result[i].e[2] = e2[i];
    }
    return result;
  }

  float_simd_t<D> e[3];
};

// Quad vec3

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

// Oct vec3

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
