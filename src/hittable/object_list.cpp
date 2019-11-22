#include "hittable/object_list.h"

#include "hit_record.h"
#include "ray.h"

ObjectList::ObjectList(std::vector<std::unique_ptr<Object>>&& l)
  : list(std::move(l))
{
}

ObjectList::~ObjectList() = default;

bool
ObjectList::hit(const Ray& r, float t_min, float t_max, hit_record& rec) const
{
  hit_record temp_rec;
  bool hit_anything = false;
  double closest_so_far = t_max;
  for (auto& object : list) {
    if (object->hit(r, t_min, closest_so_far, temp_rec)) {
      hit_anything = true;
      closest_so_far = temp_rec.t;
      rec = temp_rec;
    }
  }
  return hit_anything;
}
