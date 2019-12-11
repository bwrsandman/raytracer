#pragma once

#include "math/vec3.h"

namespace Raytracer {
using Raytracer::Math::vec3;

struct Ray;
struct hit_record;

struct Aabb
{
  vec3 min, max;

  static bool hit(const Aabb& box, const Ray& r, float t_min, float t_max);
};

static_assert(sizeof(Aabb) == 24,
              "axis-aligned bounding box is not minimal size");

} // namespace Raytracer
