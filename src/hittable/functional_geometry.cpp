#include "hittable/functional_geometry.h"

#include "hit_record.h"
#include "ray.h"
#include "sdf.h"

FunctionalGeometry::FunctionalGeometry(const vec3& center,
                                       uint8_t max_steps,
                                       signed_distance_function_t sdf,
                                       uint16_t m)
  : sdf(std::move(sdf))
  , max_steps(max_steps)
  , center(center)
  , mat_id(m)
{}

bool
FunctionalGeometry::hit(const Ray& r,
                        float tmin,
                        float tmax,
                        hit_record& rec) const
{
  static constexpr float f32_1_2PI = 0.5f / M_PI;
  static constexpr float f32_1_PI = 1.0f / M_PI;
  static constexpr float grad_step = 0.05f;

  Ray ray{ r.origin - center, r.direction };
  float t = tmin;
  float distance = 10000.0f;
  float dt = 0.0f;
  // Ray march
  for (uint8_t steps = 0; steps < max_steps; steps++) {
    auto p = ray.point_at_parameter(t);
    distance = sdf(p);
    if (distance < 0.001f) {
      break;
    }
    dt = std::min(std::abs(distance), 0.1f);
    t += dt;
    if (t > tmax) {
      break;
    }
  }
  if (distance >= 0.001f) {
    return false;
  }

  t -= dt;

  for (int i = 0; i < 4; i++) {
    dt *= 0.5f;

    vec3 p = ray.point_at_parameter(t + dt);
    if (sdf(p) >= 0.001f) {
      t += dt;
    }
  }

  rec.t = t;
  rec.p = ray.point_at_parameter(t);

  // Take gradient
  const vec3 dx = vec3(grad_step, 0.0, 0.0);
  const vec3 dy = vec3(0.0, grad_step, 0.0);
  const vec3 dz = vec3(0.0, 0.0, grad_step);
  rec.normal = vec3(sdf(rec.p + dx) - sdf(rec.p - dx),
                    sdf(rec.p + dy) - sdf(rec.p - dy),
                    sdf(rec.p + dz) - sdf(rec.p - dz));
  rec.normal.make_unit_vector();
  rec.tangent = cross(rec.normal, vec3(0, 1, 0));
  rec.tangent.make_unit_vector();
  rec.uv.e[0] = 0.5f + std::atan2(-rec.normal.z(), rec.normal.x()) * f32_1_2PI;
  rec.uv.e[1] = 0.5f - std::asin(-rec.normal.y()) * f32_1_PI;
  rec.mat_id = mat_id;
  return true;
}
