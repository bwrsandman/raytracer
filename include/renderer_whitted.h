#pragma once

#include "renderer.h"

typedef void* SDL_GLContext;

class RendererWhitted : public Renderer
{
public:
  explicit RendererWhitted(const Window& window);
  void run(std::chrono::microseconds dt) override;

private:
  SDL_GLContext context;
};
