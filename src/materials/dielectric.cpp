#include "materials/dielectric.h"

#include "ray.h"

using Raytracer::Ray;
using Raytracer::RayPayload;
using Raytracer::Scene;
using Raytracer::Materials::Dielectric;
using Raytracer::Math::vec3;

Dielectric::Dielectric(const vec3& a, float ri, float ni)
  : albedo(a)
  , ref_idx(ri)
  , ni(ni)
{}

void
Dielectric::fill_type_data(
  [[maybe_unused]] const Scene& scene,
  RayPayload& payload,
  [[maybe_unused]] const vec3& texture_coordinates) const
{
  payload.type = RayPayload::Type::Dielectric;
  payload.attenuation = albedo;
  payload.dielectric.ni = ni;
  payload.dielectric.nt = ref_idx;
}
