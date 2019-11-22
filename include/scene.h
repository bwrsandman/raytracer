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
  const Object& get_lights() const;
  const Material& get_material(uint16_t id) const;

private:
  std::vector<std::unique_ptr<Material>> materials;
  std::unique_ptr<Object> world_objects;
  std::unique_ptr<Object> lights;
};
