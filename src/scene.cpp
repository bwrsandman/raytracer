#include "scene.h"

#include "camera.h"
#include "hittable/line_segment.h"
#include "hittable/object_list.h"
#include "hittable/plane.h"
#include "hittable/point.h"
#include "hittable/sphere.h"
#include "materials/dielectric.h"
#include "materials/emissive_linear_drop_off.h"
#include "materials/lambert_scatter.h"
#include "materials/lambert_shadow_ray.h"
#include "materials/metal.h"

Scene::Scene()
  : camera(std::make_unique<Camera>(vec3(0, 0, 0),
                                    vec3(0, 0, -1),
                                    vec3(0, 1, 0),
                                    90,
                                    1))
{
  materials.emplace_back(
    std::make_unique<LambertShadowRay>(vec3(0.8, 0.3, 0.3)));
  materials.emplace_back(
    std::make_unique<LambertShadowRay>(vec3(0.8, 0.8, 0.8)));
  materials.emplace_back(std::make_unique<Metal>(vec3(0.8, 0.6, 0.2)));
  materials.emplace_back(std::make_unique<Metal>(vec3(0.8, 0.8, 0.8)));
  materials.emplace_back(
    std::make_unique<EmissiveLinearDropOff>(vec3(0.8, 0, 0), 0.01f));
  materials.emplace_back(
    std::make_unique<EmissiveLinearDropOff>(vec3(0, 0, 0.8), 0.01f));
  materials.emplace_back(
    std::make_unique<EmissiveLinearDropOff>(vec3(0.5, 0.5, 0.5), 0.01f));
  materials.emplace_back(std::make_unique<Dielectric>(1.5f));

  std::vector<std::unique_ptr<Object>> list;
  list.emplace_back(std::make_unique<Sphere>(vec3(0, 0, -1), 0.5, 0));
  list.emplace_back(std::make_unique<Sphere>(vec3(0, -100.5, -1), 100, 1));
  list.emplace_back(std::make_unique<Sphere>(vec3(1, 0, -1.1), 0.5, 2));
  list.emplace_back(std::make_unique<Sphere>(vec3(-1, 0, -1.1), 0.5, 7));
  list.emplace_back(std::make_unique<Sphere>(vec3(-1, 0, -1.1), -0.45, 7));
  //list.emplace_back(std::make_unique<Plane>(0.5, 1.5, -0.5, 0.5, -2, 2));
  world_objects = std::make_unique<ObjectList>(std::move(list));

  std::vector<std::unique_ptr<Object>> light_list;
  light_list.emplace_back(std::make_unique<Point>(vec3(100, 100, -1), 6));
  //light_list.emplace_back(std::make_unique<Point>(vec3(-100, 100, -1), 6));
  //vec3 line_segment[2] = { vec3(-10, 100, 0), vec3(10, 100, 0) };
  //light_list.emplace_back(std::make_unique<LineSegment>(line_segment, 6));
  lights = std::make_unique<ObjectList>(std::move(light_list));
}

Scene::~Scene() = default;

void
Scene::run(float width, float height)
{
  camera->set_clean();
  camera->set_aspect(width / height);
}

const Camera&
Scene::get_camera() const
{
  return *camera;
}

const Object&
Scene::get_world() const
{
  return *world_objects;
}

Object&
Scene::get_world()
{
  return *world_objects;
}

const Object&
Scene::get_lights() const
{
  return *lights;
}

Object&
Scene::get_lights()
{
  return *lights;
}

const Material&
Scene::get_material(uint16_t id) const
{
  return *materials[id];
}

std::vector<std::unique_ptr<Material>>&
Scene::get_material_list()
{
  return materials;
}
