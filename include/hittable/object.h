#pragma once

class Ray;
struct hit_record;

class Object
{
public:
  virtual bool hit(const Ray& r,
                   float t_min,
                   float t_max,
                   hit_record& rec) const = 0;
};
