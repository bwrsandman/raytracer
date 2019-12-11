#pragma once

#include "object.h"

#include "aabb.h"
#include "math/vec3.h"

namespace Raytracer::Hittable {
using Raytracer::Math::vec3;
using Raytracer::Hittable::AABB;

struct Sphere : public Object
{
  Sphere(vec3 cen, float r, uint16_t m);
  ~Sphere() override;
  bool hit(const Ray& r,
           bool early_out,
           float t_min,
           float t_max,
           hit_record& rec) const override;
  bool bounding_box(AABB& box);

  vec3 center;
  float radius;
  uint16_t mat_id;
  AABB aabb;
};
} // namespace Raytracer::Hittable
