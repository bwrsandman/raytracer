#pragma once

#include <memory>
#include <vector>

class Camera;
class Object;
class Material;

class Scene
{
public:
  Scene();
  virtual ~Scene();

  void run(float width, float height);

  const Camera& get_camera() const;
  const Object& get_world() const;
  Object& get_world();
  const Object& get_lights() const;
  Object& get_lights();
  const Material& get_material(uint16_t id) const;
  std::vector<std::unique_ptr<Material>>& get_material_list();

private:
  std::unique_ptr<Camera> camera;
  std::vector<std::unique_ptr<Material>> materials;
  std::unique_ptr<Object> world_objects;
  std::unique_ptr<Object> lights;
};
