#include "texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

std::unique_ptr<Texture>
Texture::load_from_file(const std::string& filename)
{
  int width, height, channels;
  float* data = stbi_loadf(filename.c_str(), &width, &height, &channels, 0);
  if (data == nullptr) {
    std::printf(
      "Unable to load \"%s\" texture, make sure it is in the current path\n",
      filename.c_str());
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
Texture::sample(const vec3& texture_coordinates) const
{
  uint32_t x = static_cast<uint32_t>(texture_coordinates.e[0] * (width - 0.0001)) % width;
  uint32_t y =
      static_cast<uint32_t>((1.0f - std::fmod(texture_coordinates.e[1], 1.0f)) * (height - 0.0001)) % height;
  auto base = &data[(x + y * width) * num_channels];
  return vec3(base[0], base[1], base[2]);
}
