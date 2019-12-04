#include "hittable/plane.h"

#include "hit_record.h"
#include "ray.h"

Plane::Plane(vec3 _min, vec3 _max, vec3 _n, uint16_t _m)
  : min(_min)
  , max(_max)
  , n(_n)
  , mat_id(_m)
{}

bool
Plane::hit(const Ray& r, float tmin, float tmax, hit_record& rec) const
{
  float t;
  int index_one = 0, index_two = 0;

  if (n.x() != 0.f) { // yz plane

    t = (min.x() - r.origin.x()) / r.direction.x();
    index_one = 1, index_two = 2;

  } else if (n.y() != 0.f) { // xz plane

    t = (min.y() - r.origin.y()) / r.direction.y();
    index_one = 0, index_two = 2;

  } else { // xy plane

    t = (min.z() - r.origin.z()) / r.direction.z();
    index_one = 0, index_two = 1;
  }

  float one = r.origin.e[index_one] + t * r.direction.e[index_one];
  float two = r.origin.e[index_two] + t * r.direction.e[index_two];

  if (t < tmin || t > tmax)
    return false;
  if (one < min.e[index_one] || one > max.e[index_one] ||
      two < min.e[index_two] || two > max.e[index_two])
    return false;

  rec.t = t;
  rec.uv.e[0] =
    (one - min.e[index_one]) / (max.e[index_one] - min.e[index_one]);
  rec.uv.e[1] =
    (two - min.e[index_two]) / (max.e[index_two] - min.e[index_two]);
  rec.mat_id = mat_id;
  rec.p = r.point_at_parameter(t);
  rec.normal = n;
  return true;
}
