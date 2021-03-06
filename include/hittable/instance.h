#pragma once

#include "object.h"

#include <cstdint>

#include "math/vec3.h"

namespace Raytracer::Hittable {
using Raytracer::Math::vec3;

class Translate : public Object
{
public:
  Translate(const Object* _p, vec3 _offset);
  bool hit(const Ray& r,
           bool early_out,
           float t_min,
           float t_max,
           hit_record& rec) const override;
  uint16_t get_mat_id() const override;
  std::unique_ptr<Object> copy() const override;

private:
  const Object* p;
  const vec3 offset;
};

class Rotate_y : public Object
{
public:
  Rotate_y(const Object* _p, float _angle);
  bool hit(const Ray& r,
           bool early_out,
           float t_min,
           float t_max,
           hit_record& rec) const override;
  uint16_t get_mat_id() const override;
  std::unique_ptr<Object> copy() const override;

private:
  const Object* p;
  float angle;
  float sin_theta;
  float cos_theta;
};
} // namespace Raytracer::Hittable
