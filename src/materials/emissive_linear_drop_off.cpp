#include "materials/emissive_linear_drop_off.h"

#include "ray.h"

using Raytracer::Materials::EmissiveLinearDropOff;
using Raytracer::Math::vec3;
using Raytracer::RayPayload;
using Raytracer::Ray;

EmissiveLinearDropOff::EmissiveLinearDropOff(const vec3& a, float factor)
  : albedo(a)
  , drop_off_factor(factor)
{}

void
EmissiveLinearDropOff::fill_type_data(const Scene& scene,
                                      RayPayload& payload,
                                      const vec3& texture_coordinates) const
{
  payload.type = RayPayload::Type::Emissive;
  payload.emission = albedo / payload.distance * drop_off_factor;
}
