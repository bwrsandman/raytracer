#include "materials/emissive.h"

#include "ray.h"

Emissive::Emissive(const vec3& a)
  : albedo(a)
{}

void
Emissive::fill_type_data(RayPayload& payload) const
{
  payload.type = RayPayload::Type::Emissive;
  payload.emission = albedo;
}
