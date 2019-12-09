#include "hittable/instance.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include "hit_record.h"
#include "ray.h"

using namespace Raytracer::Hittable;

Translate::Translate(Object* _p, vec3 _offset)
  : p(_p)
  , offset(_offset)
{}

bool
Translate::hit(const Ray& r,
               bool early_out,
               float t_min,
               float t_max,
               hit_record& rec) const
{
  Ray moved_r(r.origin - offset, r.direction);

  if (p->hit(moved_r, early_out, t_min, t_max, rec)) {
    rec.p += offset;
    return true;
  }

  return false;
}


Rotate_y::Rotate_y(Object* _p, float _angle)
  : p(_p)
  , sin_theta(0.f)
  , cos_theta(0.f)
{
  float radians = (M_PI / 180.f) * _angle;
  sin_theta = sin(radians);
  cos_theta = cos(radians);
}

bool
Rotate_y::hit(const Ray& r,
               bool early_out,
               float t_min,
               float t_max,
               hit_record& rec) const
{
  vec3 origin = r.origin;
  vec3 direction = r.direction;

  origin[0] = cos_theta * r.origin.x() - sin_theta * r.origin.z();
  origin[2] = sin_theta * r.origin.x() + cos_theta * r.origin.z();

  direction[0] = cos_theta * r.direction.x() - sin_theta * r.direction.z();
  direction[2] = sin_theta * r.direction.x() + cos_theta * r.direction.z();

  Ray rotated_r(origin, direction);

  if (p->hit(rotated_r, early_out, t_min, t_max, rec)) {
    vec3 p = rec.p;
    vec3 n = rec.normal;

	p[0] = cos_theta * rec.p.x() + sin_theta * rec.p.z();
    p[2] = -sin_theta * rec.p.x() + cos_theta * rec.p.z();

	n[0] = cos_theta * rec.normal.x() + sin_theta * rec.normal.z();
    n[2] = -sin_theta * rec.normal.x() + cos_theta * rec.normal.z();

	rec.p = p;
    rec.normal = n;

    return true;
  }

  return false;
}
