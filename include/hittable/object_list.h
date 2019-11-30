#pragma once

#include "object.h"

#include <memory>
#include <vector>

struct ObjectList : public Object
{
  explicit ObjectList(std::vector<std::unique_ptr<Object>>&& l);
  ~ObjectList() override;
  bool hit(const Ray& r,
           float tmin,
           float tmax,
           hit_record& rec) const override;

  std::vector<std::unique_ptr<Object>> list;
};
