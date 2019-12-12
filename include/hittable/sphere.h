#pragma once

#include "object.h"

#include "aabb.h"
#include "math/vec3.h"

namespace Raytracer::Hittable {
using Raytracer::Aabb;
using Raytracer::Math::vec3;

struct Sphere : public Object
{
  Sphere(vec3 cen, float r, uint16_t m);
  ~Sphere() override;
  bool hit(const Ray& r,
           bool early_out,
           float t_min,
           float t_max,
           hit_record& rec) const override;
  bool bounding_box(Aabb& box);
  void build_bvh() {};

  vec3 center;
  float radius;
  uint16_t mat_id;
  Aabb aabb;
};
} // namespace Raytracer::Hittable
