#pragma once

#include "object.h"

class LineSegment : public Object
{
public:
  LineSegment(vec3 pos[2], uint16_t m);
  bool hit(const Ray& r,
           float tmin,
           float tmax,
           hit_record& rec) const override;
  vec3 random_point() const override;

private:
  const vec3 position[2];
  const uint16_t mat_id;
};
