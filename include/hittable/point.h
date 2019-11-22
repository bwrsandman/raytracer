#pragma once

#include "object.h"

class Point : public Object
{
public:
  Point(vec3 pos, uint16_t m);
  bool hit(const Ray& r,
           float tmin,
           float tmax,
           hit_record& rec) const override;
  vec3 random_point() const override;

private:
  const vec3 position;
  const uint16_t mat_id;
};
