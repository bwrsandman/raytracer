#include "materials/emissive_quadratic_drop_off.h"
#include <ray.h>

#include "ray.h"

EmissiveQuadraticDropOff::EmissiveQuadraticDropOff(const vec3& a, float factor)
  : albedo(a)
  , drop_off_factor(factor)
{}

void
EmissiveQuadraticDropOff::fill_type_data(const Scene& scene,
                                         RayPayload& payload,
                                         const vec2& texture_coordinates) const
{
  payload.type = RayPayload::Type::Emissive;
  payload.emission =
    albedo / (payload.distance * payload.distance * drop_off_factor);
}
