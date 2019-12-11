#pragma once

#include <cstdint>
#include <immintrin.h>

#include "bool_simd.h"

namespace Raytracer::Math {

namespace _details_float_simd_t {
template<uint8_t D>
struct raw_type;

template<>
struct raw_type<4>
{
  typedef __m128 type;
};

template<>
struct raw_type<8>
{
  typedef __m256 type;
};
} // namespace _details_float_simd_t

template<uint8_t D>
struct float_simd_t
{
  using raw_type_t = typename _details_float_simd_t::raw_type<D>::type;

  inline explicit float_simd_t(float value);
  inline explicit float_simd_t(raw_type_t value);
  inline explicit float_simd_t(const float (&values)[D]);
  inline explicit float_simd_t(const float values[]);

  template<uint8_t index>
  inline float get_scalar() const
  {
    static_assert(index < D, "scalar index out of bounds");
    union scalar_getter_t
    {
      bool scalar[D];
      raw_type_t vector;
    };
    return ((scalar_getter_t*)(&_raw))->scalar[index];
  }

  inline float_simd_t operator+(float_simd_t rhs) const;
  inline float_simd_t operator-(float_simd_t rhs) const;
  inline float_simd_t operator*(float_simd_t rhs) const;
  inline bool_simd_t<D> operator>(float_simd_t rhs) const;
  inline bool_simd_t<D> operator<(float_simd_t rhs) const;
  inline bool_simd_t<D> operator>=(float_simd_t rhs) const;
  inline bool_simd_t<D> operator<=(float_simd_t rhs) const;
  inline bool_simd_t<D> operator==(float_simd_t rhs) const;
  inline bool_simd_t<D> operator!=(float_simd_t rhs) const;

  /// multiply add: this * multiplier + addition
  inline float_simd_t multiply_add(float_simd_t multiplier,
                                   float_simd_t addition) const;
  /// multiply sub: this * multiplier - subtraction
  inline float_simd_t multiply_sub(float_simd_t multiplier,
                                   float_simd_t subtraction) const;

  raw_type_t _raw;
};

// Quad float

template<>
inline float_simd_t<4>::float_simd_t(float value)
  : _raw(_mm_set_ps1(value))
{}

template<>
inline float_simd_t<4>::float_simd_t(__m128 value)
  : _raw(value)
{}

template<>
inline float_simd_t<4>::float_simd_t(const float (&values)[4])
  : _raw(_mm_set_ps(values[3], values[2], values[1], values[0]))
{}

template<>
inline float_simd_t<4>::float_simd_t(const float values[])
  : _raw(_mm_set_ps(values[3], values[2], values[1], values[0]))
{}

template<>
inline float_simd_t<4>
float_simd_t<4>::operator+(float_simd_t rhs) const
{
  return float_simd_t{ _mm_add_ps(_raw, rhs._raw) };
}

template<>
inline float_simd_t<4>
float_simd_t<4>::operator-(float_simd_t rhs) const
{
  return float_simd_t{ _mm_sub_ps(_raw, rhs._raw) };
}

template<>
inline float_simd_t<4> float_simd_t<4>::operator*(float_simd_t rhs) const
{
  return float_simd_t{ _mm_mul_ps(_raw, rhs._raw) };
}

template<>
inline bool_simd_t<4>
float_simd_t<4>::operator>(float_simd_t rhs) const
{
  return bool_simd_t<4>{ _mm_cmpgt_ps(_raw, rhs._raw) };
}

template<>
inline bool_simd_t<4>
float_simd_t<4>::operator<(float_simd_t rhs) const
{
  return bool_simd_t<4>{ _mm_cmplt_ps(_raw, rhs._raw) };
}

template<>
inline bool_simd_t<4>
float_simd_t<4>::operator>=(float_simd_t rhs) const
{
  return bool_simd_t<4>{ _mm_cmpge_ps(_raw, rhs._raw) };
}

template<>
inline bool_simd_t<4>
float_simd_t<4>::operator<=(float_simd_t rhs) const
{
  return bool_simd_t<4>{ _mm_cmple_ps(_raw, rhs._raw) };
}

template<>
inline bool_simd_t<4>
float_simd_t<4>::operator==(float_simd_t rhs) const
{
  return bool_simd_t<4>{ _mm_cmpeq_ps(_raw, rhs._raw) };
}

template<>
inline bool_simd_t<4>
float_simd_t<4>::operator!=(float_simd_t rhs) const
{
  return bool_simd_t<4>{ _mm_cmpneq_ps(_raw, rhs._raw) };
}

template<>
inline float_simd_t<4>
float_simd_t<4>::multiply_add(float_simd_t multiplier,
                              float_simd_t addition) const
{
  return float_simd_t{ _mm_fmadd_ps(_raw, multiplier._raw, addition._raw) };
}

template<>
inline float_simd_t<4>
float_simd_t<4>::multiply_sub(float_simd_t multiplier,
                              float_simd_t subtraction) const
{
  return float_simd_t{ _mm_fmsub_ps(_raw, multiplier._raw, subtraction._raw) };
}

} // namespace Raytracer::Math

namespace std {
using Raytracer::Math::float_simd_t;

// Quad floats
inline float_simd_t<4>
min(float_simd_t<4> lhs, float_simd_t<4> rhs)
{
  return float_simd_t<4>{ _mm_min_ps(lhs._raw, rhs._raw) };
}
inline float_simd_t<4>
max(float_simd_t<4> lhs, float_simd_t<4> rhs)
{
  return float_simd_t<4>{ _mm_max_ps(lhs._raw, rhs._raw) };
}
} // namespace std
