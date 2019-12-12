#pragma once

#include "object.h"

#include <cstdint>

#include "math/mat4.h"
#include "math/quat.h"
#include "math/vec3.h"
#include "math/vec4.h"

namespace Raytracer::Hittable {
using Raytracer::Math::mat4;
using Raytracer::Math::quat;
using Raytracer::Math::vec3;
using Raytracer::Math::vec4;

class Translate : public Object
{
public:
  Translate(std::unique_ptr<Object>&& _p, vec3 _offset);
  bool hit(const Ray& r,
           bool early_out,
           float t_min,
           float t_max,
           hit_record& rec) const override;
  void build_bvh(){};

private:
  const std::unique_ptr<Object> p;
  const vec3 offset;
};

class Rotate : public Object
{
public:
  Rotate(std::unique_ptr<Object>&& _p, vec4 _axis);
  bool hit(const Ray& r,
           bool early_out,
           float t_min,
           float t_max,
           hit_record& rec) const override;
  void build_bvh() override;

private:
  const std::unique_ptr<Object> p;
  quat total_rot;
  quat local_rot;
  mat4 rotation, inv_rotation;
  vec4 axis;
};



} // namespace Raytracer::Hittable
