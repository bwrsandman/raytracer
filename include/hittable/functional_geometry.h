#pragma once

#include "object.h"

#include <cstdint>
#include <functional>
#include <memory>

#include "aabb.h"
#include "math/vec3.h"

namespace Raytracer::Hittable {
using Raytracer::Aabb;
using Raytracer::Math::vec3;

struct FunctionalGeometry : public Object
{
  typedef std::function<float(const vec3&)> signed_distance_function_t;

  FunctionalGeometry(const vec3& center,
                     uint8_t max_steps,
                     signed_distance_function_t sdf,
                     uint16_t m);

  static std::unique_ptr<FunctionalGeometry> mandrelbulb(const vec3& center,
                                                         uint8_t max_iterations,
                                                         float max_radius,
                                                         float power,
                                                         uint16_t m);

  bool hit(const Ray& r,
           bool early_out,
           float t_min,
           float t_max,
           hit_record& rec) const override;
  bool bounding_box(Aabb& box);

  const signed_distance_function_t sdf;
  const uint8_t max_steps;
  vec3 center;
  uint16_t mat_id;
  Aabb aabb;
};
} // namespace Raytracer::Hittable
