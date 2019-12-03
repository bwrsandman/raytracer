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
  std::vector<vec3> color;
  color.resize(width * height);
  for (uint32_t y = 0; y < height; ++y) {
    for (uint32_t x = 0; x < width; ++x) {
      color[x + y * width].e[0] = data[(x + y * width) * channels];
      if (channels > 2) {
        color[x + y * width].e[1] = data[(x + y * width) * channels + 1];
        color[x + y * width].e[2] = data[(x + y * width) * channels + 2];
      } else {

        color[x + y * width].e[1] = data[(x + y * width) * channels];
        color[x + y * width].e[2] = data[(x + y * width) * channels];
      }
    }
  }
  stbi_image_free(data);
  return std::unique_ptr<Texture>(new Texture(width, height, std::move(color)));
}

Texture::Texture(uint32_t width, uint32_t height, std::vector<vec3>&& data)
  : width(width)
  , height(height)
  , data(data)
{}

Texture::~Texture() = default;

const vec3&
Texture::sample(const vec3& texture_coordinates) const
{
  uint32_t x = static_cast<uint32_t>(texture_coordinates.e[0] * (width - 0.0001)) % width;
  uint32_t y =
      static_cast<uint32_t>((1.0f - std::fmod(texture_coordinates.e[1], 1.0f)) * (height - 0.0001)) % height;
  return data[(x + y * width)];
}
