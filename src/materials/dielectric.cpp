#include "materials/dielectric.h"
#include <ray.h>

#include "ray.h"

Dielectric::Dielectric(float ri, float ni)
  : ref_idx(ri)
  , ni(ni)
{}

void
Dielectric::fill_type_data(RayPayload& payload) const
{
  payload.type = RayPayload::Type::Dielectric;
  payload.dielectric.ni = ni;
  payload.dielectric.nt = ref_idx;
}
