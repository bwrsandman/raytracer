#include "renderer_gpu.h"

#include <SDL_video.h>
#include <glad/glad.h>

#include "math/vec3.h"
#include "math/vec4.h"

#include "camera.h"
#include "scene.h"

#include "../graphics/buffer.h"
#include "../graphics/framebuffer.h"
#include "../graphics/indexed_mesh.h"
#include "../graphics/texture.h"
#include "pipeline.h"

#include "../../shaders/bridging_header.h"
#include "shaders/fullscreen_fs.h"
#include "shaders/gpu_1_raygen_fs.h"
#include "shaders/passthrough_vs.h"

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

    backbuffer = Framebuffer::default_framebuffer();
    fullscreen_quad = IndexedMesh::create_fullscreen_quad();
    create_pipelines();
  }
}

RendererGpu::~RendererGpu()
{
  SDL_GL_DeleteContext(context);
}

void
RendererGpu::encode_raygen()
{
  constexpr vec4 clear_color = { 0.0f, 0.0f, 0.0f, 0.0f };
  glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "raygen");
  raygen_framebuffer->clear({ clear_color, clear_color });
  raygen_framebuffer->bind();
  raygen_pipeline->bind();
  raygen_ray_camera->bind(RG_RAY_CAMERA_BINDING);
  fullscreen_quad->draw();
  glPopDebugGroup();
}

void
RendererGpu::encode_final_blit()
{
  constexpr vec4 clear_color = { 1.0f, 1.0f, 0.0f, 1.0f };
  glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "final blit");
  backbuffer->clear({ clear_color });
  backbuffer->bind();
  final_blit_pipeline->bind();
  raygen_textures[RG_OUT_RAY_ORIGIN_LOCATION]->bind(0);
  fullscreen_quad->draw();
  glPopDebugGroup();
}

void
RendererGpu::upload_camera_uniforms(const Camera& camera)
{
  camera_uniform_t camera_uniform{
    camera.origin,
    camera.lower_left_corner,
    camera.horizontal,
    camera.vertical,
  };
  raygen_ray_camera->upload(&camera_uniform, sizeof(camera_uniform));
}

void
RendererGpu::upload_uniforms(const Scene& world)
{
  upload_camera_uniforms(world.get_camera());
}

void
RendererGpu::run(const Scene& world)
{
  if (context) {
    glViewport(0, 0, width, height);

    upload_uniforms(world);
    encode_raygen();
    encode_final_blit();

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
RendererGpu::rebuild_raygen_buffers()
{
  raygen_textures[RG_OUT_RAY_ORIGIN_LOCATION] =
    Texture::create(width, height, Texture::Format::rgba32f);
  raygen_textures[RG_OUT_RAY_ORIGIN_LOCATION]->set_debug_name("ray origin");
  raygen_textures[RG_OUT_RAY_DIRECTION_LOCATION] =
    Texture::create(width, height, Texture::Format::rgba32f);
  raygen_textures[RG_OUT_RAY_DIRECTION_LOCATION]->set_debug_name(
    "ray direction");
  raygen_framebuffer =
    Framebuffer::create(raygen_textures.data(), raygen_textures.size());
}

void
RendererGpu::rebuild_backbuffers()
{
  rebuild_raygen_buffers();
}

bool
RendererGpu::get_debug() const
{
  return false;
}

void
RendererGpu::set_debug([[maybe_unused]] bool /*value*/)
{}

void RendererGpu::set_debug_data([[maybe_unused]] uint32_t /*data*/) {}

void
RendererGpu::create_pipelines()
{
  // Common values
  PipelineCreateInfo info;
  info.vertex_shader_binary = passthrough_vs;
  info.vertex_shader_size = sizeof(passthrough_vs) / sizeof(passthrough_vs[0]);
  info.vertex_shader_entry_point = "main";

  // Raygen
  info.fragment_shader_binary = gpu_1_raygen_fs;
  info.fragment_shader_size =
    sizeof(gpu_1_raygen_fs) / sizeof(gpu_1_raygen_fs[0]);
  info.fragment_shader_entry_point = "main";
  raygen_pipeline = Pipeline::create(Pipeline::Type::RasterOpenGL, info);
  raygen_ray_camera = Buffer::create(sizeof(camera_uniform_t));

  // Accumulation
  info.fragment_shader_binary = fullscreen_fs;
  info.fragment_shader_size = sizeof(fullscreen_fs) / sizeof(fullscreen_fs[0]);
  info.fragment_shader_entry_point = "main";
  final_blit_pipeline = Pipeline::create(Pipeline::Type::RasterOpenGL, info);
}
