#pragma once

struct Ray;
class Scene;
struct hit_record;

struct Object
{
  virtual ~Object() = default;
  virtual bool hit(const Ray& r,
                   float t_min,
                   float t_max,
                   hit_record& rec) const = 0;
};
