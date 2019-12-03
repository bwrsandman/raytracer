#pragma once

#include <memory>

#include "vec2.h"
#include "vec3.h"

class Texture
{
public:
  static std::unique_ptr<Texture> load_from_file(const std::string& filename);
  virtual ~Texture();

  vec3 sample(const vec3& texture_coordinates) const;

private:
  Texture(uint32_t width, uint32_t height, uint8_t num_channels, float* data);

  const uint32_t width;
  const uint32_t height;
  const uint8_t num_channels;
  float* const data;
};
