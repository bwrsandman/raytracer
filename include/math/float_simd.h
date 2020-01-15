#pragma once

#include <cstdint>

#include <array>

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
  inline constexpr static float get_scalar(const float_simd_t& vector)
  {
    static_assert(index < D, "scalar index out of bounds");
    union scalar_getter_t
    {
      float scalar[D];
      raw_type_t vector;
    };
    return ((scalar_getter_t*)(&vector._raw))->scalar[index];
  }
  inline constexpr static std::array<float, D> get_scalars(
    const float_simd_t& vector);

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
  /// 1 / this
  inline float_simd_t reciprocal() const;
  /// sqrt(1 / this)
  inline float_simd_t reciprocal_sqrt() const;

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

template<>
inline float_simd_t<4>
float_simd_t<4>::reciprocal() const
{
  return float_simd_t{ _mm_rcp_ps(_raw) };
}

template<>
inline float_simd_t<4>
float_simd_t<4>::reciprocal_sqrt() const
{
  return float_simd_t{ _mm_rsqrt_ps(_raw) };
}

template<>
constexpr std::array<float, 4>
float_simd_t<4>::get_scalars(const float_simd_t<4>& vector)
{
  return {
    get_scalar<0>(vector),
    get_scalar<1>(vector),
    get_scalar<2>(vector),
    get_scalar<3>(vector),
  };
}

// Oct float

template<>
inline float_simd_t<8>::float_simd_t(float value)
  : _raw(_mm256_set1_ps(value))
{}

template<>
inline float_simd_t<8>::float_simd_t(__m256 value)
  : _raw(value)
{}

template<>
inline float_simd_t<8>::float_simd_t(const float (&values)[8])
  : _raw(_mm256_set_ps(values[7],
                       values[6],
                       values[5],
                       values[4],
                       values[3],
                       values[2],
                       values[1],
                       values[0]))
{}

template<>
inline float_simd_t<8>::float_simd_t(const float values[])
  : _raw(_mm256_set_ps(values[7],
                       values[6],
                       values[5],
                       values[4],
                       values[3],
                       values[2],
                       values[1],
                       values[0]))
{}

template<>
inline float_simd_t<8>
float_simd_t<8>::operator+(float_simd_t rhs) const
{
  return float_simd_t{ _mm256_add_ps(_raw, rhs._raw) };
}

template<>
inline float_simd_t<8>
float_simd_t<8>::operator-(float_simd_t rhs) const
{
  return float_simd_t{ _mm256_sub_ps(_raw, rhs._raw) };
}

template<>
inline float_simd_t<8> float_simd_t<8>::operator*(float_simd_t rhs) const
{
  return float_simd_t{ _mm256_mul_ps(_raw, rhs._raw) };
}

template<>
inline bool_simd_t<8>
float_simd_t<8>::operator>(float_simd_t rhs) const
{
  return bool_simd_t<8>{ _mm256_cmp_ps(_raw, rhs._raw, _CMP_GT_OQ) };
}

template<>
inline bool_simd_t<8>
float_simd_t<8>::operator<(float_simd_t rhs) const
{
  return bool_simd_t<8>{ _mm256_cmp_ps(_raw, rhs._raw, _CMP_LT_OQ) };
}

template<>
inline bool_simd_t<8>
float_simd_t<8>::operator>=(float_simd_t rhs) const
{
  return bool_simd_t<8>{ _mm256_cmp_ps(_raw, rhs._raw, _CMP_GE_OQ) };
}

template<>
inline bool_simd_t<8>
float_simd_t<8>::operator<=(float_simd_t rhs) const
{
  return bool_simd_t<8>{ _mm256_cmp_ps(_raw, rhs._raw, _CMP_LE_OQ) };
}

template<>
inline bool_simd_t<8>
float_simd_t<8>::operator==(float_simd_t rhs) const
{
  return bool_simd_t<8>{ _mm256_cmp_ps(_raw, rhs._raw, _CMP_EQ_OQ) };
}

template<>
inline bool_simd_t<8>
float_simd_t<8>::operator!=(float_simd_t rhs) const
{
  return bool_simd_t<8>{ _mm256_cmp_ps(_raw, rhs._raw, _CMP_NEQ_OQ) };
}

template<>
inline float_simd_t<8>
float_simd_t<8>::multiply_add(float_simd_t multiplier,
                              float_simd_t addition) const
{
  return float_simd_t{ _mm256_fmadd_ps(_raw, multiplier._raw, addition._raw) };
}

template<>
inline float_simd_t<8>
float_simd_t<8>::multiply_sub(float_simd_t multiplier,
                              float_simd_t subtraction) const
{
  return float_simd_t{ _mm256_fmsub_ps(
    _raw, multiplier._raw, subtraction._raw) };
}

template<>
inline float_simd_t<8>
float_simd_t<8>::reciprocal() const
{
  return float_simd_t{ _mm256_rcp_ps(_raw) };
}

template<>
inline float_simd_t<8>
float_simd_t<8>::reciprocal_sqrt() const
{
  return float_simd_t{ _mm256_rsqrt_ps(_raw) };
}

template<>
constexpr std::array<float, 8>
float_simd_t<8>::get_scalars(const float_simd_t<8>& vector)
{
  return {
    get_scalar<0>(vector), get_scalar<1>(vector), get_scalar<2>(vector),
    get_scalar<3>(vector), get_scalar<4>(vector), get_scalar<5>(vector),
    get_scalar<6>(vector), get_scalar<7>(vector),
  };
}

} // namespace Raytracer::Math

namespace std {
using Raytracer::Math::bool_simd_t;
using Raytracer::Math::float_simd_t;

// Quad floats
inline float_simd_t<4>
abs(float_simd_t<4> value)
{
  thread_local __m128i sign_mask_epi32 = _mm_set1_epi32(0x7FFFFFFFU);
  thread_local __m128 sign_mask = *reinterpret_cast<__m128*>(&sign_mask_epi32);
  return float_simd_t<4>{ _mm_and_ps(value._raw, sign_mask) };
}
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

inline void
swap(float_simd_t<4>& lhs, float_simd_t<4>& rhs, bool_simd_t<4> mask)
{
  //  true  false   mix with a mask
  // c = a; c = a; c = a;
  // a = b; a = a; a = (b & mask) | (a & ~mask)
  // b = c; b = b; b = (c & mask) | (b & ~mask)

  float_simd_t<4> temp = lhs;
  lhs._raw = _mm_or_ps(_mm_and_ps(rhs._raw, mask._raw),
                       _mm_andnot_ps(lhs._raw, mask._raw));
  rhs._raw = _mm_or_ps(_mm_and_ps(temp._raw, mask._raw),
                       _mm_andnot_ps(rhs._raw, mask._raw));
}

inline void
swap(float_simd_t<8>& lhs, float_simd_t<8>& rhs, bool_simd_t<8> mask)
{
  //  true  false   mix with a mask
  // c = a; c = a; c = a;
  // a = b; a = a; a = (b & mask) | (a & ~mask)
  // b = c; b = b; b = (c & mask) | (b & ~mask)

  float_simd_t<8> temp = lhs;
  lhs._raw = _mm256_or_ps(_mm256_and_ps(rhs._raw, mask._raw),
                          _mm256_andnot_ps(lhs._raw, mask._raw));
  rhs._raw = _mm256_or_ps(_mm256_and_ps(temp._raw, mask._raw),
                          _mm256_andnot_ps(rhs._raw, mask._raw));
}

// Oct floats
inline float_simd_t<8>
min(float_simd_t<8> lhs, float_simd_t<8> rhs)
{
  return float_simd_t<8>{ _mm256_min_ps(lhs._raw, rhs._raw) };
}
inline float_simd_t<8>
max(float_simd_t<8> lhs, float_simd_t<8> rhs)
{
  return float_simd_t<8>{ _mm256_max_ps(lhs._raw, rhs._raw) };
}
} // namespace std
