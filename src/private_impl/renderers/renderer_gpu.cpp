#include "renderer_gpu.h"

#include <SDL_video.h>
#include <glad/glad.h>

#include "math/vec4.h"

using namespace Raytracer::Graphics;
using namespace Raytracer::Math;

namespace {
void GLAPIENTRY
MessageCallback([[maybe_unused]] GLenum /*source*/,
                GLenum type,
                [[maybe_unused]] GLuint /*id*/,
                GLenum severity,
                [[maybe_unused]] GLsizei /*length*/,
                const GLchar* message,
                [[maybe_unused]] const void* /*userParam*/)
{
  if (type == GL_DEBUG_TYPE_OTHER ||
      type == GL_DEBUG_TYPE_PUSH_GROUP ||
      type == GL_DEBUG_TYPE_POP_GROUP) {
    return;
  }
  fprintf(stderr,
          "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
          (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
          type,
          severity,
          message);
}
} // namespace
RendererGpu::RendererGpu(SDL_Window* window)
{

  // Request opengl 3.2 context.
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
#if __EMSCRIPTEN__
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#else
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#endif

  // Turn on double buffering with a 24bit Z buffer.
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

  context = SDL_GL_CreateContext(window);
  if (context) {
    gladLoadGLES2Loader(SDL_GL_GetProcAddress);

#if !__EMSCRIPTEN__
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(MessageCallback, this);
#endif
  }
}

RendererGpu::~RendererGpu()
{
  SDL_GL_DeleteContext(context);
}

void
RendererGpu::run(const Raytracer::Scene& world)
{
  static const vec4 clear_color = { 1.0f, 1.0f, 0.0f, 1.0f };

  if (context) {
    glViewport(0, 0, width, height);

    // clearing screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearBufferfv(GL_COLOR, 0, clear_color.e);

    glFinish();
  }
}

void
RendererGpu::set_backbuffer_size(uint16_t w, uint16_t h)
{
  if (w != width || h != height) {
    width = w;
    height = h;

    rebuild_backbuffers();
  }
}

void
RendererGpu::rebuild_backbuffers()
{}

bool
RendererGpu::get_debug() const
{
  return false;
}

void
RendererGpu::set_debug([[maybe_unused]] bool /*value*/)
{}

void RendererGpu::set_debug_data([[maybe_unused]] uint32_t /*data*/) {}
