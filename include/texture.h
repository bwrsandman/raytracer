#pragma once

#include <memory>
#include <vector>

#include "vec2.h"
#include "vec3.h"

namespace tinygltf {
struct Image;
}

class Texture
{
public:
  static std::unique_ptr<Texture> load_from_file(const std::string& filename);
  static std::unique_ptr<Texture> load_from_gltf_image(
    const tinygltf::Image& image);
  virtual ~Texture();

  const vec3& sample(const vec3& texture_coordinates) const;

private:
  Texture(uint32_t width, uint32_t height, std::vector<vec3>&& data);

  const uint32_t width;
  const uint32_t height;
  const std::vector<vec3> data;
};
