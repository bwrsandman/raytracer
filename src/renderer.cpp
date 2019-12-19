#include "renderer.h"

#include "private_impl/renderers/renderer_gpu.h"
#include "private_impl/renderers/renderer_whitted.h"

using namespace Raytracer::Graphics;

std::unique_ptr<Renderer>
Renderer::create(Type type, SDL_Window* window)
{
  switch (type) {
    case Renderer::Type::Whitted:
      return std::make_unique<RendererWhitted>(window);
    case Renderer::Type::Gpu:
      return std::make_unique<RendererGpu>(window);
  }
  return nullptr;
}
