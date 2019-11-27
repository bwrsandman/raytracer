#pragma once

#include <memory>
#include <vector>

class Object;
class Material;

class Scene
{
public:
  Scene();
  virtual ~Scene();

  const Object& get_world() const;
  Object& get_world();
  const Object& get_lights() const;
  Object& get_lights();
  const Material& get_material(uint16_t id) const;
  std::vector<std::unique_ptr<Material>>& get_material_list();

private:
  std::vector<std::unique_ptr<Material>> materials;
  std::unique_ptr<Object> world_objects;
  std::unique_ptr<Object> lights;
};
