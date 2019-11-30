#include "materials/dielectric.h"
#include <ray.h>

#include "ray.h"

Dielectric::Dielectric(float ri, float ni)
  : ref_idx(ri)
  , ni(ni)
{}

void
Dielectric::fill_type_data(const Scene& scene,
                           RayPayload& payload,
                           float (&texture_coordinates)[2]) const
{
  payload.type = RayPayload::Type::Dielectric;
  payload.dielectric.ni = ni;
  payload.dielectric.nt = ref_idx;
}
