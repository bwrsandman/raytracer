#include "hittable/point.h"

using Raytracer::Hittable::Point;
using Raytracer::Math::vec3;
using Raytracer::hit_record;
using Raytracer::Ray;

Point::Point(vec3 pos, uint16_t m)
  : position(pos)
  , mat_id(m)
{}

bool
Point::hit(const Ray& r,
           bool early_out,
           float t_min,
           float t_max,
           hit_record& rec) const
{
  // In continuous space, the probability to hit a point is 0
  return false;
}
