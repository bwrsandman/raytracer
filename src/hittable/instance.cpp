#include "hittable/instance.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include "hit_record.h"
#include "math/vec4.h"
#include "ray.h"

using Raytracer::hit_record;
using Raytracer::Ray;
using Raytracer::Hittable::Rotate;
using Raytracer::Hittable::Translate;
using Raytracer::Math::mat4;
using Raytracer::Math::quat;
using Raytracer::Math::vec3;
using Raytracer::Math::vec4;

Translate::Translate(std::unique_ptr<Object>&& _p, vec3 _offset)
  : p(std::move(_p))
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

Rotate::Rotate(std::unique_ptr<Object>&& _p, vec4 _axis)
  : p(std::move(_p))
  , local_rot()
  , total_rot(quat{ 1.f, 0.f, 0.f, 0.f })
  , axis(_axis)
{
  axis.normalize_special();

  local_rot = quat{ axis.x() * sin(axis.w() * 0.5f),
                    axis.y() * sin(axis.w() * 0.5f),
                    axis.z() * sin(axis.w() * 0.5f),
                    cos(axis.w() * 0.5f) };

  total_rot = local_rot * total_rot;

  rotation = (mat4)total_rot.normalize();
  inv_rotation = (mat4)total_rot.inverse();
}

bool
Rotate::hit(const Ray& r,
            bool early_out,
            float t_min,
            float t_max,
            hit_record& rec) const
{
  quat current_total_rot =
    local_rot * total_rot;

  mat4 current_rotation = (mat4)total_rot.normalize();
  mat4 current_inv_rotation = (mat4)total_rot.inverse();

  vec4 origin = { 1.f, r.origin.x(), r.origin.y(), r.origin.z() };
  vec4 direction = { 0.f, r.direction.x(), r.direction.y(), r.direction.z() };

  // inverse matrix
  origin = dot(current_inv_rotation, origin);
  direction = dot(current_inv_rotation, direction);

  Ray rotated_r((vec3)origin, (vec3)direction);

  if (p->hit(rotated_r, early_out, t_min, t_max, rec)) {

    vec4 point = { 1.f, rec.p.x(), rec.p.y(), rec.p.z() };
    vec4 normal = { 0.f, rec.normal.x(), rec.normal.y(), rec.normal.z() };

    point = dot(current_rotation, point);
    normal = dot(current_rotation, normal);

    rec.p = (vec3)point;
    rec.normal = (vec3)normal;
    return true;
  }
  return false;
}

void
Rotate::build_bvh()
{
  p->build_bvh();
}

