#pragma once

#include "vec3.h"
class Ray;
class Material;

struct hit_record
{
  float t;
  vec3 p;
  vec3 normal;
  Material* mat_ptr;
};

class Object
{
public:
  virtual bool hit(const Ray& r,
                   float t_min,
                   float t_max,
                   hit_record& rec) const = 0;
};

class ObjectList : public Object
{
public:
  ObjectList() {}
  ObjectList(Object** l, int n)
  {
    list = l;
    list_size = n;
  }
  virtual bool hit(const Ray& r, float tmin, float tmax, hit_record& rec) const;
  Object** list;
  int list_size;
};

class Sphere : public Object
{
public:
  Sphere() {}
  Sphere(vec3 cen, float r, Material* m)
    : center(cen)
    , radius(r)
    , mat_ptr(m) {};
  virtual bool hit(const Ray& r, float tmin, float tmax, hit_record& rec) const;
  vec3 center;
  float radius;
  Material* mat_ptr;
};