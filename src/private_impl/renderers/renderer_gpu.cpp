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
#include "shaders/gpu_2_scene_traversal_fs.h"
#include "shaders/gpu_3a_closest_hit_fs.h"
#include "shaders/gpu_3b_miss_all_fs.h"
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
  : frame_count(0)
  , width(0)
  , height(0)
  , scene_traversal_framebuffer_active(0)
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
RendererGpu::encode_scene_traversal()
{
  constexpr vec4 clear_color = { 0.0f, 0.0f, 0.0f, 1.0f };
  static const vec4 st_previous_hit_record_0_clear = {
    std::numeric_limits<float>::max(), 0.0f, 0.0f, 1.0f
  };
  glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "scene traversal");
  for (uint8_t i = 0; i < 2; ++i) {
    // set t_max to float max
    scene_traversal_framebuffer[i]->clear({
      st_previous_hit_record_0_clear,
      clear_color,
      clear_color,
      clear_color,
      clear_color,
      clear_color,
    });
  }
  scene_traversal_pipeline->bind();
  {
    GLint st_ray_direction = glGetUniformLocation(
      scene_traversal_pipeline->get_native_handle(), "st_ray_direction");
    glUniform1i(st_ray_direction, ST_RAY_DIRECTION_LOCATION);
    GLint st_previous_hit_record_0 =
      glGetUniformLocation(scene_traversal_pipeline->get_native_handle(),
                           "st_previous_hit_record_0");
    glUniform1i(st_previous_hit_record_0, ST_PREVIOUS_HIT_RECORD_0_LOCATION);
  }
  raygen_textures[RG_OUT_RAY_ORIGIN_LOCATION]->bind(ST_RAY_ORIGIN_LOCATION);
  raygen_textures[ST_RAY_DIRECTION_LOCATION]->bind(ST_RAY_DIRECTION_LOCATION);
  scene_traversal_textures_ah_hit_record_0[scene_traversal_framebuffer_active]
    ->bind(ST_PREVIOUS_HIT_RECORD_0_LOCATION);
  scene_traversal_framebuffer[1 - scene_traversal_framebuffer_active]->bind();
  fullscreen_quad->draw();

  scene_traversal_framebuffer_active = 1 - scene_traversal_framebuffer_active;
  glPopDebugGroup(); // scene_traversal_pipeline
}

void
RendererGpu::encode_any_hit()
{
  constexpr vec4 clear_color = { 0.0f, 0.0f, 0.0f, 1.0f };
  constexpr std::string_view debug_strs[2] = {
    "closest hit",
    "miss all",
  };
  Pipeline* pipelines[2] = {
    closest_hit_pipeline.get(),
    miss_all_pipeline.get(),
  };
  Framebuffer* framebuffers[2] = {
    closest_hit_framebuffer.get(),
    miss_all_framebuffer.get(),
  };
  for (uint8_t i = 0; i < 2; ++i) {
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, debug_strs[i].data());
    framebuffers[i]->clear({ clear_color });
    pipelines[i]->bind();
    {
      GLint ah_hit_record_0 = glGetUniformLocation(
        pipelines[i]->get_native_handle(), "ah_hit_record_0");
      glUniform1i(ah_hit_record_0, AH_HIT_RECORD_0_LOCATION);
      GLint ah_hit_record_1 = glGetUniformLocation(
        pipelines[i]->get_native_handle(), "ah_hit_record_1");
      glUniform1i(ah_hit_record_1, AH_HIT_RECORD_1_LOCATION);
      GLint ah_hit_record_2 = glGetUniformLocation(
        pipelines[i]->get_native_handle(), "ah_hit_record_2");
      glUniform1i(ah_hit_record_2, AH_HIT_RECORD_2_LOCATION);
      GLint ah_hit_record_3 = glGetUniformLocation(
        pipelines[i]->get_native_handle(), "ah_hit_record_3");
      glUniform1i(ah_hit_record_3, AH_HIT_RECORD_3_LOCATION);
      GLint ah_incident_ray_origin = glGetUniformLocation(
        pipelines[i]->get_native_handle(), "ah_incident_ray_origin");
      glUniform1i(ah_incident_ray_origin, AH_INCIDENT_RAY_ORIGIN_LOCATION);
      GLint ah_incident_ray_direction = glGetUniformLocation(
        pipelines[i]->get_native_handle(), "ah_incident_ray_direction");
      glUniform1i(ah_incident_ray_direction,
                  AH_INCIDENT_RAY_DIRECTION_LOCATION);
      GLint ma_color =
        glGetUniformLocation(pipelines[i]->get_native_handle(), "ma_color");
      glUniform1i(ma_color, MA_IN_COLOR_LOCATION);
    }
    scene_traversal_textures_ah_hit_record_0[scene_traversal_framebuffer_active]
      ->bind(AH_HIT_RECORD_0_LOCATION);
    scene_traversal_textures[AH_HIT_RECORD_1_LOCATION - 1]->bind(
      AH_HIT_RECORD_1_LOCATION);
    scene_traversal_textures[AH_HIT_RECORD_2_LOCATION - 1]->bind(
      AH_HIT_RECORD_2_LOCATION);
    scene_traversal_textures[AH_HIT_RECORD_3_LOCATION - 1]->bind(
      AH_HIT_RECORD_3_LOCATION);
    scene_traversal_textures[AH_INCIDENT_RAY_ORIGIN_LOCATION - 1]->bind(
      AH_INCIDENT_RAY_ORIGIN_LOCATION);
    scene_traversal_textures[AH_INCIDENT_RAY_DIRECTION_LOCATION - 1]->bind(
      AH_INCIDENT_RAY_DIRECTION_LOCATION);
    closest_hit_textures[AH_OUT_COLOR_LOCATION]->bind(MA_IN_COLOR_LOCATION);
    anyhit_uniform->bind(AH_UNIFORM_BINDING);
    framebuffers[i]->bind();
    fullscreen_quad->draw();
    glPopDebugGroup();
  }
}

void
RendererGpu::encode_final_blit()
{
  constexpr vec4 clear_color = { 1.0f, 1.0f, 0.0f, 1.0f };
  glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "final blit");
  backbuffer->clear({ clear_color });
  backbuffer->bind();
  final_blit_pipeline->bind();
  miss_all_textures[AH_OUT_COLOR_LOCATION]->bind(EA_COLOR_LOCATION);
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
RendererGpu::upload_anyhit_uniforms()
{
  anyhit_uniform_data_t anyhit_uniform_data{
    frame_count,
  };
  anyhit_uniform->upload(&anyhit_uniform_data, sizeof(anyhit_uniform_data));
}

void
RendererGpu::upload_uniforms(const Scene& world)
{
  upload_camera_uniforms(world.get_camera());
  upload_anyhit_uniforms();
}

void
RendererGpu::run(const Scene& world)
{
  if (world.get_camera().is_dirty()) {
    frame_count = 0;
  }

  if (context) {
    ++frame_count;

    glViewport(0, 0, width, height);

    upload_uniforms(world);
    encode_raygen();
    encode_scene_traversal();
    encode_any_hit();
    // TODO: Accumulate in framebuffer and sample it as second texture
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
  raygen_textures[RG_OUT_RAY_ORIGIN_LOCATION] = Texture::create(
    width, height, Texture::MipMapFilter::nearest, Texture::Format::rgba32f);
  raygen_textures[RG_OUT_RAY_ORIGIN_LOCATION]->set_debug_name("ray origin");
  raygen_textures[RG_OUT_RAY_DIRECTION_LOCATION] = Texture::create(
    width, height, Texture::MipMapFilter::nearest, Texture::Format::rgba32f);
  raygen_textures[RG_OUT_RAY_DIRECTION_LOCATION]->set_debug_name(
    "ray direction");
  raygen_framebuffer =
    Framebuffer::create(raygen_textures.data(), raygen_textures.size());
}

void
RendererGpu::rebuild_scene_traversal()
{
  std::unique_ptr<Texture> textures[6] = {
    Texture::create(
      width, height, Texture::MipMapFilter::nearest, Texture::Format::rgba32f),
    Texture::create(
      width, height, Texture::MipMapFilter::nearest, Texture::Format::rgba32f),
    Texture::create(
      width, height, Texture::MipMapFilter::nearest, Texture::Format::rgba32f),
    Texture::create(
      width, height, Texture::MipMapFilter::nearest, Texture::Format::rgba32i),
    Texture::create(
      width, height, Texture::MipMapFilter::nearest, Texture::Format::rgba32f),
    Texture::create(
      width, height, Texture::MipMapFilter::nearest, Texture::Format::rgba32f),
  };
  scene_traversal_framebuffer[0] =
    Framebuffer::create(textures, sizeof(textures) / sizeof(textures[0]));

  scene_traversal_textures_ah_hit_record_0[0] =
    std::move(textures[AH_HIT_RECORD_0_LOCATION]);
  scene_traversal_textures_ah_hit_record_0[0]->set_debug_name(
    "hit record (t, position) [0]");

  textures[AH_HIT_RECORD_0_LOCATION] = Texture::create(
    width, height, Texture::MipMapFilter::nearest, Texture::Format::rgba32f);

  scene_traversal_framebuffer[1] =
    Framebuffer::create(textures, sizeof(textures) / sizeof(textures[0]));

  scene_traversal_textures_ah_hit_record_0[1] =
    std::move(textures[AH_HIT_RECORD_0_LOCATION]);
  scene_traversal_textures_ah_hit_record_0[1]->set_debug_name(
    "hit record (t, position) [1]");

  scene_traversal_textures[AH_HIT_RECORD_1_LOCATION - 1] =
    std::move(textures[AH_HIT_RECORD_1_LOCATION]);
  scene_traversal_textures[AH_HIT_RECORD_1_LOCATION - 1]->set_debug_name(
    "hit record (normal, u)");
  scene_traversal_textures[AH_HIT_RECORD_2_LOCATION - 1] =
    std::move(textures[AH_HIT_RECORD_2_LOCATION]);
  scene_traversal_textures[AH_HIT_RECORD_2_LOCATION - 1]->set_debug_name(
    "hit record (tangent, v)");
  scene_traversal_textures[AH_HIT_RECORD_3_LOCATION - 1] =
    std::move(textures[AH_HIT_RECORD_3_LOCATION]);
  scene_traversal_textures[AH_HIT_RECORD_3_LOCATION - 1]->set_debug_name(
    "hit record (status, mat_id, bvh_hits)");
  scene_traversal_textures[AH_INCIDENT_RAY_ORIGIN_LOCATION - 1] =
    std::move(textures[AH_INCIDENT_RAY_ORIGIN_LOCATION]);
  scene_traversal_textures[AH_INCIDENT_RAY_ORIGIN_LOCATION - 1]->set_debug_name(
    "ray origin");
  scene_traversal_textures[AH_INCIDENT_RAY_DIRECTION_LOCATION - 1] =
    std::move(textures[AH_INCIDENT_RAY_DIRECTION_LOCATION]);
  scene_traversal_textures[AH_INCIDENT_RAY_DIRECTION_LOCATION - 1]
    ->set_debug_name("ray direction");
}

void
RendererGpu::rebuild_backbuffers()
{
  rebuild_raygen_buffers();
  rebuild_scene_traversal();
  {
    closest_hit_textures[AH_OUT_COLOR_LOCATION] =
      Texture::create(width,
                      height,
                      Texture::MipMapFilter::nearest,
                      Texture::Format::rgba32f); // TODO could be 8 bit colors
    closest_hit_framebuffer = Framebuffer::create(
      closest_hit_textures,
      sizeof(closest_hit_textures) / sizeof(closest_hit_textures[0]));
  }
  {
    miss_all_textures[AH_OUT_COLOR_LOCATION] =
      Texture::create(width,
                      height,
                      Texture::MipMapFilter::nearest,
                      Texture::Format::rgba32f); // TODO could be 8 bit colors
    miss_all_framebuffer = Framebuffer::create(miss_all_textures,
                                               sizeof(miss_all_textures) /
                                                 sizeof(miss_all_textures[0]));
  }

  frame_count = 0;
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
  raygen_ray_camera->set_debug_name("raygen_ray_camera");

  // Scene Traversal
  info.fragment_shader_binary = gpu_2_scene_traversal_fs;
  info.fragment_shader_size =
    sizeof(gpu_2_scene_traversal_fs) / sizeof(gpu_2_scene_traversal_fs[0]);
  info.fragment_shader_entry_point = "main";
  scene_traversal_pipeline =
    Pipeline::create(Pipeline::Type::RasterOpenGL, info);

  // TODO Any hit
  anyhit_uniform = Buffer::create(sizeof(anyhit_uniform_data_t));
  anyhit_uniform->set_debug_name("anyhit_uniform");

  // Closest Hit
  info.fragment_shader_binary = gpu_3a_closest_hit_fs;
  info.fragment_shader_size =
    sizeof(gpu_3a_closest_hit_fs) / sizeof(gpu_3a_closest_hit_fs[0]);
  info.fragment_shader_entry_point = "main";
  closest_hit_pipeline = Pipeline::create(Pipeline::Type::RasterOpenGL, info);

  // Miss All
  info.fragment_shader_binary = gpu_3b_miss_all_fs;
  info.fragment_shader_size =
    sizeof(gpu_3b_miss_all_fs) / sizeof(gpu_3b_miss_all_fs[0]);
  info.fragment_shader_entry_point = "main";
  miss_all_pipeline = Pipeline::create(Pipeline::Type::RasterOpenGL, info);

  // Accumulation
  info.fragment_shader_binary = fullscreen_fs;
  info.fragment_shader_size = sizeof(fullscreen_fs) / sizeof(fullscreen_fs[0]);
  info.fragment_shader_entry_point = "main";
  final_blit_pipeline = Pipeline::create(Pipeline::Type::RasterOpenGL, info);
}
