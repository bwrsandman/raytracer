#pragma once

#include <cstdint>

#include <memory>
#include <vector>

namespace Raytracer::Math {
struct vec4;
}

namespace Raytracer::Graphics {
struct Texture;

using Math::vec4;

struct Framebuffer
{
  static std::unique_ptr<Framebuffer> create(const std::unique_ptr<Texture>* textures, uint8_t size);
  static std::unique_ptr<Framebuffer> default_framebuffer();
  virtual ~Framebuffer();

  void bind() const;
  void clear(const std::vector<vec4>& color) const;

private:
  Framebuffer(uint32_t native_handle,
              uint8_t size);

  const uint32_t native_handle;
  [[maybe_unused]] const uint8_t size;
};
} // namespace Raytracer::Graphics
