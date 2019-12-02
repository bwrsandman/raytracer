#pragma once

#include <algorithm>
#include <iostream>
#include <math.h>
#include <stdlib.h>

class vec3
{
public:
  vec3()
    : e{ 0.0f, 0.0f, 0.0f }
  {}
  vec3(float e0, float e1, float e2)
  {
    e[0] = e0;
    e[1] = e1;
    e[2] = e2;
  }
  inline float x() const { return e[0]; }
  inline float y() const { return e[1]; }
  inline float z() const { return e[2]; }
  inline float r() const { return e[0]; }
  inline float g() const { return e[1]; }
  inline float b() const { return e[2]; }

  inline const vec3& operator+() const { return *this; }
  inline vec3 operator-() const { return vec3(-e[0], -e[1], -e[2]); }
  inline float operator[](int i) const { return e[i]; }
  inline float& operator[](int i) { return e[i]; }

  inline vec3& operator+=(const vec3& v2);
  inline vec3& operator-=(const vec3& v2);
  inline vec3& operator*=(const vec3& v2);
  inline vec3& operator/=(const vec3& v2);
  inline vec3& operator*=(const float t);
  inline vec3& operator/=(const float t);

  inline float length() const
  {
    return sqrt(e[0] * e[0] + e[1] * e[1] + e[2] * e[2]);
  }
  inline float squared_length() const
  {
    return e[0] * e[0] + e[1] * e[1] + e[2] * e[2];
  }
  inline void make_unit_vector();

  float e[3];
};

inline std::istream&
operator>>(std::istream& is, vec3& t)
{
  is >> t.e[0] >> t.e[1] >> t.e[2];
  return is;
}

inline std::ostream&
operator<<(std::ostream& os, const vec3& t)
{
  os << t.e[0] << " " << t.e[1] << " " << t.e[2];
  return os;
}

inline void
vec3::make_unit_vector()
{
  float k = 1.0 / sqrt(e[0] * e[0] + e[1] * e[1] + e[2] * e[2]);
  e[0] *= k;
  e[1] *= k;
  e[2] *= k;
}

inline vec3
operator+(const vec3& v1, const vec3& v2)
{
  return vec3(v1.e[0] + v2.e[0], v1.e[1] + v2.e[1], v1.e[2] + v2.e[2]);
}

inline vec3
operator-(const vec3& v1, const vec3& v2)
{
  return vec3(v1.e[0] - v2.e[0], v1.e[1] - v2.e[1], v1.e[2] - v2.e[2]);
}

inline vec3 operator*(const vec3& v1, const vec3& v2)
{
  return vec3(v1.e[0] * v2.e[0], v1.e[1] * v2.e[1], v1.e[2] * v2.e[2]);
}

inline vec3 operator*(float t, const vec3& v)
{
  return vec3(t * v.e[0], t * v.e[1], t * v.e[2]);
}

inline vec3 operator*(const vec3& v, float t)
{
  return vec3(t * v.e[0], t * v.e[1], t * v.e[2]);
}

inline vec3
operator/(const vec3& v1, const vec3& v2)
{
  return vec3(v1.e[0] / v2.e[0], v1.e[1] / v2.e[1], v1.e[2] / v2.e[2]);
}

inline vec3
operator/(vec3 v, float t)
{
  return vec3(v.e[0] / t, v.e[1] / t, v.e[2] / t);
}

inline float
dot(const vec3& v1, const vec3& v2)
{
  return v1.e[0] * v2.e[0] + v1.e[1] * v2.e[1] + v1.e[2] * v2.e[2];
}

inline vec3
cross(const vec3& v1, const vec3& v2)
{
  return vec3(v1.e[1] * v2.e[2] - v1.e[2] * v2.e[1],
              v1.e[2] * v2.e[0] - v1.e[0] * v2.e[2],
              v1.e[0] * v2.e[1] - v1.e[1] * v2.e[0]);
}

inline vec3&
vec3::operator+=(const vec3& v)
{
  e[0] += v.e[0];
  e[1] += v.e[1];
  e[2] += v.e[2];
  return *this;
}

inline vec3&
vec3::operator-=(const vec3& v)
{
  e[0] -= v.e[0];
  e[1] -= v.e[1];
  e[2] -= v.e[2];
  return *this;
}

inline vec3&
vec3::operator*=(const vec3& v)
{
  e[0] *= v.e[0];
  e[1] *= v.e[1];
  e[2] *= v.e[2];
  return *this;
}

inline vec3&
vec3::operator*=(const float t)
{
  e[0] *= t;
  e[1] *= t;
  e[2] *= t;
  return *this;
}

inline vec3&
vec3::operator/=(const vec3& v)
{
  e[0] /= v.e[0];
  e[1] /= v.e[1];
  e[2] /= v.e[2];
  return *this;
}

inline vec3&
vec3::operator/=(const float t)
{
  float k = 1.0 / t;

  e[0] *= k;
  e[1] *= k;
  e[2] *= k;
  return *this;
}

inline vec3
unit_vector(vec3 v)
{
  return v / v.length();
}

inline double
random_double()
{
  return rand() / (RAND_MAX + 1.0);
}

inline vec3
random_in_unit_sphere()
{
  vec3 p;
  do {
    p = 2.0 * vec3(random_double(), random_double(), random_double()) -
        vec3(1, 1, 1);
  } while (p.squared_length() >= 1.0);
  return p;
}

inline vec3
reflect(const vec3& v, const vec3& n)
{
  return v - 2 * dot(v, n) * n;
}

inline bool
refract_old(const vec3& v, const vec3& n, float ni_over_nt, vec3& refracted)
{
  vec3 uv = unit_vector(v);
  float dt = dot(uv, n);
  float discriminant = 1.0 - ni_over_nt * ni_over_nt * (1 - dt * dt);
  if (discriminant > 0) {
    refracted = ni_over_nt * (uv - n * dt) - n * sqrt(discriminant);
    return true;
  } else {
    //refracted = vec3(1.f, 1.f, 1.f);
    return false;
  }
    
}

inline float
schlick(float cosine, float ref_idx)
{
  float r0 = (1 - ref_idx) / (1 + ref_idx);
  r0 = r0 * r0;

  return r0 + (1 - r0) * pow((1 - cosine), 5);
  //return vec3(r, r, r);
}

// Snells law
inline bool
refract(const vec3& v, vec3& n, float ni, float nt, vec3& refracted)
{
  vec3 uv = unit_vector(v);
  float cosi = dot(uv, n); // cosi()

  if (cosi < 0) {
    // outside in
    cosi = -cosi;
  } else {
    // inside out
    std::swap(ni, nt);
    n = vec3(-n);
  }
  float ni_over_nt = ni / nt;

  // Refraction
  float cost2 = 1.0 - ni_over_nt * ni_over_nt * (1 - cosi * cosi);
  if (cost2 <= 0) {
    refracted = vec3(1.f, 1.f, 1.f);
    return false;
  }
    

  // refracted = ni_over_nt * (uv - n * cosi) - n * sqrt(cost2);

  refracted = ni_over_nt * uv + (ni_over_nt * cosi - sqrt(cost2)) * n;
  return true;
}

// Fresnels law
inline vec3
fresnel_rate(const vec3& v, vec3& n, float ni, float nt)
{
  //	  1 // ni cosi - nt cost \2   / ni cost - nt cosi \2\
  // Fr = - || ----------------- |  + | ----------------- | |
  //	  2 \\ ni cosi + nt cost /    \ ni cost + nt cosi / /

  //			---------------------
  //		   /	/ ni		\2
  // cost =	  / 1 - | -- * sini |
  //		\/		\ nt		/
  float ni_over_nt = (ni / nt);
  vec3 uv = unit_vector(v);
  vec3 un = unit_vector(n);
  float cosi = std::min(1.0f, std::max(-1.0f, dot(uv, un)));

  //if (cosi >= 0) {
  //  // Inside object
  //  std::swap(ni, nt);
  //}

  // using Snells law (from cosi to sini to sint)
  float cost2 = 1.f - (ni_over_nt * ni_over_nt * (1 - cosi * cosi));
  // Total internal reflection
  //if (cost2 < 0) {
  //  return vec3(1.0, 1.0, 1.0);
 // }

  // from sint to cost
  float cost = sqrtf(cost2);
  cosi = fabsf(cosi);

  float rs = (ni * cosi - nt * cost) / (ni * cost + nt * cosi);
  float rp = (ni * cosi - nt * cost) / (ni * cost + nt * cosi);

  float fr = (rs * rs + rp * rp) * 0.5f;
  if (fr > 1.0f) {
    //printf("whoops: %f \n", fr);
    fr = 1.0f;
  }

  return vec3(fr, fr, fr);
  // As a consequence of the conservation of energy, transmittance is given by:
  // ft = 1 - fr;
}



inline vec3
reciprocal(const vec3& v)
{
  return vec3(1.0 / v.e[0], 1.0 / v.e[1], 1.0 / v.e[2]);
}

static vec3
min(const vec3& lhs, const vec3& rhs)
{
  return vec3(std::min(lhs.e[0], rhs.e[0]),
              std::min(lhs.e[1], rhs.e[1]),
              std::min(lhs.e[2], rhs.e[2]));
}

static vec3
max(const vec3& lhs, const vec3& rhs)
{
  return vec3(std::max(lhs.e[0], rhs.e[0]),
              std::max(lhs.e[1], rhs.e[1]),
              std::max(lhs.e[2], rhs.e[2]));
}

static vec3
clamp(const vec3& val, const vec3& minimum, const vec3& maximum)
{
  return min(maximum, max(minimum, val));
}

static vec3
saturate(const vec3& val)
{
  return clamp(val, vec3(0.0f, 0.0f, 0.0f), vec3(1.0f, 1.0f, 1.0f));
}

static vec3
lerp(const vec3& from, const vec3& to, float t)
{
  return from * t + to * (1.0f - t);
}
