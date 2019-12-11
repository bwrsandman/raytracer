#pragma once

#include "object.h"

class vec3;

namespace Raytracer::Hittable {

class AABB : Object
{
  AABB(const vec3& _min, const vec3& _max);
  ~AABB() override;
  bool hit(const Ray& r,
           bool early_out,
           float t_min,
           float t_max,
           hit_record& rec) const override;

  vec3 min, max;
};
} // namespace Raytracer::Hittable