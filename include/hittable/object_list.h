#pragma once

#include "object.h"

class ObjectList : public Object
{
public:
  ObjectList(Object** l, int n);
  bool hit(const Ray& r,
           float tmin,
           float tmax,
           hit_record& rec) const override;
  Object** list;
  int list_size;
};
