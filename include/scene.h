#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "scene_node.h"

namespace Raytracer {

class Camera;
class Texture;
namespace Materials {
struct Material;
}
namespace Hittable {
struct Object;
}

using namespace Materials;
using namespace Hittable;

class Scene
{
public:
  virtual ~Scene();

  static std::unique_ptr<Scene> load_whitted_scene();
  static std::unique_ptr<Scene> load_cornel_box();
  static std::unique_ptr<Scene> load_from_gltf(const std::string& file_name);
  static std::unique_ptr<Scene> load_mandrelbulb();

  void run(float width, float height);

  Camera& get_camera();
  const Camera& get_camera() const;
  const std::vector<std::unique_ptr<Object>>& get_world() const;
  std::vector<std::unique_ptr<Object>>& get_world();
  const std::vector<std::unique_ptr<Object>>& get_lights() const;
  std::vector<std::unique_ptr<Object>>& get_lights();
  const Material& get_material(uint16_t id) const;
  const Texture& get_texture(uint16_t id) const;
  std::vector<std::unique_ptr<Material>>& get_material_list();
  const float min_attenuation_magnitude;
  const uint8_t max_secondary_rays;

private:
  Scene(std::vector<SceneNode>&& nodes,
        uint32_t camera_index,
        std::vector<std::unique_ptr<Texture>>&& textures,
        std::vector<std::unique_ptr<Material>>&& materials,
        std::vector<std::unique_ptr<Object>>&& world_objects,
        std::vector<std::unique_ptr<Object>>&& lights,
        float min_attenuation_magnitude,
        uint8_t max_secondary_rays);

  std::vector<SceneNode> nodes;
  uint32_t camera_index;
  std::vector<std::unique_ptr<Material>> materials;
  std::vector<std::unique_ptr<Texture>> textures;
  std::vector<std::unique_ptr<Object>> world_objects;
  std::vector<std::unique_ptr<Object>> lights;
};
} // namespace Raytracer
