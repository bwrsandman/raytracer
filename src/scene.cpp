#include "scene.h"

#include "hittable/object_list.h"
#include "hittable/sphere.h"
#include "materials/lambert.h"
#include "materials/metal.h"

Scene::Scene()
{
  materials.emplace_back(std::make_unique<Lambertian>(vec3(0.8, 0.3, 0.3)));
  materials.emplace_back(std::make_unique<Lambertian>(vec3(0.8, 0.8, 0.0)));
  materials.emplace_back(std::make_unique<Metal>(vec3(0.8, 0.6, 0.2)));
  materials.emplace_back(std::make_unique<Metal>(vec3(0.8, 0.8, 0.8)));

  std::vector<std::unique_ptr<Object>> list;
  list.emplace_back(std::make_unique<Sphere>(vec3(0, 0, -1), 0.5, 0));
  list.emplace_back(std::make_unique<Sphere>(vec3(0, -100.5, -1), 100, 1));
  list.emplace_back(std::make_unique<Sphere>(vec3(1, 0, -1), 0.5, 2));
  list.emplace_back(std::make_unique<Sphere>(vec3(-1, 0, -1), 0.5, 3));
  world_objects = std::make_unique<ObjectList>(std::move(list));
}

const Object&
Scene::get_world() const
{
  return *world_objects;
}

const Material&
Scene::get_material(uint16_t id) const
{
  return *materials[id];
}

Scene::~Scene() = default;
