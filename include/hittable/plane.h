#pragma once

#include "object.h"

#include <cstdint>

#include "vec3.h"

class Plane : public Object
{
public:
  Plane(vec3 _min, vec3 _max, vec3 _n, uint16_t _m);
  bool hit(const Ray& r,
           float tmin,
           float tmax,
           hit_record& rec) const override;

private:
  const vec3 min, max, n;
  const uint16_t mat_id;
};



