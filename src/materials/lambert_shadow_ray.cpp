#include "materials/lambert_shadow_ray.h"

#include "hit_record.h"
#include "hittable/object_list.h"
#include "ray.h"
#include "scene.h"

LambertShadowRay::LambertShadowRay(const vec3& a)
  : albedo(a)
{}

bool
LambertShadowRay::scatter(const Scene& scene,
                          const Ray& r_in,
                          const hit_record& rec,
                          vec3& attenuation,
                          Ray (&scattered)[2]) const
{
  vec3 target = scene.get_lights().random_point();
  auto direction = target - rec.p;
  direction.make_unit_vector();
  auto shadow_ray = Ray(rec.p, direction);
  attenuation = vec3();
  hit_record shadow_rec{};
  float distance = (target - shadow_ray.origin).length(); // FIXME: a sqrt here
  if (scene.get_world().hit(shadow_ray, 0.001, distance, shadow_rec)) {
    return false;
  } else {// doing extra work?
    if (scene.get_lights().hit(
          shadow_ray, 0.001f, std::numeric_limits<float>::max(), shadow_rec)) {
      vec3 light_emission;
      scene.get_material(shadow_rec.mat_id)
        .scatter(scene, shadow_ray, shadow_rec, light_emission, scattered);
      auto cos = dot(rec.normal, -shadow_rec.normal);
      if (cos > 0.001f) {
        attenuation = albedo * light_emission * cos;
      }
    }
  }
  return false;
}
