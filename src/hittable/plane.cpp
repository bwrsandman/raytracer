#include "hittable/plane.h"

#include "hit_record.h"
#include "ray.h"

using Raytracer::Aabb;
using Raytracer::hit_record;
using Raytracer::Ray;
using Raytracer::Hittable::Object;
using Raytracer::Hittable::Plane;
using Raytracer::Math::vec2;
using Raytracer::Math::vec3;

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
           [[maybe_unused]] bool early_out,
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

  auto point = r.point_at_parameter(t);
  vec2 point2 = vec2(point[index_one], point[index_two]);
  vec2 min2 = vec2(min[index_one], min[index_two]);
  vec2 max2 = vec2(max[index_one], max[index_two]);

  if (t < t_min || t > t_max || point2.e[0] < min2.e[0] ||
      point2.e[0] > max2.e[0] || point2.e[1] < min2.e[1] ||
      point2.e[1] > max2.e[1]) {
    return false;
  }

  vec2 uv = (point2 - min2) / (max2 - min2);

  rec.tangent[axis] = 0.0f;
  rec.tangent[index_one] = uv.e[0];
  rec.tangent[index_two] = uv.e[1];
  rec.tangent.make_unit_vector();

  rec.t = t;
  rec.uv.e[0] =
    (point2.e[0] - min.e[index_one]) / (max.e[index_one] - min.e[index_one]);
  rec.uv.e[1] =
    (point2.e[1] - min.e[index_two]) / (max.e[index_two] - min.e[index_two]);
  rec.mat_id = mat_id;
  rec.p = point;
  rec.normal = n;
  return true;
}

bool
Plane::bounding_box(Aabb& box)
{
  box = Aabb{ min, max };
  return true;
}

uint16_t
Plane::get_mat_id() const
{
  return mat_id;
}

std::unique_ptr<Object>
Plane::copy() const
{
  return std::make_unique<Plane>(min, max, n, mat_id);
}
