#pragma once 

#include "ray.h"

class Camera
{
public:
  Camera()
  {
    lower_left_corner = vec3(-4.0, 3.0, -3.0);
    horizontal = vec3(8.0, 0.0, 0.0);
    vertical = vec3(0.0, -6.0, 0.0);
    origin = vec3(0.0, 0.0, 0.0);
  }
  Ray get_ray(float u, float v)
  {
    auto direction = lower_left_corner + u * horizontal + v * vertical - origin;
    direction.make_unit_vector();
    return Ray(origin, direction);
  }

  vec3 origin;
  vec3 lower_left_corner;
  vec3 horizontal;
  vec3 vertical;
};
