#pragma once

#include <memory>
#include <vector>

#include "scene_node.h"
#include "hittable/object.h"
#include "material.h"

class Camera;
//class Object;
//class Material;
class Texture;

class Scene
{
public:
  virtual ~Scene();

  static std::unique_ptr<Scene> load_cornel_box();

  void run(float width, float height);

  Camera& get_camera();
  const Camera& get_camera() const;
  const std::vector<std::unique_ptr<Object>>& get_world() const;
  std::vector<std::unique_ptr<Object>>& get_world();
  const std::vector<uint32_t>& get_light_indices() const;
  std::vector<uint32_t>& get_light_indices();
  const Material& get_material(uint16_t id) const;
  const Texture& get_texture(uint16_t id) const;
  std::vector<std::unique_ptr<Material>>& get_material_list();

private:
  Scene(std::vector<SceneNode>&& nodes,
        uint32_t camera_index,
        std::vector<std::unique_ptr<Texture>>&& textures,
        std::vector<std::unique_ptr<Material>>&& materials,
        std::vector<std::unique_ptr<Object>>&& world_objects,
        std::vector<uint32_t>&& light_indices);

  std::vector<SceneNode> nodes;
  uint32_t camera_index;
  std::vector<std::unique_ptr<Material>> materials;
  std::vector<std::unique_ptr<Texture>> textures;
  std::vector<std::unique_ptr<Object>> world_objects;
  std::vector<uint32_t> light_indices;
};
