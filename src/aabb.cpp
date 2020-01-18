#include "aabb.h"

#include "ray.h"

#if !__EMSCRIPTEN__
using Raytracer::AabbSimd;
using Raytracer::RaySimd;
using Raytracer::Math::bool_simd_t;
using Raytracer::Math::float_simd_t;
using Raytracer::Math::vec3_simd;
#endif
using Raytracer::Aabb;
using Raytracer::Ray;
using Raytracer::Math::vec3;

bool
Aabb::hit(const Aabb& box, const Ray& r, float t_min, float t_max)
{
  thread_local float reciprocal[3];
  reciprocal[0] = 1.f / r.direction.e[0];
  reciprocal[1] = 1.f / r.direction.e[1];
  reciprocal[2] = 1.f / r.direction.e[2];

  // If the ray enters from back to front (t0 is bigger than t1)
  thread_local bool negative[3];
  negative[0] = std::signbit(r.direction.e[0]);
  negative[1] = std::signbit(r.direction.e[1]);
  negative[2] = std::signbit(r.direction.e[2]);

  for (uint8_t axis = 0; axis < 3; axis++) {
    if ((negative[axis] ? -r.direction.e[axis] : r.direction.e[axis]) <
        std::numeric_limits<float>::epsilon()) {
      continue;
    }
    float ray_origin_axis_scaled(r.origin.e[axis] * reciprocal[axis]);
    float t0 = (&box.min)[negative[axis]].e[axis] * reciprocal[axis] -
               ray_origin_axis_scaled;
    float t1 = (&box.min)[1 - negative[axis]].e[axis] * reciprocal[axis] -
               ray_origin_axis_scaled;

    t_min = std::max(t0, t_min);
    t_max = std::min(t1, t_max);

    if (t_max <= t_min) {
      return false;
    }
  }
  return true;
}

#if !__EMSCRIPTEN__
template<uint8_t D>
bool_simd_t<D>
Raytracer::Aabb::hit(const Aabb& box,
                     const Raytracer::RaySimd<D>& r,
                     float_simd_t<D> t_min,
                     float_simd_t<D> t_max)
{
  thread_local float_simd_t<D> reciprocal[3] = {
    r.direction.e[0].reciprocal(),
    r.direction.e[1].reciprocal(),
    r.direction.e[2].reciprocal(),
  };
  thread_local bool_simd_t<D> swap_mask[3] = {
    reciprocal[0] < float_simd_t<D>(0.f),
    reciprocal[1] < float_simd_t<D>(0.f),
    reciprocal[2] < float_simd_t<D>(0.f),
  };

  thread_local float_simd_t<D> ray_origin_axis_scaled[3] = {
    r.origin.e[0] * reciprocal[0],
    r.origin.e[1] * reciprocal[1],
    r.origin.e[2] * reciprocal[2],
  };

  for (uint8_t axis = 0; axis < 3; axis++) {
    // FIXME: ignoring divide by zero since simd can continue on...
    //        behaviour is uncertain so keep an eye out
    // if (std::abs(r.direction.e[axis]) <
    // std::numeric_limits<float>::epsilon()) {
    //   continue;
    // }
    auto box_min = float_simd_t<D>(box.min.e[axis]);
    auto box_max = float_simd_t<D>(box.max.e[axis]);

    auto t0 =
      box_min.multiply_sub(reciprocal[axis], ray_origin_axis_scaled[axis]);
    auto t1 =
      box_max.multiply_sub(reciprocal[axis], ray_origin_axis_scaled[axis]);

    // If the ray enters from back to front (t0 is bigger than t1)
    std::swap(t0, t1, swap_mask[axis]);

    t_min = std::max(t0, t_min);
    t_max = std::min(t1, t_max);
  }
  return t_max > t_min;
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
#endif
