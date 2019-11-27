#pragma once

#include "object.h"

#include <memory>

#include "vec3.h"

struct Sphere : public Object
{
  Sphere(vec3 cen, float r, uint16_t m);
  ~Sphere() override;
  bool hit(const Ray& r,
           float tmin,
           float tmax,
           hit_record& rec) const override;
  vec3 random_point() const override;

  vec3 center;
  float radius;
  uint16_t mat_id;
};
