#pragma once

#include "vec3.h"

#include <cstdint>

struct hit_record
{
  float t;
  vec3 p;
  vec3 normal;
  uint16_t mat_id;
};
