#pragma once

#include "object.h"

#include "vec3.h"

struct Sphere : public Object
{
  Sphere(vec3 cen, float r, uint16_t m);
  ~Sphere() override;
  bool hit(const Ray& r,
           bool early_out,
           float t_min,
           float t_max,
           hit_record& rec) const override;

  vec3 center;
  float radius;
  uint16_t mat_id;
};
