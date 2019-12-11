#include "hittable/plane.h"

#include "hit_record.h"
#include "ray.h"

using Raytracer::Hittable::Plane;
using Raytracer::Hittable::AABB;
using Raytracer::Math::vec3;
using Raytracer::hit_record;
using Raytracer::Ray;

Plane::Plane(vec3 _min, vec3 _max, vec3 _n, uint16_t _m)
  : min(_min)
  , max(_max)
  , n(_n)
  , mat_id(_m)
  , aabb()
{
  bounding_box(aabb);
}

bool
Plane::hit(const Ray& r,
           bool early_out,
           float t_min,
           float t_max,
           hit_record& rec) const
{
  /*if (!aabb.hit(r, early_out, t_min, t_max, rec)) {
    return false;
  }*/

  int index_one = 0, index_two = 0, axis = 0;
  float t;

  if (n.x() != 0.f) { // yz plane

    index_one = 1;
    index_two = 2;
    axis = 0;

  } else if (n.y() != 0.f) { // xz plane

    index_one = 0;
    index_two = 2;
    axis = 1;

  } else { // xy plane

    index_one = 0;
    index_two = 1;
    axis = 2;
  }
  t = (min.e[axis] - r.origin.e[axis]) / r.direction.e[axis];

  float one = r.origin.e[index_one] + t * r.direction.e[index_one];
  float two = r.origin.e[index_two] + t * r.direction.e[index_two];

  if (t < t_min || t > t_max)
    return false;
  if (one < min.e[index_one] || one > max.e[index_one] ||
      two < min.e[index_two] || two > max.e[index_two])
    return false;

  vec3 tang(0.f, 0.f, 0.f);

  float u = (one - min.e[index_one]) / (max.e[index_one] - min.e[index_one]);
  float v = (two - min.e[index_two]) / (max.e[index_two] - min.e[index_two]);

  tang.e[index_one] = u;
  tang.e[index_two] = v;

  rec.uv = vec3(u, v, 0.0f);
  rec.uv.make_unit_vector();
  rec.tangent = tang;
  rec.tangent.make_unit_vector();

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

bool
Plane::bounding_box(AABB& box)
{
  box = AABB(min, max);
  return true;
}