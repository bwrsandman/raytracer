#include "materials/lambert.h"

#include "hit_record.h"
#include "hittable/object_list.h"
#include "ray.h"
#include "scene.h"

Lambert::Lambert(const vec3& a)
  : albedo(a)
{}

void
Lambert::fill_type_data(const Scene& scene,
                        RayPayload& payload,
                        float (&texture_coordinates)[2]) const
{
  payload.type = RayPayload::Type::Lambert;
  payload.attenuation = albedo;
}
