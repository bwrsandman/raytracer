#include "hittable/line_segment.h"
#include <random>

#include "hit_record.h"
#include "ray.h"

LineSegment::LineSegment(const vec3 pos[2], uint16_t m)
  : position{ pos[0], pos[1] }
  , mat_id(m)
{}

bool
LineSegment::hit(const Ray& r, float tmin, float tmax, hit_record& rec) const
{
  vec3 db = position[1] - position[0];
  vec3 dc = position[0] - r.origin;

  // Lines are not coplanar
  if (std::abs(dot(dc, cross(r.direction, db))) >= 0.001f) {
    return false;
  }

  double s = dot(cross(dc, db), db) / cross(r.direction, db).squared_length();

  // Means we have an intersection
  if (s >= 0.0 && s <= 1.0) {
    vec3 intersection = lerp(position[0], position[1], s);

    rec.t = std::sqrt((r.origin - intersection).squared_length());
    rec.p = intersection;

    vec3 line_unit = position[1] - position[0];
    line_unit.make_unit_vector();
    vec3 perp = cross(line_unit, -r.direction);
    perp.make_unit_vector();

    rec.normal = cross(perp, line_unit);
    rec.normal.make_unit_vector();
    rec.mat_id = mat_id;
    return true;
  }

  return false;
}
