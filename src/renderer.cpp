#include "renderer.h"

#include "private_impl/renderers/renderer_whitted.h"

std::unique_ptr<Renderer>
Renderer::create(Type type, const Window& window)
{
  switch (type) {
    case Renderer::Type::Whitted:
      return std::make_unique<RendererWhitted>(window);
  }
}
