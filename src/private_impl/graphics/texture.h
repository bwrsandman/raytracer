#pragma once

#include <cstdint>

#include <memory>
#include <string>

namespace Raytracer::Graphics {
struct Texture
{
  enum class Format
  {
    rgb32f,
    rgba32f,
    rgba32u,
    rgba32i,
  };

  enum class MipMapFilter
  {
    linear,
    nearest,
  };

  static std::unique_ptr<Texture> create(uint32_t width,
                                         uint32_t height,
                                         MipMapFilter filter,
                                         Format format);
  virtual ~Texture();

  void set_debug_name(const std::string& name) const;
  void bind(uint32_t slot) const;
  void upload(const void* data, uint32_t size) const;

private:
  Texture(uint32_t native_texture,
          uint32_t native_sampler,
          uint32_t width,
          uint32_t height,
          Format format);

  const uint32_t native_texture;
  const uint32_t native_sampler;

  const uint32_t width;
  const uint32_t height;
  const Format format;

  friend struct Framebuffer;
};
} // namespace Raytracer::Graphics
