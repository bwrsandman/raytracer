#pragma once

#include <vec3.h>

struct Ray;
class Scene;
struct hit_record;

class Object
{
public:
  virtual ~Object() = default;
  virtual bool hit(const Ray& r,
                   float t_min,
                   float t_max,
                   hit_record& rec) const = 0;
  virtual vec3 random_point() const = 0;
};
