#pragma once

#include "object.h"

struct Point : public Object
{
  Point(vec3 pos, uint16_t m);
  bool hit(const Ray& r,
           float tmin,
           float tmax,
           hit_record& rec) const override;
  vec3 random_point() const override;

  vec3 position;
  uint16_t mat_id;
};
