#pragma once

struct Ray;
class Scene;
struct hit_record;

struct Object
{
  virtual ~Object() = default;
  virtual bool hit(const Ray& r,
                   bool early_out,
                   float t_min,
                   float t_max,
                   hit_record& rec) const = 0;
};
