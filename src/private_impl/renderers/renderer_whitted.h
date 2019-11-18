#pragma once

#include "renderer.h"
#include <vector>

typedef void* SDL_GLContext;
struct Pixel
{
  float r;
  float g;
  float b;
  float a;
};

class RendererWhitted : public Renderer
{
public:
  explicit RendererWhitted(const Window& window);
  virtual ~RendererWhitted() override;
  void run(std::chrono::microseconds dt) override;
  void set_backbuffer_size(uint16_t w, uint16_t h) override;

private:
  void rebuild_backbuffers();

  SDL_GLContext context;
  uint16_t width;
  uint16_t height;

  std::vector<Pixel> cpu_buffer;
  uint32_t gpu_buffer;
};
