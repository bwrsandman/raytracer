#include "texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

std::unique_ptr<Texture>
Texture::load_from_file(const std::string& filename)
{
  int width, height, channels;
  float* data = stbi_loadf(filename.c_str(), &width, &height, &channels, 0);
  if (data == nullptr) {
    return nullptr;
  }
  return std::unique_ptr<Texture>(new Texture(width, height, channels, data));
}

Texture::Texture(uint32_t width,
                 uint32_t height,
                 uint8_t num_channels,
                 float* data)
  : width(width)
  , height(height)
  , num_channels(num_channels)
  , data(data)
{}

Texture::~Texture()
{
  if (data) {
    stbi_image_free(data);
  }
}

vec3
Texture::sample(const float (&texture_coordinates)[2]) const
{
  uint32_t x = std::clamp(texture_coordinates[0], 0.0f, 1.0f) * width;
  uint32_t y = std::clamp(texture_coordinates[1], 0.0f, 1.0f) * height;
  auto base = &data[(x + y * width) * num_channels];
  return vec3(base[0], base[1], base[2]);
}
