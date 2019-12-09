#include "renderer.h"

#include "private_impl/renderers/renderer_whitted.h"

std::unique_ptr<Renderer>
Renderer::create(Type type, SDL_Window* window)
{
  switch (type) {
    case Renderer::Type::Whitted:
      return std::make_unique<RendererWhitted>(window);
  }
}
