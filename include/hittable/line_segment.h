#pragma once

#include "object.h"

#include <cstdint>

#include "math/vec3.h"

namespace Raytracer::Hittable {
using Raytracer::Math::vec3;

struct LineSegment : public Object
{
  LineSegment(const vec3 pos[2], uint16_t m);
  bool hit(const Ray& r,
           bool early_out,
           float t_min,
           float t_max,
           hit_record& rec) const override;
  uint16_t get_mat_id() const override;
  std::unique_ptr<Object> copy() const override;

  vec3 position[2];
  uint16_t mat_id;
};
} // namespace Raytracer::Hittable
