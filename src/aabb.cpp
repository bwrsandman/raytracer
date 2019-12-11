#include "aabb.h"

#include "ray.h"

using Raytracer::Aabb;
using Raytracer::Ray;
using Raytracer::Math::vec3;

bool
Aabb::hit(const Aabb& box, const Ray& r, float t_min, float t_max)
{
  for (uint8_t axis = 0; axis < 3; axis++) {
    if (std::abs(r.direction.e[axis]) < std::numeric_limits<float>::epsilon()) {
      continue;
    }
    float reciprocal = 1.f / r.direction.e[axis];
    float t0 = (box.min.e[axis] - r.origin.e[axis]) * reciprocal;
    float t1 = (box.max.e[axis] - r.origin.e[axis]) * reciprocal;

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
