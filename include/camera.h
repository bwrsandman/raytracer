#pragma once

#include "ray.h"

class Camera
{
public:
  Camera(vec3 lookfrom, vec3 lookat, vec3 vup, float vfov, float aspect)
    : look_from(lookfrom)
    , look_at(lookat)
    , v_up(vup)
    , v_fov(vfov)
    , screen_aspect(aspect)
  {
    calculate_camera();
  }
  Ray get_ray(float s, float t)
  {
    auto direction = lower_left_corner + s * horizontal + t * vertical - origin;
    direction.make_unit_vector();
    return Ray(origin, direction);
  }

  void change_camera(vec3 c_lookfrom,
                     vec3 c_lookat,
                     vec3 c_vup,
                     float c_vfov,
                     float c_aspect)
  {
    look_from += c_lookfrom;
    look_at += c_lookat;
    v_up += c_vup;
    v_fov += c_vfov;
    screen_aspect += c_aspect;
  }

  void calculate_camera()
  {
    vec3 u, v, w;
    float theta = v_fov * (3.14159265359) / 180;
    float half_height = tan(theta / 2);
    float half_width = screen_aspect * half_height;

    origin - look_from;
    w = unit_vector(look_from - look_at);
    u = unit_vector(cross(v_up, w));
    v = -cross(w, u);

    lower_left_corner = origin - half_width * u - half_height * v - w;
    horizontal = 2 * half_width * u;
    vertical = 2 * half_height * v;
  }

  vec3 look_from;
  vec3 look_at;
  vec3 v_up;
  float v_fov;
  float screen_aspect;

  vec3 origin;
  vec3 lower_left_corner;
  vec3 horizontal;
  vec3 vertical;
};
