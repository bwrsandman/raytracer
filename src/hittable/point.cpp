#include "hittable/point.h"

Point::Point(vec3 pos, uint16_t m)
  : position(pos)
  , mat_id(m)
{}

bool
Point::hit(const Ray& r, float tmin, float tmax, hit_record& rec) const
{
  // In continuous space, the probability to hit a point is 0
  return false;
}
