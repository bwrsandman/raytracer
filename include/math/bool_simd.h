#pragma once

#include <cstdint>
#include <immintrin.h>

namespace Raytracer::Math {

namespace _details_bool_simd_t {
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
} // namespace _details_bool_simd_t

template<uint8_t D>
struct bool_simd_t
{
  using raw_type_t = typename _details_bool_simd_t::raw_type<D>::type;

  inline explicit bool_simd_t(bool value);
  inline explicit bool_simd_t(raw_type_t value);
  inline explicit bool_simd_t(const bool (&values)[D]);
  inline explicit bool_simd_t(const bool values[]);

  template<uint8_t index>
  inline bool get_scalar() const
  {
    static_assert(index < D, "scalar index out of bounds");
    union scalar_getter_t
    {
      bool scalar[D];
      raw_type_t vector;
    };
    return ((scalar_getter_t*)(&_raw))->scalar[index];
  }

  inline bool_simd_t operator&&(bool_simd_t rhs) const;
  inline bool_simd_t operator||(bool_simd_t rhs) const;
  inline bool_simd_t and_not(bool_simd_t rhs) const;
  inline bool any() const;
  inline bool all() const;

  raw_type_t _raw;
};

// Quad bool

template<>
inline bool_simd_t<4>::bool_simd_t(bool value)
  : _raw(_mm_cmp_ps(_mm_set1_ps(value), _mm_set1_ps(0.0f), _CMP_NEQ_UQ))
{}

template<>
inline bool_simd_t<4>::bool_simd_t(
  typename _details_bool_simd_t::raw_type<4>::type value)
  : _raw(_mm_cmp_ps(value, _mm_set1_ps(0.0f), _CMP_NEQ_UQ))
{}

template<>
inline bool_simd_t<4>::bool_simd_t(const bool (&values)[4])
  : _raw(_mm_cmp_ps(_mm_set_ps(values[3], values[2], values[1], values[0]),
                    _mm_set1_ps(0.0f),
                    _CMP_NEQ_UQ))
{}

template<>
inline bool_simd_t<4>::bool_simd_t(const bool values[])
  : _raw(_mm_cmp_ps(_mm_set_ps(values[3], values[2], values[1], values[0]),
                    _mm_set1_ps(0.0f),
                    _CMP_NEQ_UQ))
{}

template<>
inline bool_simd_t<4>
bool_simd_t<4>::operator&&(bool_simd_t rhs) const
{
  return bool_simd_t{ _mm_and_ps(_raw, rhs._raw) };
}

template<>
inline bool_simd_t<4>
bool_simd_t<4>::operator||(bool_simd_t rhs) const
{
  return bool_simd_t{ _mm_or_ps(_raw, rhs._raw) };
}

template<>
inline bool_simd_t<4>
bool_simd_t<4>::and_not(bool_simd_t rhs) const
{
  return bool_simd_t{ _mm_andnot_ps(_raw, rhs._raw) };
}

template<>
inline bool
bool_simd_t<4>::any() const
{
  return _mm_movemask_ps(_raw) != 0;
}

template<>
inline bool
bool_simd_t<4>::all() const
{
  return _mm_movemask_ps(_raw) == 0xF;
}

} // namespace Raytracer::Math
