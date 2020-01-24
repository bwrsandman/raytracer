#include "hittable/point.h"

using Raytracer::hit_record;
using Raytracer::Ray;
using Raytracer::Hittable::Object;
using Raytracer::Hittable::Point;
using Raytracer::Math::vec3;

Point::Point(vec3 pos, uint16_t m)
  : position(pos)
  , mat_id(m)
{}

bool
Point::hit([[maybe_unused]] const Ray& r,
           [[maybe_unused]] bool early_out,
           [[maybe_unused]] float t_min,
           [[maybe_unused]] float t_max,
           [[maybe_unused]] hit_record& rec) const
{
  // In continuous space, the probability to hit a point is 0
  return false;
}

uint16_t
Point::get_mat_id() const
{
  return mat_id;
}

std::unique_ptr<Object>
Point::copy() const
{
  return std::make_unique<Point>(position, mat_id);
}
