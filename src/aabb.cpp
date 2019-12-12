#include "aabb.h"

#include "ray.h"

using Raytracer::Aabb;
using Raytracer::AabbSimd;
using Raytracer::Ray;
using Raytracer::RaySimd;
using Raytracer::Math::bool_simd_t;
using Raytracer::Math::float_simd_t;
using Raytracer::Math::vec3;

bool
Aabb::hit(const Aabb& box, const Ray& r, float t_min, float t_max)
{
  for (uint8_t axis = 0; axis < 3; axis++) {
    if (std::abs(r.direction.e[axis]) < std::numeric_limits<float>::epsilon()) {
      continue;
    }
    float reciprocal = 1.f / r.direction.e[axis];
    float ray_origin_axis_scaled(r.origin.e[axis] * reciprocal);
    float t0 = box.min.e[axis] * reciprocal - ray_origin_axis_scaled;
    float t1 = box.max.e[axis] * reciprocal - ray_origin_axis_scaled;

    // If the ray enters from back to front (t0 is bigger than t1)
    if (reciprocal < 0.f) {
      std::swap(t0, t1);
    }

    t_min = std::max(t0, t_min);
    t_max = std::min(t1, t_max);

    if (t_max <= t_min) {
      return false;
    }
  }
  return true;
}

template<uint8_t D>
bool_simd_t<D>
Raytracer::Aabb::hit(const Aabb& box,
                     const Raytracer::RaySimd<D>& r,
                     float_simd_t<D> t_min,
                     float_simd_t<D> t_max)
{
  bool_simd_t<D> result(true);
  for (uint8_t axis = 0; axis < 3; axis++) {
    // FIXME: ignoring divide by zero since simd can continue on...
    //        behaviour is uncertain so keep an eye out
    // if (std::abs(r.direction.e[axis]) <
    // std::numeric_limits<float>::epsilon()) {
    //   continue;
    // }
    float_simd_t<D> box_min = float_simd_t<D>(box.min.e[axis]);
    float_simd_t<D> box_max = float_simd_t<D>(box.max.e[axis]);
    float_simd_t<D> reciprocal = r.direction.e[axis].reciprocal();
    float_simd_t<D> ray_origin_axis_scaled(r.origin.e[axis] * reciprocal);

    float_simd_t<D> t0 =
      box_min.multiply_sub(reciprocal, ray_origin_axis_scaled);
    float_simd_t<D> t1 =
      box_max.multiply_sub(reciprocal, ray_origin_axis_scaled);

    // If the ray enters from back to front (t0 is bigger than t1)
    auto swap_mask = reciprocal < float_simd_t<D>(0.f);
    std::swap(t0, t1, swap_mask);

    t_min = std::max(t0, t_min);
    t_max = std::min(t1, t_max);

    result = result && (t_max > t_min);
    if (!result.any()) {
      return result;
    }
  }
  return result;
}

template<uint8_t D>
bool_simd_t<D>
AabbSimd<D>::hit(const AabbSimd& box,
                 const Ray& r,
                 float_simd_t<D> t_min,
                 float_simd_t<D> t_max)
{
  bool_simd_t<D> result(true);
  for (uint8_t axis = 0; axis < 3; axis++) {
    if (std::abs(r.direction.e[axis]) < std::numeric_limits<float>::epsilon()) {
      continue;
    }

    float reciprocal = 1.f / r.direction.e[axis];
    float_simd_t<D> ray_origin_axis_scaled(r.origin.e[axis] * reciprocal);
    float_simd_t<D> reciprocal_simd4(reciprocal);

    float_simd_t<D> t0 =
      box.min.e[axis].multiply_sub(reciprocal_simd4, ray_origin_axis_scaled);
    float_simd_t<D> t1 =
      box.max.e[axis].multiply_sub(reciprocal_simd4, ray_origin_axis_scaled);

    // If the ray enters from back to front (t0 is bigger than t1)
    if (reciprocal < 0.f) {
      std::swap(t0, t1);
    }

    t_min = std::max(t0, t_min);
    t_max = std::min(t1, t_max);

    result = result && (t_max > t_min);
    if (!result.any()) {
      return result;
    }
  }
  return result;
}

// Define template implementation. If you get linker errors it's probably the
// cause
template bool_simd_t<4>
Raytracer::Aabb::hit(const Aabb& box,
                     const Raytracer::RaySimd<4>& r,
                     float_simd_t<4> t_min,
                     float_simd_t<4> t_max);
template bool_simd_t<8>
Raytracer::Aabb::hit(const Aabb& box,
                     const Raytracer::RaySimd<8>& r,
                     float_simd_t<8> t_min,
                     float_simd_t<8> t_max);
template bool_simd_t<4>
AabbSimd<4>::hit(const AabbSimd<4>& box,
                 const Ray& r,
                 float_simd_t<4> t_min,
                 float_simd_t<4> t_max);
template bool_simd_t<8>
AabbSimd<8>::hit(const AabbSimd<8>& box,
                 const Ray& r,
                 float_simd_t<8> t_min,
                 float_simd_t<8> t_max);
