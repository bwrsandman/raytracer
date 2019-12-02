#pragma once

#include "object.h"

#include <cstdint>

#include "vec3.h"

class Plane : public Object
{
public:
  //Plane(float _x0, float _x1, float _y0, float _y1, float _k, uint16_t _m);
  Plane(float _p0, vec3 top_right, vec3 _norm, uint16_t _m);
  bool hit(const Ray& r,
           float tmin,
           float tmax,
           hit_record& rec) const override;

private:
  //const float x0, x1, y0, y1, k;
  float p0;
  vec3 norm;
  const uint16_t mat_id;
};
