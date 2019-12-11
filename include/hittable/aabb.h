#pragma once

#include "object.h"

#include <cstdint>

#include "math/vec3.h"

namespace Raytracer::Hittable {
using Raytracer::Math::vec3;

class AABB : public Object
{
public:
  AABB();
  AABB(const vec3& _min, const vec3& _max);
  ~AABB() override;
  bool hit(const Ray& r,
           bool early_out,
           float t_min,
           float t_max,
           hit_record& rec) const override;

private:
  vec3 min, max;
};
} // namespace Raytracer::Hittable