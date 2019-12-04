#pragma once

#include "object.h"

#include <cstdint>

#include "vec3.h"

class Translate : public Object
{
public:
  Translate(Object* _p, vec3 _offset);
  bool hit(const Ray& r,
           bool early_out,
           float t_min,
           float t_max,
           hit_record& rec) const override;

private:
  const Object* p;
  const vec3 offset;
};

class Rotate_y : public Object
{
public:
  Rotate_y(Object* _p, float _angle);
  bool hit(const Ray& r,
           bool early_out,
           float t_min,
           float t_max,
           hit_record& rec) const override;

private:
  const Object* p;
  float sin_theta;
  float cos_theta;
};
