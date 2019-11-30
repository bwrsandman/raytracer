#include "materials/metal.h"

#include "ray.h"

#include "hit_record.h"

Metal::Metal(const vec3& a)
  : albedo(a)
{}

void
Metal::fill_type_data(const Scene& scene,
                      RayPayload& payload,
                      float (&texture_coordinates)[2]) const
{
  payload.type = RayPayload::Type::Metal;
  payload.attenuation = albedo;
}
