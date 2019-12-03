#include "scene.h"
#include <cassert>
#include <sdf.h>

#include "camera.h"
#include "hittable/functional_geometry.h"
#include "hittable/line_segment.h"
#include "hittable/plane.h"
#include "hittable/point.h"
#include "hittable/sphere.h"
#include "hittable/triangle_mesh.h"
#include "materials/dielectric.h"
#include "materials/emissive_linear_drop_off.h"
#include "materials/emissive_quadratic_drop_off.h"
#include "materials/lambert.h"
#include "materials/metal.h"
#include "scene_node.h"
#include "texture.h"

std::unique_ptr<Scene>
Scene::load_cornel_box()
{
  std::vector<std::unique_ptr<Texture>> textures;
  std::vector<std::unique_ptr<Material>> materials;
  std::vector<uint32_t> light_indices;

  textures.emplace_back(Texture::load_from_file("earth_albedo.jpg"));     // 0
  textures.emplace_back(Texture::load_from_file("earth_normal_map.tga")); // 1

  materials.emplace_back(
    std::make_unique<Lambert>(vec3(1.0, 1.0, 1.0), 0));                   // 0
  materials.emplace_back(std::make_unique<Lambert>(vec3(0.6, 0.6, 0.6))); // 1
  materials.emplace_back(std::make_unique<Metal>(
    vec3(0.8, 0.6, 0.2), std::numeric_limits<uint16_t>::max(), 1));     // 2
  materials.emplace_back(std::make_unique<Metal>(vec3(0.8, 0.8, 0.8))); // 3
  materials.emplace_back(
    std::make_unique<EmissiveQuadraticDropOff>(vec3(0.8, 0, 0), 1.0f)); // 4
  materials.emplace_back(
    std::make_unique<EmissiveQuadraticDropOff>(vec3(0, 0, 0.8), 1.0f)); // 5
  materials.emplace_back(
    std::make_unique<EmissiveQuadraticDropOff>(vec3(2.0, 2.0, 2.0), 1.0f)); // 6
  materials.emplace_back(std::make_unique<Dielectric>(1.5f, 1.0f));         // 7
  materials.emplace_back(std::make_unique<Lambert>(vec3(1.0, 0.0, 0.0)));   // 8
  materials.emplace_back(std::make_unique<Lambert>(vec3(0.0, 0.5, 1.0)));   // 9

  std::vector<std::unique_ptr<Object>> list;
  list.emplace_back(std::make_unique<Sphere>(vec3(0, -0.5, -2), 0.5, 0));
  list.emplace_back(std::make_unique<Sphere>(vec3(0, -101.0, -2), 100, 1));
  auto complex_shape = [](const vec3& position) -> float {
    auto sphere = sdf::sphere(position, 0.6f);
    auto sphere2 = sdf::sphere(position - vec3(-0.5, 0.4, 0.4f), 0.3f);
    auto box = sdf::box(position, vec3(0.4f, 0.4f, 0.4f));
    auto sphere_box = sdf::intersect(sphere, box);
    float cylinder_length = 1.0f;
    float radius = 0.35f;
    auto cylinder_y = sdf::cylinder(position, radius, cylinder_length);
    auto cylinder_x = sdf::cylinder(
      vec3(position.z(), position.x(), position.y()), radius, cylinder_length);
    auto cylinder_z = sdf::cylinder(
      vec3(position.x(), position.z(), position.y()), radius, cylinder_length);
    auto cylinder_cross =
      sdf::combine(cylinder_x, sdf::combine(cylinder_y, cylinder_z));
    return sdf::difference(sphere_box, cylinder_cross);
  };
  list.emplace_back(std::make_unique<FunctionalGeometry>(
    vec3(1.5, -0.5, -2.1), 40, complex_shape, 3));

  list.emplace_back(std::make_unique<Sphere>(vec3(-1.5, -0.5, -2.1), 0.5, 7));
  list.emplace_back(std::make_unique<Sphere>(vec3(-1.5, -0.5, -2.1), -0.45, 7));

  /*std::vector<uint16_t> indices = {
    0, 1, 2, 2, 3, 0,
  };
  std::vector<MeshVertexData> data0 = {
    MeshVertexData{
      vec2{ 1, 0 },
      vec3{ 0, 1, 0 },
      vec3{ 1, 0, 0 },
    },
    MeshVertexData{
      vec2{ 0, 0 },
      vec3{ 0, 1, 0 },
      vec3{ 1, 0, 0 },
    },
    MeshVertexData{
      vec2{ 0, 1 },
      vec3{ 0, 1, 0 },
      vec3{ 1, 0, 0 },
    },
    MeshVertexData{
      vec2{ 1, 1 },
      vec3{ 0, 1, 0 },
      vec3{ 1, 0, 0 },
    },
  };
  std::vector<vec3> positions0 = {
    vec3{ 2.6f, -1.5f, -4.0f },
    vec3{ -2.6f, -1.5f, -4.0f },
    vec3{ -2.6f, 4.0f, -4.0f },
    vec3{ 2.6f, 4.0f, -4.0f },
  };
  std::vector<MeshVertexData> data1 = {
    MeshVertexData{
      vec2{ 1, 0 },
      vec3{ 1, 0, 0 },
      vec3{ 0, 0, -1 },
    },
    MeshVertexData{
      vec2{ 0, 0 },
      vec3{ 1, 0, 0 },
      vec3{ 0, 0, -1 },
    },
    MeshVertexData{
      vec2{ 0, 1 },
      vec3{ 1, 0, 0 },
      vec3{ 0, 0, -1 },
    },
    MeshVertexData{
      vec2{ 1, 1 },
      vec3{ 1, 0, 0 },
      vec3{ 0, 0, -1 },
    },
  };
  std::vector<vec3> positions1 = {
    vec3{ -2.5f, -1.5f, -4.0f },
    vec3{ -2.5f, -1.5f, 0.0f },
    vec3{ -2.5f, 4.0f, 0.0f },
    vec3{ -2.5f, 4.0f, -4.0f },
  };
  std::vector<MeshVertexData> data2 = {
    MeshVertexData{
      vec2{ 1, 0 },
      vec3{ -1, 0, 0 },
      vec3{ 0, 0, 1 },
    },
    MeshVertexData{
      vec2{ 0, 0 },
      vec3{ -1, 0, 0 },
      vec3{ 0, 0, 1 },
    },
    MeshVertexData{
      vec2{ 0, 1 },
      vec3{ -1, 0, 0 },
      vec3{ 0, 0, 1 },
    },
    MeshVertexData{
      vec2{ 1, 1 },
      vec3{ -1, 0, 0 },
      vec3{ 0, 0, 1 },
    },
  };
  std::vector<vec3> positions2 = {
    vec3{ 2.5f, -1.5f, 0.0f },
    vec3{ 2.5f, -1.5f, -4.0f },
    vec3{ 2.5f, 4.0f, -4.0f },
    vec3{ 2.5f, 4.0f, 0.0f },
  };

  list.emplace_back(
    std::make_unique<TriangleMesh>(std::move(positions0),
                                   std::move(data0),
                                   std::vector<uint16_t>(indices),
                                   1));
  list.emplace_back(
    std::make_unique<TriangleMesh>(std::move(positions1),
                                   std::move(data1),
                                   std::vector<uint16_t>(indices),
                                   8));
  list.emplace_back(
    std::make_unique<TriangleMesh>(std::move(positions2),
                                   std::move(data2),
                                   std::vector<uint16_t>(indices),
                                   9));*/
  /*
        vec3{ 2.6f, -1.5f, -4.0f },
    vec3{ -2.6f, -1.5f, -4.0f },
    vec3{ -2.6f, 4.0f, -4.0f },
    vec3{ 2.6f, 4.0f, -4.0f },

        vec3{ -2.5f, -1.5f, -4.0f },
    vec3{ -2.5f, -1.5f, 0.0f },
    vec3{ -2.5f, 4.0f, 0.0f },
    vec3{ -2.5f, 4.0f, -4.0f },

        vec3{ 2.5f, -1.5f, 0.0f },
    vec3{ 2.5f, -1.5f, -4.0f },
    vec3{ 2.5f, 4.0f, -4.0f },
    vec3{ 2.5f, 4.0f, 0.0f },
  */
  // list.emplace_back(std::make_unique<Plane_xy>(0, 555, 0, 555, 555, 8));
  // list.emplace_back(std::make_unique<Plane_yz>(-1.5, 4, -4, 0, 2.5, 8));
  // list.emplace_back(std::make_unique<Plane_yz>(-1.5, 4, -4, 0, -2.5, 8));
  // list.emplace_back(std::make_unique<Plane_xz>(213, 343, 227, 332, 554, 9));
  // list.emplace_back(std::make_unique<Plane_xz>(0, 555, 0, 555, 0, 8));

  list.emplace_back(std::make_unique<Plane>(vec3(-2.6f, -1.5f, -4.0f),
                                               vec3(2.6f, 4.0f, -4.0f),
                                               vec3(0.f, 0.f, 1.f),
                                               1));

  list.emplace_back(std::make_unique<Plane>(
    vec3(2.5f, -1.5f, -4.0f), vec3(2.5f, 4.0f, 0.0f), vec3(-1.f, 0.f, 0.f), 8));

  list.emplace_back(std::make_unique<Plane>(vec3(-2.5f, -1.5f, -4.0f),
                                               vec3(-2.5f, 4.0f, 0.0f),
                                               vec3(1.f, 0.f, 0.f),
                                               9));


  std::vector<std::unique_ptr<Object>> light_list;
  light_list.emplace_back(std::make_unique<Point>(vec3(1, 1.5, -2), 4));
  light_list.emplace_back(std::make_unique<Point>(vec3(-1, 1.5, -2), 5));
  light_list.emplace_back(std::make_unique<Point>(vec3(0, 2, -1.5), 6));

  // Move lights to world object list and keep their indices
  for (auto& light : light_list) {
    light_indices.emplace_back(list.size());
    list.emplace_back(std::move(light));
  }

  // Construct scene graph
  std::vector<SceneNode> nodes;
  // Root node, only parent in graph
  SceneNode& root_node = nodes.emplace_back();
  root_node.children_id_offset = nodes.size();
  // Camera and camera node
  SceneNode& camera_node = nodes.emplace_back();
  camera_node.camera = std::make_unique<Camera>(
    vec3(0, 0, 0), vec3(0, 0, -1), vec3(0, 1, 0), 90, 1);
  camera_node.type = SceneNode::Type::Camera;
  root_node.children_id_length++;
  // Meshes
  for (uint32_t i = 0; i < list.size(); ++i) {
    auto& node = nodes.emplace_back();
    node.type = SceneNode::Type::Mesh;
    node.mesh_id = i;
    root_node.children_id_length++;
  }

  return std::unique_ptr<Scene>(new Scene(std::move(nodes),
                                          1,
                                          std::move(textures),
                                          std::move(materials),
                                          std::move(list),
                                          std::move(light_indices)));
}

Scene::Scene(std::vector<SceneNode>&& nodes,
             uint32_t camera_index,
             std::vector<std::unique_ptr<Texture>>&& textures,
             std::vector<std::unique_ptr<Material>>&& materials,
             std::vector<std::unique_ptr<Object>>&& world_objects,
             std::vector<uint32_t>&& light_indices)
  : nodes(std::move(nodes))
  , camera_index(camera_index)
  , textures(std::move(textures))
  , materials(std::move(materials))
  , world_objects(std::move(world_objects))
  , light_indices(std::move(light_indices))
{}

Scene::~Scene() = default;

void
Scene::run(float width, float height)
{
  auto& camera = get_camera();
  camera.set_clean();
  camera.set_aspect(width / height);
}

Camera&
Scene::get_camera()
{
  assert(nodes[camera_index].type == SceneNode::Type::Camera);
  assert(nodes[camera_index].camera);
  return *nodes[camera_index].camera;
}

const Camera&
Scene::get_camera() const
{
  assert(nodes[camera_index].type == SceneNode::Type::Camera);
  assert(nodes[camera_index].camera);
  return *nodes[camera_index].camera;
}

const std::vector<std::unique_ptr<Object>>&
Scene::get_world() const
{
  return world_objects;
}

std::vector<std::unique_ptr<Object>>&
Scene::get_world()
{
  return world_objects;
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
