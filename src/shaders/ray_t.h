#ifndef RAYTRACER_RAY_T_H
#define RAYTRACER_RAY_T_H

struct ray_t
{
  vec4 origin;
  vec4 direction;
};

vec4
ray_point_at(ray_t ray, float t)
{
  return ray.origin + t * ray.direction;
}

#endif // RAYTRACER_RAY_T_H
