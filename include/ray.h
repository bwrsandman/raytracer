#pragma once 


#include "vec3.h"

struct Ray
{
public:
  Ray()
    : origin()
    , direction()
  {}
  Ray(const vec3& a, const vec3& b)
    : origin(a)
    , direction(b)
  {
  }
  vec3 point_at_parameter(float t) const { return origin + t * direction; }

  vec3 origin;
  vec3 direction;
};
