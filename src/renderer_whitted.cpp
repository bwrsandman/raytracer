#include <renderer_whitted.h>

#include <SDL.h>
#include <SDL_video.h>

#include<glad/glad.h>

#include "window.h"

RendererWhitted::RendererWhitted(const Window& window)
  : Renderer(window)
{
  context = SDL_GL_CreateContext(window.get_native_handle());
  gladLoadGL();
}

void
RendererWhitted::run(std::chrono::microseconds dt)
{
  static const float clear_color[4] = { 1.0f, 1.0f, 0.0f, 1.0f };

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glClearBufferfv(GL_COLOR, 0, clear_color);
}
