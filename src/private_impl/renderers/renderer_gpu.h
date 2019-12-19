#pragma once

#include "renderer.h"

typedef void* SDL_GLContext;

namespace Raytracer::Graphics {
struct IndexedMesh;
struct Texture;
class RendererGpu : public Renderer
{
public:
  explicit RendererGpu(SDL_Window* window);
  ~RendererGpu() override;

  void run(const Scene& world) override;
  void set_backbuffer_size(uint16_t width, uint16_t height) override;

  bool get_debug() const override;
  void set_debug(bool value) override;
  void set_debug_data(uint32_t data) override;

private:
  void rebuild_backbuffers();

  SDL_GLContext context;
  uint16_t width;
  uint16_t height;
};
} // namespace Raytracer::Graphics
