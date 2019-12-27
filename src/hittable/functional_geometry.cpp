#include "hittable/functional_geometry.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include "hit_record.h"
#include "ray.h"
#include "sdf.h"

using Raytracer::Aabb;
using Raytracer::hit_record;
using Raytracer::Ray;
using Raytracer::Hittable::FunctionalGeometry;
using Raytracer::Math::vec3;

FunctionalGeometry::FunctionalGeometry(const vec3& center,
                                       uint8_t max_steps,
                                       signed_distance_function_t sdf,
                                       uint16_t m)
  : sdf(std::move(sdf))
  , max_steps(max_steps)
  , center(center)
  , mat_id(m)
  , aabb()
{
  bounding_box(aabb);
}

// From
// http://blog.hvidtfeldts.net/index.php/2011/09/distance-estimated-3d-fractals-v-the-mandelbulb-different-de-approximations/
std::unique_ptr<FunctionalGeometry>
FunctionalGeometry::mandrelbulb(const vec3& center,
                                uint8_t max_iterations,
                                float max_radius,
                                float power,
                                uint16_t m)
{
  auto signed_distance_function =
    [center, max_iterations, max_radius, power](const vec3& position) {
      vec3 z = position - center;
      float dr = 1.0;
      float radius = 0.0;
      for (uint8_t i = 0; i < max_iterations; i++) {
        radius = z.length();
        if (radius > max_radius)
          break;

        // convert to polar coordinates
        float theta = std::acos(z.y() / radius);
        float phi = std::atan2(z.z(), z.x());
        dr = std::pow(radius, power - 1.0f) * power * dr + 1.0f;

        // scale and rotate the point
        float zr = pow(radius, power);
        theta = theta * power;
        phi = phi * power;

        // convert back to cartesian coordinates
        z = zr * vec3(std::sin(theta) * std::cos(phi),
                      std::sin(phi) * std::sin(theta),
                      std::cos(theta));
        z += position;
      }
      return 0.5f * std::log(radius) * radius / dr;
    };
  return std::make_unique<FunctionalGeometry>(
    center, static_cast<uint8_t>(100), signed_distance_function, m);
}

bool
FunctionalGeometry::hit(const Ray& r,
                        [[maybe_unused]] bool early_out,
                        float t_min,
                        float t_max,
                        hit_record& rec) const
{
  if (!Aabb::hit(aabb, r, t_min, t_max)) {
    return false;
  }

  static constexpr float f32_1_PI = static_cast<float>(M_1_PI);
  static constexpr float f32_1_2PI = 0.5f * f32_1_PI;
  static constexpr float grad_step = 0.05f;

  Ray ray{ r.origin - center, r.direction };
  float t = t_min;
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
    if (t > t_max) {
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

bool
FunctionalGeometry::bounding_box(Aabb& box)
{
  vec3 min(0.f, 0.f, 0.f), max(1.f, 1.f, 1.f);

  box = Aabb{ min, max };
  return true;
}
