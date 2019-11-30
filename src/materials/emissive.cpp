#include "materials/emissive.h"

#include "ray.h"

Emissive::Emissive(const vec3& a)
  : albedo(a)
{}

void
Emissive::fill_type_data(const Scene& scene,
                         RayPayload& payload,
                         const float (&texture_coordinates)[2]) const
{
  payload.type = RayPayload::Type::Emissive;
  payload.emission = albedo;
}
