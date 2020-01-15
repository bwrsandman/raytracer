#ifndef RAYTRACER_RAY_T_H
#define RAYTRACER_RAY_T_H

#define RAY_STATUS_DEAD 0
#define RAY_STATUS_ACTIVE 1

struct ray_t
{
  /// x, y, z, unused
  vec4 origin;
  /// x, y, z, status
  vec4 direction;
};

static vec4
ray_point_at(ray_t ray, float t)
{
  vec4 direction = ray.direction;
  direction[3] = 0.0f; // blank status
  return ray.origin + direction * t;
}

#endif // RAYTRACER_RAY_T_H
