//#include "aabb.h"
//
//#include "vec3.h"
//
//AABB::AABB(const vec3& _min, const vec3& _max) {}
//AABB::~AABB() = default;
//
//bool
//AABB::hit(const Ray& r,
//          bool early_out,
//          float t_min,
//          float t_max,
//          hit_record& rec) const
//{
//  for (int axis = 0; axis < 3; axis++) {
//    float reciprocal = 1.f / r.direction.e[axis];
//    float t0 = (min.e[axis] - r.origin.e[axis]) / reciprocal;
//    float t1 = (max.e[axis] - r.origin.e[axis]) / reciprocal;
//
//    // If the ray enters from back to front (t0 is bigger than t1)
//    if (reciprocal < 0.f) {
//      std::swap(t0, t1);
//    }
//
//    tmin = t0 > tmin ? t0 : tmin;
//    tmax = t1 < tmax ? t1 : tmax;
//
//	if (tmax <= tmin) {
//      return false;
//    }
//  }
//  return true;
//}