#pragma once

#include "object.h"

#include <memory>

#include "vec3.h"

class Sphere : public Object
{
public:
  Sphere(vec3 cen, float r, uint16_t m);
  ~Sphere() override;
  bool hit(const Ray& r,
           float tmin,
           float tmax,
           hit_record& rec) const override;
  vec3 random_point() const override;

private:
  const vec3 center;
  const float radius;
  const uint16_t mat_id;
};
