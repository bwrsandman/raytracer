#include "hittable/plane.h"

#include "hit_record.h"
#include "ray.h"

//Plane::Plane(float _x0, float _x1, float _y0, float _y1, float _k, uint16_t _m)
//  : x0(_x0)
//  , x1(_x1)
//  , y0(_y0)
//  , y1(_y1)
//  , k(_k)
//  , mat_id(_m)
//{}


//bool
//Plane::hit(const Ray& r, float tmin, float tmax, hit_record& rec) const
//{
//  float t = (k - r.origin.z()) / r.direction.z();
//
//  if (t < tmin || t > tmax)
//    return false;
//
//  float x = r.origin.x() + t * r.direction.x();
//  float y = r.origin.y() + t * r.direction.y();
//
//  if (x < x0 || x > x1 || y < y0 || y > y1)
//    return false;
//
//  rec.t = t;
//  rec.p = r.point_at_parameter(rec.t);
//  rec.mat_id = mat_id;
//  rec.normal = vec3(0, 0, 1);
//  return true;
//
//}

Plane::Plane(float _p0, vec3 top_right, vec3 _norm, uint16_t _m)
  : p0(_p0)
  , norm(_norm)
  , mat_id(_m)
{}

bool
Plane::hit(const Ray& r, float tmin, float tmax, hit_record& rec) const
{
  ////vec3 ur = unit_vector(r);
  //float denom = dot(r.direction, norm);

  //// not perpendicular to plane
  //if (denom > 0.000001) {
  //  vec3 p0l0 = p0 - r.origin;
  //  t = dot(p0l0, norm);
  //}


  //float t = (k - r.origin.z()) / r.direction.z();

  //if (t < tmin || t > tmax)
  //  return false;

  //float x = r.origin.x() + t * r.direction.x();
  //float y = r.origin.y() + t * r.direction.y();

  //if (x < x0 || x > x1 || y < y0 || y > y1)
  //  return false;

  //rec.t = t;
  //rec.p = r.point_at_parameter(rec.t);
  //rec.mat_id = mat_id;
  //rec.normal = vec3(0, 0, 1);
  return true;
}
