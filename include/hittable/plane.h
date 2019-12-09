#pragma once

#include "object.h"

#include <cstdint>

#include "math/vec3.h"

namespace Raytracer::Hittable {
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

private:
  const vec3 min, max, n;
  const uint16_t mat_id;
};
} // namespace Raytracer::Hittable
