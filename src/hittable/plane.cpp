#include "hittable/plane.h"

#include "hit_record.h"
#include "ray.h"

Plane_xy::Plane_xy(float _x0, float _x1, float _y0, float _y1, float _k, uint16_t _m)
  : x0(_x0)
  , x1(_x1)
  , y0(_y0)
  , y1(_y1)
  , k(_k)
  , mat_id(_m)
{}


bool
Plane_xy::hit(const Ray& r, float tmin, float tmax, hit_record& rec) const
{
  float t = (k - r.origin.z()) / r.direction.z();

  if (t < tmin || t > tmax)
    return false;

  float x = r.origin.x() + t * r.direction.x();
  float y = r.origin.y() + t * r.direction.y();

  if (x < x0 || x > x1 || y < y0 || y > y1)
    return false;

  rec.t = t;
  rec.p = r.point_at_parameter(rec.t);
  rec.mat_id = mat_id;
  rec.normal = vec3(0, 0, 1);
  return true;

}

Plane_yz::Plane_yz(float _y0,
                   float _y1,
                   float _z0,
                   float _z1,
                   float _k,
                   uint16_t _m)
  : y0(_y0)
  , y1(_y1)
  , z0(_z0)
  , z1(_z1)
  , k(_k)
  , mat_id(_m)
{}

bool
Plane_yz::hit(const Ray& r, float tmin, float tmax, hit_record& rec) const
{
  float t = (k - r.origin.z()) / r.direction.z();

  if (t < tmin || t > tmax)
    return false;

  float y = r.origin.y() + t * r.direction.y();
  float z = r.origin.z() + t * r.direction.z();

  if (y < y0 || y > y1 || z < z0 || z > z1)
    return false;

  rec.t = t;
  rec.p = r.point_at_parameter(rec.t);
  rec.mat_id = mat_id;
  rec.normal = vec3(0, 0, 1);
  return true;
}

