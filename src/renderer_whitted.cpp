#include <renderer_whitted.h>

#include <SDL.h>
#include <SDL_video.h>

#include <glad/glad.h>

#include "window.h"

RendererWhitted::RendererWhitted(const Window& window)
  : Renderer(window)
  , width(0)
  , height(0)
{
  context = SDL_GL_CreateContext(window.get_native_handle());
  gladLoadGL();

  glGenTextures(1, &gpu_buffer);
}

RendererWhitted::~RendererWhitted()
{
  glDeleteTextures(1, &gpu_buffer);
}

void
RendererWhitted::run(std::chrono::microseconds dt)
{
  static const float clear_color[4] = { 1.0f, 1.0f, 0.0f, 1.0f };

  glTexSubImage2D(
    GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_FLOAT, cpu_buffer.data());

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glClearBufferfv(GL_COLOR, 0, clear_color);
}

void
RendererWhitted::set_backbuffer_size(uint16_t w, uint16_t h)
{
  if (w != width || h != height) {
    width = w;
    height = h;

    rebuild_backbuffers();
  }
}

void
RendererWhitted::rebuild_backbuffers()
{
  cpu_buffer.resize(width * height);

  glBindTexture(GL_TEXTURE_2D, gpu_buffer);
  glTexImage2D(
    GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, cpu_buffer.data());
}