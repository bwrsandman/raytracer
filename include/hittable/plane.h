#pragma once

#include "object.h"

#include <cstdint>

#include "aabb.h"
#include "math/vec3.h"

namespace Raytracer::Hittable {
using Raytracer::Aabb;
using Raytracer::Math::vec3;

class Plane : public Object
{
public:
  Plane(vec3 _min, vec3 _max, vec3 _n, uint16_t _m);
  bool hit(const Ray& r,
           bool early_out,
           float t_min,
           float t_max,
           hit_record& rec) const override;
  uint16_t get_mat_id() const override;
  std::unique_ptr<Object> copy() const override;
  bool bounding_box(Aabb& box);

  const vec3 min, max, n;
  const uint16_t mat_id;
  Aabb aabb;
};
} // namespace Raytracer::Hittable
