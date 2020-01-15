#include "materials/emissive.h"

#include "ray.h"

using Raytracer::Ray;
using Raytracer::RayPayload;
using Raytracer::Scene;
using Raytracer::Materials::Emissive;
using Raytracer::Math::vec3;

Emissive::Emissive(const vec3& a)
  : albedo(a)
{}

void
Emissive::fill_type_data([[maybe_unused]] const Scene& scene,
                         RayPayload& payload,
                         [[maybe_unused]] const vec3& texture_coordinates) const
{
  payload.type = RayPayload::Type::Emissive;
  payload.emission = albedo;
}
