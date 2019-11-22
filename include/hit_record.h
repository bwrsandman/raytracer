#pragma once

#include "vec3.h"

class Material;

struct hit_record
{
  float t;
  vec3 p;
  vec3 normal;
  Material* mat_ptr;
};
