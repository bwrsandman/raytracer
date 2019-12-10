#pragma once

#include <chrono>

#include "ray.h"

union SDL_Event;

namespace Raytracer {
using Raytracer::Math::vec3;

class Camera
{
public:
  Camera(const vec3& origin,
         const vec3& forward,
         const vec3& up,
         float vfov,
         float aspect);

  Ray get_ray(float s, float t) const;
  void set_aspect(float aspect);
  void calculate_camera();
  /// Call to know if camera related cached changes need to be refreshed
  bool is_dirty() const;
  /// Call once per frame to reset dirty flag
  void set_clean();
  /// Update camera state based on SDL events
  void process_event(const SDL_Event& event, std::chrono::microseconds& dt);

  vec3 look_from;
  vec3 look_at;
  vec3 v_up;
  float v_fov;
  float screen_aspect;

  vec3 origin;
  vec3 lower_left_corner;
  vec3 horizontal;
  vec3 vertical;

  /// Has the camera been changed this frame?
  /// If not, then we can skip a lot of computation.
  bool dirty;
};
} // namespace Raytracer
