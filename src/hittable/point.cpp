#include "hittable/point.h"

#include <cmath>

#include "hit_record.h"
#include "ray.h"

Point::Point(vec3 pos, uint16_t m)
  : position(pos)
  , mat_id(m)
{}

bool
Point::hit(const Ray& r, float tmin, float tmax, hit_record& rec) const
{
  vec3 ray_origin_to_point = position - r.origin;
  auto distance =
    std::sqrt(ray_origin_to_point.squared_length()); // FIXME: sqrt
  ray_origin_to_point /= distance;
  // If ray passes through point, then both ray direction and ray origin to
  // point will be parallel and their dot product will be 1
  if (dot(ray_origin_to_point, r.direction) > 0.999f) {
    rec.t = distance;
    rec.p = position;
    rec.normal = -r.direction;
    rec.mat_id = mat_id;
    return true;
  }
  return false;
}

vec3
Point::random_point() const
{
  return position;
}
