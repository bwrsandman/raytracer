#pragma once

#include "object.h"

#include <cstdint>
#include <functional>
#include <memory>

#include "vec3.h"

struct FunctionalGeometry : public Object
{
  typedef std::function<float(const vec3&)> signed_distance_function_t;

  FunctionalGeometry(const vec3& center,
                     uint8_t max_steps,
                     signed_distance_function_t sdf,
                     uint16_t m);

  bool hit(const Ray& r,
           bool early_out,
           float t_min,
           float t_max,
           hit_record& rec) const override;
  const signed_distance_function_t sdf;
  const uint8_t max_steps;
  vec3 center;
  uint16_t mat_id;
};
