#pragma once

#include "object.h"

#include <memory>
#include <vector>

class ObjectList : public Object
{
public:
  explicit ObjectList(std::vector<std::unique_ptr<Object>>&& l);
  ~ObjectList() override;
  bool hit(const Ray& r,
           float tmin,
           float tmax,
           hit_record& rec) const override;

private:
  std::vector<std::unique_ptr<Object>> list;
};
