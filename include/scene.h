#pragma once

#include <memory>
#include <vector>

class Camera;
class Object;
class Material;

class Scene
{
public:
  virtual ~Scene();

  static std::unique_ptr<Scene> load_test_scene();

  void run(float width, float height);

  Camera& get_camera();
  const Camera& get_camera() const;
  const Object& get_world() const;
  Object& get_world();
  const Object& get_lights() const;
  Object& get_lights();
  const Material& get_material(uint16_t id) const;
  std::vector<std::unique_ptr<Material>>& get_material_list();

private:
  Scene(std::unique_ptr<Camera>&& camera,
        std::vector<std::unique_ptr<Material>>&& materials,
        std::unique_ptr<Object>&& world_objects,
        std::unique_ptr<Object>&& lights);

  std::unique_ptr<Camera> camera;
  std::vector<std::unique_ptr<Material>> materials;
  std::unique_ptr<Object> world_objects;
  std::unique_ptr<Object> lights;
};
