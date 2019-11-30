#include "scene.h"

#include "camera.h"
#include "hittable/line_segment.h"
#include "hittable/object_list.h"
#include "hittable/plane.h"
#include "hittable/point.h"
#include "hittable/sphere.h"
#include "materials/dielectric.h"
#include "materials/emissive_linear_drop_off.h"
#include "materials/emissive_quadratic_drop_off.h"
#include "materials/lambert.h"
#include "materials/metal.h"
#include "texture.h"

std::unique_ptr<Scene>
Scene::load_test_scene()
{
  std::vector<std::unique_ptr<Texture>> textures;
  std::vector<std::unique_ptr<Material>> materials;
  std::vector<uint32_t> light_indices;

  auto camera = std::make_unique<Camera>(
    vec3(0, 0, 0), vec3(0, 0, -1), vec3(0, 1, 0), 90, 1);

  textures.emplace_back(Texture::load_from_file("earth_albedo.jpg"));     // 0
  textures.emplace_back(Texture::load_from_file("earth_normal_map.tga")); // 1

  materials.emplace_back(std::make_unique<Lambert>(vec3(1.0, 1.0, 1.0), 0));
  materials.emplace_back(std::make_unique<Lambert>(vec3(0.6, 0.6, 0.6)));
  materials.emplace_back(std::make_unique<Metal>(
    vec3(0.8, 0.6, 0.2), std::numeric_limits<uint16_t>::max(), 1));
  materials.emplace_back(std::make_unique<Metal>(vec3(0.8, 0.8, 0.8)));
  materials.emplace_back(
    std::make_unique<EmissiveQuadraticDropOff>(vec3(8, 0, 0), 1.0f));
  materials.emplace_back(
    std::make_unique<EmissiveQuadraticDropOff>(vec3(0, 0, 8), 1.0f));
  materials.emplace_back(std::make_unique<EmissiveQuadraticDropOff>(
    vec3(10000.0, 10000.0, 10000.0), 1.0f));
  materials.emplace_back(std::make_unique<Dielectric>(1.5f, 1.0f));

  std::vector<std::unique_ptr<Object>> list;
  list.emplace_back(std::make_unique<Sphere>(vec3(0, 0, -1), 0.5, 0));
  list.emplace_back(std::make_unique<Sphere>(vec3(0, -100.5, -1), 100, 1));
  list.emplace_back(std::make_unique<Sphere>(vec3(1, 0, -1.1), 0.5, 2));
  list.emplace_back(std::make_unique<Sphere>(vec3(-1, 0, -1.1), 0.5, 7));
  list.emplace_back(std::make_unique<Sphere>(vec3(-1, 0, -1.1), -0.45, 7));

  std::vector<std::unique_ptr<Object>> light_list;
  light_list.emplace_back(std::make_unique<Point>(vec3(100, 100, -1), 6));
  light_list.emplace_back(std::make_unique<Point>(vec3(-100, 100, -1), 6));

  // Move lights to world object list and keep their indices
  for (auto& light : light_list) {
    light_indices.emplace_back(list.size());
    list.emplace_back(std::move(light));
  }

  return std::unique_ptr<Scene>(
    new Scene(std::move(camera),
              std::move(textures),
              std::move(materials),
              std::make_unique<ObjectList>(std::move(list)),
              std::move(light_indices)));
}

Scene::Scene(std::unique_ptr<Camera>&& camera,
             std::vector<std::unique_ptr<Texture>>&& textures,
             std::vector<std::unique_ptr<Material>>&& materials,
             std::unique_ptr<Object>&& world_objects,
             std::vector<uint32_t>&& light_indices)
  : camera(std::move(camera))
  , textures(std::move(textures))
  , materials(std::move(materials))
  , world_objects(std::move(world_objects))
  , light_indices(std::move(light_indices))
{}

Scene::~Scene() = default;

void
Scene::run(float width, float height)
{
  camera->set_clean();
  camera->set_aspect(width / height);
}

Camera&
Scene::get_camera()
{
  return *camera;
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

const std::vector<uint32_t>&
Scene::get_light_indices() const
{
  return light_indices;
}

std::vector<uint32_t>&
Scene::get_light_indices()
{
  return light_indices;
}

const Texture&
Scene::get_texture(uint16_t id) const
{
  return *textures[id];
}
