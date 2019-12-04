#pragma once

#include "object.h"

#include <cstdint>

#include "vec3.h"

struct Point : public Object
{
  Point(vec3 pos, uint16_t m);
  bool hit(const Ray& r,
           bool early_out,
           float t_min,
           float t_max,
           hit_record& rec) const override;

  vec3 position;
  uint16_t mat_id;
};
