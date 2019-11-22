#pragma once

#include "object.h"

#include "vec3.h"

class Material;

class Sphere : public Object
{
public:
  Sphere(vec3 cen, float r, Material* m)
    : center(cen)
    , radius(r)
    , mat_ptr(m){};
  bool hit(const Ray& r,
           float tmin,
           float tmax,
           hit_record& rec) const override;
  vec3 center;
  float radius;
  Material* mat_ptr;
};
