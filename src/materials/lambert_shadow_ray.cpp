#include "materials/lambert_shadow_ray.h"

#include "hit_record.h"
#include "hittable/object_list.h"
#include "ray.h"
#include "scene.h"

LambertShadowRay::LambertShadowRay(const vec3& a)
  : albedo(a)
{}

void
LambertShadowRay::fill_type_data(RayPayload& payload) const
{
  payload.type = RayPayload::Type::Lambert;
  payload.attenuation = albedo;
}
