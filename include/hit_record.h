#pragma once

#include <cstdint>

#include "vec2.h"
#include "vec3.h"

struct hit_record
{
  float t;
  vec3 p;
  vec3 normal;
  vec3 tangent;
  vec2 uv;
  uint16_t mat_id;
};
