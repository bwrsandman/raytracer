#include "materials/emissive_quadratic_drop_off.h"
#include <ray.h>

#include "ray.h"

using Raytracer::Ray;
using Raytracer::RayPayload;
using Raytracer::Scene;
using Raytracer::Materials::EmissiveQuadraticDropOff;
using Raytracer::Math::vec3;

EmissiveQuadraticDropOff::EmissiveQuadraticDropOff(const vec3& a, float factor)
  : albedo(a)
  , drop_off_factor(factor)
{}

void
EmissiveQuadraticDropOff::fill_type_data(
  [[maybe_unused]] const Scene& scene,
  RayPayload& payload,
  [[maybe_unused]] const vec3& texture_coordinates) const
{
  payload.type = RayPayload::Type::Emissive;
  payload.emission =
    albedo / (payload.distance * payload.distance * drop_off_factor);
}
