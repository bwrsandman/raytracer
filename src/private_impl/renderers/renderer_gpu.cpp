#include "renderer_gpu.h"

#include <SDL_video.h>
#include <cassert>
#include <glad/glad.h>

#include "math/vec3.h"
#include "math/vec4.h"

#include "../graphics/buffer.h"
#include "../graphics/framebuffer.h"
#include "../graphics/indexed_mesh.h"
#include "../graphics/texture.h"
#include "camera.h"
#include "hittable/plane.h"
#include "hittable/sphere.h"
#include "pipeline.h"
#include "scene.h"

#include "../../shaders/bridging_header.h"
#include "shaders/fullscreen_fs.h"
#include "shaders/gpu_1_raygen_fs.h"
#include "shaders/gpu_2a_scene_traversal_sphere_fs.h"
#include "shaders/gpu_2b_scene_traversal_plane_fs.h"
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
  , raygen_framebuffer_active(0)
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
  for (auto& framebuffer : raygen_framebuffer) {
    framebuffer->clear({ clear_color, clear_color, clear_color });
  }
  raygen_framebuffer[raygen_framebuffer_active]->bind();
  raygen_pipeline->bind();
  raygen_ray_camera->bind(RG_RAY_CAMERA_BINDING);
  fullscreen_quad->draw();
  glPopDebugGroup();
}

void
RendererGpu::encode_scene_traversal()
{
  constexpr vec4 clear_color = { 0.0f, 0.0f, 0.0f, 1.0f };
  // set t_max to float max
  static const vec4 previous_hit_record_clear = {
    std::numeric_limits<float>::max(), 0.0f, 0.0f, 1.0f
  };
  glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "scene traversal");

  // Clear the double buffer
  for (auto& framebuffer : scene_traversal_framebuffer) {
    framebuffer->clear({
      previous_hit_record_clear,
      clear_color,
      clear_color,
      clear_color,
      clear_color,
      clear_color,
    });
  }

  enum primitives_t
  {
    sphere,
    plane,

    count
  };

  constexpr std::string_view debug_strs[primitives_t::count] = {
    "sphere",
    "plane",
  };
  Pipeline* pipelines[primitives_t::count] = {
    scene_traversal_sphere_pipeline.get(),
    scene_traversal_plane_pipeline.get(),
  };
  Buffer* primitive_buffers[primitives_t::count] = {
    scene_traversal_spheres.get(),
    scene_traversal_planes.get(),
  };

  for (uint8_t i = 0; i < primitives_t::count; ++i) {
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, debug_strs[i].data());
    pipelines[i]->bind();
    {
      GLint st_ray_direction = glGetUniformLocation(
        pipelines[i]->get_native_handle(), "st_in_ray_direction");
      glUniform1i(st_ray_direction, ST_IN_RAY_DIRECTION_LOCATION);
      GLint st_previous_hit_record_0 = glGetUniformLocation(
        pipelines[i]->get_native_handle(), "st_in_previous_hit_record_0");
      glUniform1i(st_previous_hit_record_0,
                  ST_IN_PREVIOUS_HIT_RECORD_0_LOCATION);
    }
    raygen_textures[raygen_framebuffer_active][RG_OUT_RAY_ORIGIN_LOCATION]
      ->bind(ST_IN_RAY_ORIGIN_LOCATION);
    raygen_textures[raygen_framebuffer_active][RG_OUT_RAY_DIRECTION_LOCATION]
      ->bind(ST_IN_RAY_DIRECTION_LOCATION);
    scene_traversal_textures_ah_hit_record_0[scene_traversal_framebuffer_active]
      ->bind(ST_IN_PREVIOUS_HIT_RECORD_0_LOCATION);
    scene_traversal_framebuffer[1 - scene_traversal_framebuffer_active]->bind();
    primitive_buffers[i]->bind(ST_OBJECT_BINDING);
    fullscreen_quad->draw();
    scene_traversal_framebuffer_active = 1 - scene_traversal_framebuffer_active;
    glPopDebugGroup(); // debug_strs[i]
  }

  glPopDebugGroup(); // scene traversal
}

void
RendererGpu::encode_any_hit()
{
  constexpr std::string_view debug_strs[2] = {
    "closest hit",
    "miss all",
  };
  Pipeline* pipelines[2] = {
    closest_hit_pipeline.get(),
    miss_all_pipeline.get(),
  };
  for (uint8_t i = 0; i < 2; ++i) {
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, debug_strs[i].data());
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
      GLint ah_hit_record_4 = glGetUniformLocation(
        pipelines[i]->get_native_handle(), "ah_hit_record_4");
      glUniform1i(ah_hit_record_4, AH_HIT_RECORD_4_LOCATION);
      GLint ah_hit_record_5 = glGetUniformLocation(
        pipelines[i]->get_native_handle(), "ah_hit_record_5");
      glUniform1i(ah_hit_record_5, AH_HIT_RECORD_5_LOCATION);
      GLint ah_incident_ray_origin = glGetUniformLocation(
        pipelines[i]->get_native_handle(), "ah_incident_ray_origin");
      glUniform1i(ah_incident_ray_origin, AH_INCIDENT_RAY_ORIGIN_LOCATION);
      GLint ah_incident_ray_direction = glGetUniformLocation(
        pipelines[i]->get_native_handle(), "ah_incident_ray_direction");
      glUniform1i(ah_incident_ray_direction,
                  AH_INCIDENT_RAY_DIRECTION_LOCATION);
      GLint ah_out_energy_accumulation_direction = glGetUniformLocation(
        pipelines[i]->get_native_handle(), "ah_in_energy_accumulation");
      glUniform1i(ah_out_energy_accumulation_direction,
                  AH_IN_ENERGY_ACCUMULATION_LOCATION);
    }
    scene_traversal_textures_ah_hit_record_0[scene_traversal_framebuffer_active]
      ->bind(AH_HIT_RECORD_0_LOCATION);
    scene_traversal_textures[AH_HIT_RECORD_1_LOCATION - 1]->bind(
      AH_HIT_RECORD_1_LOCATION);
    scene_traversal_textures[AH_HIT_RECORD_2_LOCATION - 1]->bind(
      AH_HIT_RECORD_2_LOCATION);
    scene_traversal_textures[AH_HIT_RECORD_3_LOCATION - 1]->bind(
      AH_HIT_RECORD_3_LOCATION);
    scene_traversal_textures[AH_HIT_RECORD_4_LOCATION - 1]->bind(
      AH_HIT_RECORD_4_LOCATION);
    scene_traversal_textures[AH_HIT_RECORD_5_LOCATION - 1]->bind(
      AH_HIT_RECORD_5_LOCATION);
    raygen_textures[raygen_framebuffer_active][ST_IN_RAY_ORIGIN_LOCATION]->bind(
      AH_INCIDENT_RAY_ORIGIN_LOCATION);
    raygen_textures[raygen_framebuffer_active][ST_IN_RAY_DIRECTION_LOCATION]
      ->bind(AH_INCIDENT_RAY_DIRECTION_LOCATION);
    raygen_textures[raygen_framebuffer_active]
                   [ST_IN_PREVIOUS_HIT_RECORD_0_LOCATION]
                     ->bind(AH_IN_ENERGY_ACCUMULATION_LOCATION);
    anyhit_uniform->bind(AH_UNIFORM_BINDING);
    raygen_framebuffer[1 - raygen_framebuffer_active]->bind();
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
  raygen_textures[raygen_framebuffer_active]
                 [RG_OUT_ENERGY_ACCUMULATION_LOCATION]
                   ->bind(EA_IN_COLOR_LOCATION);
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
RendererGpu::upload_scene(const std::vector<std::unique_ptr<Object>>& objects)
{
  scene_traversal_sphere_uniform_t spheres;
  spheres.count = 0;
  scene_traversal_plane_uniform_t planes;
  planes.count = 0;
  for (const auto& obj : objects) {
    auto sphere = dynamic_cast<const Sphere*>(obj.get());
    auto plane = dynamic_cast<const Plane*>(obj.get());
    if (sphere != nullptr) {
      assert(spheres.count < MAX_NUM_SPHERES);
      sphere_t shader_sphere;
      shader_sphere.radius = sphere->radius;
      shader_sphere.center = vec4(
        sphere->center.e[0], sphere->center.e[1], sphere->center.e[2], 1.0f);
      shader_sphere.mat_id = sphere->mat_id;
      sphere_serialize(shader_sphere,
                       spheres.spheres[spheres.count],
                       spheres.materials[spheres.count]);
      spheres.count++;
    } else if (plane != nullptr) {
      assert(spheres.count < MAX_NUM_PLANES);
      plane_t shader_plane;
      shader_plane.min = plane->min;
      shader_plane.max = plane->max;
      shader_plane.normal = plane->n;
      shader_plane.mat_id = plane->mat_id;
      plane_serialize(shader_plane,
                      planes.min[planes.count],
                      planes.max[planes.count],
                      planes.normal[planes.count],
                      planes.materials[planes.count]);
      planes.count++;
    }
  }
  scene_traversal_spheres->upload(&spheres, sizeof(spheres));
  scene_traversal_planes->upload(&planes, sizeof(planes));
}

void
RendererGpu::upload_anyhit_uniforms()
{
  anyhit_uniform_data_t anyhit_uniform_data{
    frame_count,
    width,
  };
  anyhit_uniform->upload(&anyhit_uniform_data, sizeof(anyhit_uniform_data));
}

void
RendererGpu::upload_uniforms(const Scene& world)
{
  upload_camera_uniforms(world.get_camera());
  upload_scene(world.get_world());
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

    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "intersections");
    constexpr uint8_t max_recursion_depth = 1;
    for (uint8_t j = 0; j < max_recursion_depth; ++j) {
      auto debug_str = (std::string("pass #") + std::to_string(j));

      glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, debug_str.c_str());
      encode_scene_traversal();
      encode_any_hit();
      glPopDebugGroup(); // pass #

      // Swap buffers
      raygen_framebuffer_active = 1 - raygen_framebuffer_active;
    }
    glPopDebugGroup(); // intersections

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
  for (uint8_t i = 0; i < 2; ++i) {
    raygen_textures[i][RG_OUT_RAY_ORIGIN_LOCATION] = Texture::create(
      width, height, Texture::MipMapFilter::nearest, Texture::Format::rgba32f);
    raygen_textures[i][RG_OUT_RAY_ORIGIN_LOCATION]->set_debug_name(
      "ray origin " + std::to_string(i));
    raygen_textures[i][RG_OUT_RAY_DIRECTION_LOCATION] = Texture::create(
      width, height, Texture::MipMapFilter::nearest, Texture::Format::rgba32f);
    raygen_textures[i][RG_OUT_RAY_DIRECTION_LOCATION]->set_debug_name(
      "ray direction " + std::to_string(i));
    raygen_textures[i][RG_OUT_ENERGY_ACCUMULATION_LOCATION] = Texture::create(
      width, height, Texture::MipMapFilter::nearest, Texture::Format::rgba32f);
    raygen_textures[i][RG_OUT_ENERGY_ACCUMULATION_LOCATION]->set_debug_name(
      "energy accumulation " + std::to_string(i));
    raygen_framebuffer[i] =
      Framebuffer::create(raygen_textures[i].data(), raygen_textures[i].size());
  }
}

void
RendererGpu::rebuild_scene_traversal()
{
  std::unique_ptr<Texture> textures[6] = {
    // t (double buffered)
    Texture::create(
      width, height, Texture::MipMapFilter::nearest, Texture::Format::rgba32f),
    // position
    Texture::create(
      width, height, Texture::MipMapFilter::nearest, Texture::Format::rgba32f),
    // uv
    Texture::create(
      width, height, Texture::MipMapFilter::nearest, Texture::Format::rgba32f),
    // normal
    Texture::create(
      width, height, Texture::MipMapFilter::nearest, Texture::Format::rgba32f),
    // tangent
    Texture::create(
      width, height, Texture::MipMapFilter::nearest, Texture::Format::rgba32f),
    // status, mat_id, bvh_hits
    Texture::create(
      width, height, Texture::MipMapFilter::nearest, Texture::Format::rgba32i),
  };
  scene_traversal_framebuffer[0] =
    Framebuffer::create(textures, sizeof(textures) / sizeof(textures[0]));

  scene_traversal_textures_ah_hit_record_0[0] =
    std::move(textures[AH_HIT_RECORD_0_LOCATION]);
  scene_traversal_textures_ah_hit_record_0[0]->set_debug_name(
    "hit record (t, unused yzw) [0]");

  textures[AH_HIT_RECORD_0_LOCATION] = Texture::create(
    width, height, Texture::MipMapFilter::nearest, Texture::Format::rgba32f);

  scene_traversal_framebuffer[1] =
    Framebuffer::create(textures, sizeof(textures) / sizeof(textures[0]));

  scene_traversal_textures_ah_hit_record_0[1] =
    std::move(textures[AH_HIT_RECORD_0_LOCATION]);
  scene_traversal_textures_ah_hit_record_0[1]->set_debug_name(
    "hit record (t, unused yzw) [1]");

  scene_traversal_textures[AH_HIT_RECORD_1_LOCATION - 1] =
    std::move(textures[AH_HIT_RECORD_1_LOCATION]);
  scene_traversal_textures[AH_HIT_RECORD_1_LOCATION - 1]->set_debug_name(
    "hit record (position, unused w)");
  scene_traversal_textures[AH_HIT_RECORD_2_LOCATION - 1] =
    std::move(textures[AH_HIT_RECORD_2_LOCATION]);
  scene_traversal_textures[AH_HIT_RECORD_2_LOCATION - 1]->set_debug_name(
    "hit record (uv, unused zw)");
  scene_traversal_textures[AH_HIT_RECORD_3_LOCATION - 1] =
    std::move(textures[AH_HIT_RECORD_3_LOCATION]);
  scene_traversal_textures[AH_HIT_RECORD_3_LOCATION - 1]->set_debug_name(
    "hit record (normal, unused w)");
  scene_traversal_textures[AH_HIT_RECORD_4_LOCATION - 1] =
    std::move(textures[AH_HIT_RECORD_4_LOCATION]);
  scene_traversal_textures[AH_HIT_RECORD_4_LOCATION - 1]->set_debug_name(
    "hit record (tangent, unused w)");
  scene_traversal_textures[AH_HIT_RECORD_5_LOCATION - 1] =
    std::move(textures[AH_HIT_RECORD_5_LOCATION]);
  scene_traversal_textures[AH_HIT_RECORD_5_LOCATION - 1]->set_debug_name(
    "hit record (status, mat_id, bvh_hits)");
}

void
RendererGpu::rebuild_backbuffers()
{
  rebuild_raygen_buffers();
  rebuild_scene_traversal();

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

  // Scene Traversal (sphere)
  info.fragment_shader_binary = gpu_2a_scene_traversal_sphere_fs;
  info.fragment_shader_size = sizeof(gpu_2a_scene_traversal_sphere_fs) /
                              sizeof(gpu_2a_scene_traversal_sphere_fs[0]);
  info.fragment_shader_entry_point = "main";
  scene_traversal_sphere_pipeline =
    Pipeline::create(Pipeline::Type::RasterOpenGL, info);
  scene_traversal_spheres =
    Buffer::create(sizeof(scene_traversal_sphere_uniform_t));
  scene_traversal_spheres->set_debug_name("scene_traversal_spheres");

  // Scene Traversal (plane)
  info.fragment_shader_binary = gpu_2b_scene_traversal_plane_fs;
  info.fragment_shader_size = sizeof(gpu_2b_scene_traversal_plane_fs) /
                              sizeof(gpu_2b_scene_traversal_plane_fs[0]);
  info.fragment_shader_entry_point = "main";
  scene_traversal_plane_pipeline =
    Pipeline::create(Pipeline::Type::RasterOpenGL, info);
  scene_traversal_planes =
    Buffer::create(sizeof(scene_traversal_plane_uniform_t));
  scene_traversal_planes->set_debug_name("scene_traversal_planes");

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
