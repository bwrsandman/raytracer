#pragma once

#include "object.h"

struct LineSegment : public Object
{
  LineSegment(const vec3 pos[2], uint16_t m);
  bool hit(const Ray& r,
           float tmin,
           float tmax,
           hit_record& rec) const override;
  vec3 random_point() const override;

  vec3 position[2];
  uint16_t mat_id;
};
