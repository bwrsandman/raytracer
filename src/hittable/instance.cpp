#include "hittable/instance.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include "hit_record.h"
#include "ray.h"

using Raytracer::hit_record;
using Raytracer::Ray;
using Raytracer::Hittable::Object;
using Raytracer::Hittable::Rotate_y;
using Raytracer::Hittable::Translate;
using Raytracer::Math::vec3;

Translate::Translate(const Object* _p, vec3 _offset)
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

uint16_t
Translate::get_mat_id() const
{
  return std::numeric_limits<uint16_t>::max();
}

std::unique_ptr<Object>
Translate::copy() const
{
  return std::make_unique<Translate>(p, offset);
}

Rotate_y::Rotate_y(const Object* _p, float _angle)
  : p(_p)
  , angle(_angle)
  , sin_theta(0.f)
  , cos_theta(0.f)
{
  float radians = static_cast<float>(M_PI) / 180.f * _angle;
  sin_theta = std::sin(radians);
  cos_theta = std::cos(radians);
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
    vec3 point = rec.p;
    vec3 n = rec.normal;

    point[0] = cos_theta * rec.p.x() + sin_theta * rec.p.z();
    point[2] = -sin_theta * rec.p.x() + cos_theta * rec.p.z();

    n[0] = cos_theta * rec.normal.x() + sin_theta * rec.normal.z();
    n[2] = -sin_theta * rec.normal.x() + cos_theta * rec.normal.z();

    rec.p = point;
    rec.normal = n;

    return true;
  }

  return false;
}

uint16_t
Rotate_y::get_mat_id() const
{
  return std::numeric_limits<uint16_t>::max();
}

std::unique_ptr<Object>
Rotate_y::copy() const
{
  return std::make_unique<Rotate_y>(p, angle);
}
