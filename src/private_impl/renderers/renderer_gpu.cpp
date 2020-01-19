#include "renderer_gpu.h"

#include <SDL_video.h>
#include <cassert>
#include <glad/glad.h>

#include "math/vec3.h"
#include "math/vec4.h"

#include "../graphics/buffer.h"
#include "../graphics/framebuffer.h"
#include "../graphics/indexed_mesh.h"
#include "../graphics/scoped_debug_group.h"
#include "../graphics/texture.h"
#include "camera.h"
#include "hittable/plane.h"
#include "hittable/point.h"
#include "hittable/sphere.h"
#include "materials/dielectric.h"
#include "materials/emissive_quadratic_drop_off.h"
#include "materials/lambert.h"
#include "materials/metal.h"
#include "pipeline.h"
#include "scene.h"

#include "../../shaders/bridging_header.h"
#include "shaders/gpu_1_raygen_fs.h"
#include "shaders/gpu_2a_scene_traversal_sphere_fs.h"
#include "shaders/gpu_2b_scene_traversal_plane_fs.h"
#include "shaders/gpu_3a_closest_hit_fs.h"
#include "shaders/gpu_3b_miss_all_fs.h"
#include "shaders/gpu_3c_shadow_ray_light_hit_fs.h"
#include "shaders/gpu_4_energy_accumulation_fs.h"
#include "shaders/gpu_5_postprocess_fs.h"
#include "shaders/passthrough_vs.h"

#if __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif
#define GL_TIMESTAMP 0x8E28

using namespace Raytracer::Graphics;
using namespace Raytracer::Math;
using Raytracer::Camera;
using Raytracer::Scene;

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
  if (type == GL_DEBUG_TYPE_OTHER || type == GL_DEBUG_TYPE_PUSH_GROUP ||
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
  , accumulation_framebuffer_active(0)
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

  context.reset(SDL_GL_CreateContext(window));
  if (context) {
    gladLoadGLES2Loader(SDL_GL_GetProcAddress);

#if !__EMSCRIPTEN__
    typedef void(APIENTRYP PFNGLQUERYCOUNTERPROC)(GLuint id, GLenum target);
    glQueryCounter = reinterpret_cast<PFNGLQUERYCOUNTERPROC>(
      SDL_GL_GetProcAddress("glQueryCounter"));
    typedef void(APIENTRYP PFNGLGETQUERYOBJECTUI64VPROC)(
      GLuint id, GLenum pname, GLuint64 * params);
    glGetQueryObjectui64v = reinterpret_cast<PFNGLGETQUERYOBJECTUI64VPROC>(
      SDL_GL_GetProcAddress("glGetQueryObjectui64v"));

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(MessageCallback, this);

    glGenQueries(sizeof(timestamp_queries_t) / sizeof(uint32_t),
                 reinterpret_cast<uint32_t*>(&timestamp_queries));
#endif

    backbuffer = Framebuffer::default_framebuffer();
    fullscreen_quad = IndexedMesh::create_fullscreen_quad();
    create_pipelines();
  }
}

RendererGpu::~RendererGpu()
{
#if !__EMSCRIPTEN__
  glDeleteQueries(sizeof(timestamp_queries_t) / sizeof(uint32_t),
                  reinterpret_cast<uint32_t*>(&timestamp_queries));
#endif
}

void
RendererGpu::SDLDestroyer::operator()(void* ctx) const
{
  SDL_GL_DeleteContext(ctx);
}

void
RendererGpu::encode_raygen()
{
  constexpr vec4 clear_color = { 0.0f, 0.0f, 0.0f, 0.0f };
  auto debug_group = ScopedDebugGroup("raygen");
  for (auto& framebuffer : raygen_framebuffer) {
    framebuffer->clear(
      { clear_color, clear_color, clear_color, clear_color, clear_color });
  }
  raygen_framebuffer[raygen_framebuffer_active]->bind();
  raygen_pipeline->bind();
  raygen_ray_uniform->bind(RG_RAY_CAMERA_BINDING);
  fullscreen_quad->draw();
#if !__EMSCRIPTEN__
  glQueryCounter(timestamp_queries.raygen, GL_TIMESTAMP);
#endif
}

void
RendererGpu::encode_scene_traversal(Texture& ray_direction)
{
  constexpr vec4 clear_color = { 0.0f, 0.0f, 0.0f, 1.0f };
  auto debug_group = ScopedDebugGroup("scene traversal");
  // set t_max to float max
  static const vec4 previous_hit_record_clear = {
    std::numeric_limits<float>::max(), 0.0f, 0.0f, 1.0f
  };

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
    auto primitive_debug_group = ScopedDebugGroup(debug_strs[i]);
    pipelines[i]->bind();
    {
      GLint st_ray_direction = glGetUniformLocation(
        pipelines[i]->get_native_handle(), "st_in_ray_direction");
      glUniform1i(st_ray_direction, ST_IN_RAY_DIRECTION_LOCATION);
      GLint st_previous_hit_record_0 = glGetUniformLocation(
        pipelines[i]->get_native_handle(), "st_in_previous_hit_record_0");
      glUniform1i(st_previous_hit_record_0,
                  ST_IN_PREVIOUS_HIT_RECORD_0_LOCATION);
      GLint st_previous_hit_record_1 = glGetUniformLocation(
        pipelines[i]->get_native_handle(), "st_in_previous_hit_record_1");
      glUniform1i(st_previous_hit_record_1,
                  ST_IN_PREVIOUS_HIT_RECORD_1_LOCATION);
      GLint st_previous_hit_record_2 = glGetUniformLocation(
        pipelines[i]->get_native_handle(), "st_in_previous_hit_record_2");
      glUniform1i(st_previous_hit_record_2,
                  ST_IN_PREVIOUS_HIT_RECORD_2_LOCATION);
      GLint st_previous_hit_record_3 = glGetUniformLocation(
        pipelines[i]->get_native_handle(), "st_in_previous_hit_record_3");
      glUniform1i(st_previous_hit_record_3,
                  ST_IN_PREVIOUS_HIT_RECORD_3_LOCATION);
      GLint st_previous_hit_record_4 = glGetUniformLocation(
        pipelines[i]->get_native_handle(), "st_in_previous_hit_record_4");
      glUniform1i(st_previous_hit_record_4,
                  ST_IN_PREVIOUS_HIT_RECORD_4_LOCATION);
      GLint st_previous_hit_record_5 = glGetUniformLocation(
        pipelines[i]->get_native_handle(), "st_in_previous_hit_record_5");
      glUniform1i(st_previous_hit_record_5,
                  ST_IN_PREVIOUS_HIT_RECORD_5_LOCATION);
    }
    raygen_textures[raygen_framebuffer_active][RG_OUT_RAY_ORIGIN_LOCATION]
      ->bind(ST_IN_RAY_ORIGIN_LOCATION);
    ray_direction.bind(ST_IN_RAY_DIRECTION_LOCATION);
    scene_traversal_textures[scene_traversal_framebuffer_active][0]->bind(
      ST_IN_PREVIOUS_HIT_RECORD_0_LOCATION);
    scene_traversal_textures[scene_traversal_framebuffer_active][1]->bind(
      ST_IN_PREVIOUS_HIT_RECORD_1_LOCATION);
    scene_traversal_textures[scene_traversal_framebuffer_active][2]->bind(
      ST_IN_PREVIOUS_HIT_RECORD_2_LOCATION);
    scene_traversal_textures[scene_traversal_framebuffer_active][3]->bind(
      ST_IN_PREVIOUS_HIT_RECORD_3_LOCATION);
    scene_traversal_textures[scene_traversal_framebuffer_active][4]->bind(
      ST_IN_PREVIOUS_HIT_RECORD_4_LOCATION);
    scene_traversal_textures[scene_traversal_framebuffer_active][5]->bind(
      ST_IN_PREVIOUS_HIT_RECORD_5_LOCATION);
    scene_traversal_framebuffer[1 - scene_traversal_framebuffer_active]->bind();
    primitive_buffers[i]->bind(ST_OBJECT_BINDING);
    fullscreen_quad->draw();
    scene_traversal_framebuffer_active = 1 - scene_traversal_framebuffer_active;
  }
}

void
RendererGpu::encode_any_hit([[maybe_unused]] uint8_t recursion_count)
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
    auto debug_group = ScopedDebugGroup(debug_strs[i]);
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
      if (ah_incident_ray_origin > 0) {
        glUniform1i(ah_incident_ray_origin, AH_INCIDENT_RAY_ORIGIN_LOCATION);
      }
      GLint ah_incident_ray_direction = glGetUniformLocation(
        pipelines[i]->get_native_handle(), "ah_incident_ray_direction");
      if (ah_incident_ray_direction > 0) {
        glUniform1i(ah_incident_ray_direction,
                    AH_INCIDENT_RAY_DIRECTION_LOCATION);
      }
      GLint ah_out_energy_accumulation_direction = glGetUniformLocation(
        pipelines[i]->get_native_handle(), "ah_in_energy_accumulation");
      if (ah_out_energy_accumulation_direction > 0) {
        glUniform1i(ah_out_energy_accumulation_direction,
                    AH_IN_ENERGY_ACCUMULATION_LOCATION);
      }
    }
    scene_traversal_textures[scene_traversal_framebuffer_active]
                            [AH_HIT_RECORD_0_LOCATION]
                              ->bind(AH_HIT_RECORD_0_LOCATION);
    scene_traversal_textures[scene_traversal_framebuffer_active]
                            [AH_HIT_RECORD_1_LOCATION]
                              ->bind(AH_HIT_RECORD_1_LOCATION);
    scene_traversal_textures[scene_traversal_framebuffer_active]
                            [AH_HIT_RECORD_2_LOCATION]
                              ->bind(AH_HIT_RECORD_2_LOCATION);
    scene_traversal_textures[scene_traversal_framebuffer_active]
                            [AH_HIT_RECORD_3_LOCATION]
                              ->bind(AH_HIT_RECORD_3_LOCATION);
    scene_traversal_textures[scene_traversal_framebuffer_active]
                            [AH_HIT_RECORD_4_LOCATION]
                              ->bind(AH_HIT_RECORD_4_LOCATION);
    scene_traversal_textures[scene_traversal_framebuffer_active]
                            [AH_HIT_RECORD_5_LOCATION]
                              ->bind(AH_HIT_RECORD_5_LOCATION);
    raygen_textures[raygen_framebuffer_active][RG_OUT_RAY_ORIGIN_LOCATION]
      ->bind(AH_INCIDENT_RAY_ORIGIN_LOCATION);
    raygen_textures[raygen_framebuffer_active][RG_OUT_RAY_DIRECTION_LOCATION]
      ->bind(AH_INCIDENT_RAY_DIRECTION_LOCATION);
    raygen_textures[raygen_framebuffer_active]
                   [RG_OUT_ENERGY_ACCUMULATION_LOCATION]
                     ->bind(AH_IN_ENERGY_ACCUMULATION_LOCATION);
    anyhit_uniform->bind(AH_UNIFORM_BINDING);
    raygen_framebuffer[1 - raygen_framebuffer_active]->bind();
    fullscreen_quad->draw();
#if !__EMSCRIPTEN__
    glQueryCounter(each_intersection_queries[recursion_count].any_hit[i],
                   GL_TIMESTAMP);
#endif
  }
}

void
RendererGpu::encode_shadow_ray_light_hit()
{
  auto debug_group = ScopedDebugGroup("shadow ray light hit");
  shadow_ray_light_hit_pipeline->bind();
  {
    GLint sr_hit_record_5 = glGetUniformLocation(
      shadow_ray_light_hit_pipeline->get_native_handle(), "sr_hit_record_5");
    glUniform1i(sr_hit_record_5, SR_HIT_RECORD_5_LOCATION);
    GLint sr_incident_ray_origin =
      glGetUniformLocation(shadow_ray_light_hit_pipeline->get_native_handle(),
                           "sr_incident_ray_origin");
    glUniform1i(sr_incident_ray_origin, SR_INCIDENT_RAY_ORIGIN_LOCATION);
    GLint sr_incident_ray_direction =
      glGetUniformLocation(shadow_ray_light_hit_pipeline->get_native_handle(),
                           "sr_incident_ray_direction");
    glUniform1i(sr_incident_ray_direction, SR_INCIDENT_RAY_DIRECTION_LOCATION);
    GLint sr_next_ray_direction =
      glGetUniformLocation(shadow_ray_light_hit_pipeline->get_native_handle(),
                           "sr_next_ray_direction");
    glUniform1i(sr_next_ray_direction, SR_NEXT_RAY_DIRECTION_LOCATION);
    GLint sr_in_energy_accumulation =
      glGetUniformLocation(shadow_ray_light_hit_pipeline->get_native_handle(),
                           "sr_in_energy_accumulation");
    glUniform1i(sr_in_energy_accumulation, SR_IN_ENERGY_ACCUMULATION_LOCATION);
    GLint sr_in_data = glGetUniformLocation(
      shadow_ray_light_hit_pipeline->get_native_handle(), "sr_in_data");
    glUniform1i(sr_in_data, SR_IN_DATA_LOCATION);
  }
  scene_traversal_textures[scene_traversal_framebuffer_active]
                          [AH_HIT_RECORD_5_LOCATION]
                            ->bind(SR_HIT_RECORD_5_LOCATION);
  raygen_textures[raygen_framebuffer_active][RG_OUT_RAY_ORIGIN_LOCATION]->bind(
    SR_INCIDENT_RAY_ORIGIN_LOCATION);
  raygen_textures[raygen_framebuffer_active]
                 [RG_OUT_SHADOW_RAY_DIRECTION_LOCATION]
                   ->bind(SR_INCIDENT_RAY_DIRECTION_LOCATION);
  raygen_textures[raygen_framebuffer_active][RG_OUT_RAY_DIRECTION_LOCATION]
    ->bind(SR_NEXT_RAY_DIRECTION_LOCATION);
  raygen_textures[raygen_framebuffer_active]
                 [RG_OUT_ENERGY_ACCUMULATION_LOCATION]
                   ->bind(SR_IN_ENERGY_ACCUMULATION_LOCATION);
  raygen_textures[raygen_framebuffer_active][RG_OUT_SHADOW_RAY_DATA_LOCATION]
    ->bind(SR_IN_DATA_LOCATION);
  shadow_ray_light_hit_uniform->bind(SR_UNIFORM_BINDING);
  raygen_framebuffer[1 - raygen_framebuffer_active]->bind();
  fullscreen_quad->draw();
}

void
RendererGpu::encode_accumulation()
{
  constexpr vec4 clear_color = { 0.0f, 0.0f, 0.0f, 1.0f };
  auto debug_group = ScopedDebugGroup("accumulation");
  accumulation_framebuffer[accumulation_framebuffer_active]->clear(
    { clear_color });
  accumulation_pipeline->bind();
  GLint ea_in_previous_energy = glGetUniformLocation(
    accumulation_pipeline->get_native_handle(), "ea_in_previous_energy");
  glUniform1i(ea_in_previous_energy, EA_IN_PREVIOUS_ENERGY_LOCATION);
  accumulation_framebuffer[accumulation_framebuffer_active]->bind();
  raygen_textures[raygen_framebuffer_active]
                 [RG_OUT_ENERGY_ACCUMULATION_LOCATION]
                   ->bind(EA_IN_CURRENT_ENERGY_LOCATION);
  accumulation_texture[1 - accumulation_framebuffer_active]->bind(
    EA_IN_PREVIOUS_ENERGY_LOCATION);
  fullscreen_quad->draw();
#if !__EMSCRIPTEN__
  glQueryCounter(timestamp_queries.accumulation, GL_TIMESTAMP);
#endif

  accumulation_framebuffer_active = 1 - accumulation_framebuffer_active;
}

void
RendererGpu::encode_final_blit()
{
  constexpr vec4 clear_color = { 1.0f, 1.0f, 0.0f, 1.0f };
  auto debug_group = ScopedDebugGroup("final blit");
  backbuffer->clear({ clear_color });
  final_blit_pipeline->bind();
  accumulation_texture[raygen_framebuffer_active]->bind(
    F_IMAGE_SAMPLER_LOCATION);
  fullscreen_quad->draw();
  backbuffer->bind();
#if !__EMSCRIPTEN__
  glQueryCounter(timestamp_queries.final_blit, GL_TIMESTAMP);
#endif
}

void
RendererGpu::upload_raygen_uniforms(const Camera& camera)
{
  raygen_uniform_t uniform{
    {
      camera.origin,
      camera.lower_left_corner,
      camera.horizontal,
      camera.vertical,
      camera.u,
      camera.v,
      camera.apature / 2,
    },
    frame_count,
    width,
    height,
  };
  raygen_ray_uniform->upload(&uniform, sizeof(uniform));
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
RendererGpu::upload_anyhit_uniforms(const Scene& world)
{
  anyhit_uniform_data_t anyhit_uniform_data{
    {}, {}, 0, 0, frame_count, width,
  };
  shadow_ray_light_hit_uniform_data_t shadow_ray_light_hit_uniform_data{
    {},
  };
  for (const auto& mat : world.get_material_list()) {
    auto lambert = dynamic_cast<const Lambert*>(mat.get());
    auto metal = dynamic_cast<const Metal*>(mat.get());
    auto dielectric = dynamic_cast<const Dielectric*>(mat.get());
    assert(anyhit_uniform_data.material_count < MAX_NUM_MATERIALS);
    if (lambert) {
      anyhit_uniform_data.material_data[anyhit_uniform_data.material_count]
        .e[0] = lambert->albedo.e[0];
      anyhit_uniform_data.material_data[anyhit_uniform_data.material_count]
        .e[1] = lambert->albedo.e[1];
      anyhit_uniform_data.material_data[anyhit_uniform_data.material_count]
        .e[2] = lambert->albedo.e[2];
      anyhit_uniform_data.material_data[anyhit_uniform_data.material_count]
        .e[3] = MATERIAL_TYPE_LAMBERT;
    } else if (metal) {
      anyhit_uniform_data.material_data[anyhit_uniform_data.material_count]
        .e[0] = metal->albedo.e[0];
      anyhit_uniform_data.material_data[anyhit_uniform_data.material_count]
        .e[1] = metal->albedo.e[1];
      anyhit_uniform_data.material_data[anyhit_uniform_data.material_count]
        .e[2] = metal->albedo.e[2];
      anyhit_uniform_data.material_data[anyhit_uniform_data.material_count]
        .e[3] = MATERIAL_TYPE_METAL;
    } else if (dielectric) {
      anyhit_uniform_data.material_data[anyhit_uniform_data.material_count]
        .e[0] = dielectric->ref_idx;
      anyhit_uniform_data.material_data[anyhit_uniform_data.material_count]
        .e[1] = dielectric->ni;
      // Bitshift with floats; x*1.000.000 + y*1.000 + z
      vec3 albedo = dielectric->albedo;
      float floatshift =
        albedo.e[0] * 1000000 + albedo.e[1] * 1000 + albedo.e[2];

      anyhit_uniform_data.material_data[anyhit_uniform_data.material_count]
        .e[2] = floatshift;
      anyhit_uniform_data.material_data[anyhit_uniform_data.material_count]
        .e[3] = MATERIAL_TYPE_DIELECTRIC;
    } else {
      anyhit_uniform_data.material_data[anyhit_uniform_data.material_count]
        .e[3] = MATERIAL_TYPE_UNKNOWN;
    }
    anyhit_uniform_data.material_count++;
  }
  for (const auto& obj : world.get_lights()) {
    auto light = dynamic_cast<const Point*>(obj.get());
    if (light != nullptr) {
      assert(anyhit_uniform_data.light_count < MAX_NUM_LIGHTS);
      const auto& mat = world.get_material(light->mat_id);
      auto emissive = dynamic_cast<const EmissiveQuadraticDropOff*>(&mat);
      if (emissive != nullptr) {
        anyhit_uniform_data.light_position_data[anyhit_uniform_data.light_count]
          .e[0] = light->position.e[0];
        anyhit_uniform_data.light_position_data[anyhit_uniform_data.light_count]
          .e[1] = light->position.e[1];
        anyhit_uniform_data.light_position_data[anyhit_uniform_data.light_count]
          .e[2] = light->position.e[2];
        shadow_ray_light_hit_uniform_data
          .light_color_data[anyhit_uniform_data.light_count]
          .e[0] = emissive->albedo.e[0];
        shadow_ray_light_hit_uniform_data
          .light_color_data[anyhit_uniform_data.light_count]
          .e[1] = emissive->albedo.e[1];
        shadow_ray_light_hit_uniform_data
          .light_color_data[anyhit_uniform_data.light_count]
          .e[2] = emissive->albedo.e[2];
        anyhit_uniform_data.light_count++;
      }
    }
  }
  anyhit_uniform->upload(&anyhit_uniform_data, sizeof(anyhit_uniform_data));
  shadow_ray_light_hit_uniform->upload(
    &shadow_ray_light_hit_uniform_data,
    sizeof(shadow_ray_light_hit_uniform_data));
}

void
RendererGpu::upload_uniforms(const Scene& world)
{
  upload_raygen_uniforms(world.get_camera());
  upload_scene(world.get_world());
  upload_anyhit_uniforms(world);
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

#if !__EMSCRIPTEN__
    if (each_intersection_queries.size() != max_recursion_depth) {
      auto current_size = each_intersection_queries.size();
      int difference = max_recursion_depth - current_size;
      if (difference > 0) {
        each_intersection_queries.resize(max_recursion_depth);
        glGenQueries(
          difference *
            (sizeof(intersection_timestamp_queries_t) / sizeof(uint32_t)),
          reinterpret_cast<GLuint*>(&each_intersection_queries[current_size]));
      } else {
        glDeleteQueries(
          -difference *
            (sizeof(intersection_timestamp_queries_t) / sizeof(uint32_t)),
          reinterpret_cast<GLuint*>(
            &each_intersection_queries[max_recursion_depth]));
        each_intersection_queries.resize(max_recursion_depth);
      }
    }
    glQueryCounter(timestamp_queries.start, GL_TIMESTAMP);
#endif
    encode_raygen();

    {
      auto intersections_debug_group = ScopedDebugGroup("intersections");
      for (uint8_t j = 0; j < max_recursion_depth; ++j) {
        auto debug_str = (std::string("pass #") + std::to_string(j));
        auto pass_debug_group = ScopedDebugGroup(debug_str.c_str());

        constexpr vec4 clear_color = { 0.0f, 0.0f, 0.0f, 0.0f };
        raygen_framebuffer[1 - raygen_framebuffer_active]->clear(
          { clear_color, clear_color, clear_color, clear_color, clear_color });

        // Primary ray
#if !__EMSCRIPTEN__
        glQueryCounter(each_intersection_queries[j].start, GL_TIMESTAMP);
#endif
        encode_scene_traversal(*raygen_textures[raygen_framebuffer_active]
                                               [RG_OUT_RAY_DIRECTION_LOCATION]);
#if !__EMSCRIPTEN__
        glQueryCounter(each_intersection_queries[j].main_traversal,
                       GL_TIMESTAMP);
#endif
        encode_any_hit(j);

        // Swap buffers
        raygen_framebuffer_active = 1 - raygen_framebuffer_active;

        // Shadow ray (and blit)
        encode_scene_traversal(
          *raygen_textures[raygen_framebuffer_active]
                          [RG_OUT_SHADOW_RAY_DIRECTION_LOCATION]);
#if !__EMSCRIPTEN__
        glQueryCounter(each_intersection_queries[j].shadow_ray_traversal,
                       GL_TIMESTAMP);
#endif
        encode_shadow_ray_light_hit();
#if !__EMSCRIPTEN__
        glQueryCounter(each_intersection_queries[j].shadow_ray_hit,
                       GL_TIMESTAMP);
#endif

        // Swap buffers
        raygen_framebuffer_active = 1 - raygen_framebuffer_active;
      }
#if !__EMSCRIPTEN__
      glQueryCounter(timestamp_queries.intersections, GL_TIMESTAMP);
#endif
    }

    encode_accumulation();
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
      "frame energy accumulation " + std::to_string(i));
    raygen_textures[i][RG_OUT_SHADOW_RAY_DIRECTION_LOCATION] = Texture::create(
      width, height, Texture::MipMapFilter::nearest, Texture::Format::rgba32f);
    raygen_textures[i][RG_OUT_SHADOW_RAY_DIRECTION_LOCATION]->set_debug_name(
      "shadow ray direction " + std::to_string(i));
    raygen_textures[i][RG_OUT_SHADOW_RAY_DATA_LOCATION] = Texture::create(
      width, height, Texture::MipMapFilter::nearest, Texture::Format::rgba32f);
    raygen_textures[i][RG_OUT_SHADOW_RAY_DATA_LOCATION]->set_debug_name(
      "shadow ray (x: t, y: mat_id, zw: unused) " + std::to_string(i));
    raygen_framebuffer[i] = Framebuffer::create(
      raygen_textures[i].data(), (uint8_t)raygen_textures[i].size());
  }
}

void
RendererGpu::rebuild_scene_traversal()
{
  for (uint8_t i = 0; i < 2; ++i) {
    // t
    scene_traversal_textures[i][AH_HIT_RECORD_0_LOCATION] = Texture::create(
      width, height, Texture::MipMapFilter::nearest, Texture::Format::rgba32f);
    scene_traversal_textures[i][AH_HIT_RECORD_0_LOCATION]->set_debug_name(
      "hit record (t, unused yzw) [" + std::to_string(i) + "]");
    // position
    scene_traversal_textures[i][AH_HIT_RECORD_1_LOCATION] = Texture::create(
      width, height, Texture::MipMapFilter::nearest, Texture::Format::rgba32f);
    scene_traversal_textures[i][AH_HIT_RECORD_1_LOCATION]->set_debug_name(
      "hit record (position, unused w) [" + std::to_string(i) + "]");
    // uv
    scene_traversal_textures[i][AH_HIT_RECORD_2_LOCATION] = Texture::create(
      width, height, Texture::MipMapFilter::nearest, Texture::Format::rgba32f);
    scene_traversal_textures[i][AH_HIT_RECORD_2_LOCATION]->set_debug_name(
      "hit record (uv, unused zw) [" + std::to_string(i) + "]");
    // normal
    scene_traversal_textures[i][AH_HIT_RECORD_3_LOCATION] = Texture::create(
      width, height, Texture::MipMapFilter::nearest, Texture::Format::rgba32f);
    scene_traversal_textures[i][AH_HIT_RECORD_3_LOCATION]->set_debug_name(
      "hit record (normal, unused w) [" + std::to_string(i) + "]");
    // tangent
    scene_traversal_textures[i][AH_HIT_RECORD_4_LOCATION] = Texture::create(
      width, height, Texture::MipMapFilter::nearest, Texture::Format::rgba32f);
    scene_traversal_textures[i][AH_HIT_RECORD_4_LOCATION]->set_debug_name(
      "hit record (tangent, unused w) [" + std::to_string(i) + "]");
    // status, mat_id, bvh_hits
    scene_traversal_textures[i][AH_HIT_RECORD_5_LOCATION] = Texture::create(
      width, height, Texture::MipMapFilter::nearest, Texture::Format::rgba32f);
    scene_traversal_textures[i][AH_HIT_RECORD_5_LOCATION]->set_debug_name(
      "hit record (status, mat_id, bvh_hits) [" + std::to_string(i) + "]");

    scene_traversal_framebuffer[i] = Framebuffer::create(
      scene_traversal_textures[i].data(), scene_traversal_textures[i].size());
  }
}

void
RendererGpu::rebuild_backbuffers()
{
  rebuild_raygen_buffers();
  rebuild_scene_traversal();

  for (uint8_t i = 0; i < 2; ++i) {
    accumulation_texture[i] = Texture::create(
      width, height, Texture::MipMapFilter::nearest, Texture::Format::rgba32f);
    accumulation_texture[i]->set_debug_name("accumulated energy " +
                                            std::to_string(i));
    accumulation_framebuffer[i] =
      Framebuffer::create(&accumulation_texture[i], 1);
    constexpr vec4 clear_color = { 0.0f, 0.0f, 0.0f, 0.0f };
    accumulation_framebuffer[i]->clear({ clear_color });
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
  raygen_ray_uniform = Buffer::create(sizeof(raygen_uniform_t));
  raygen_ray_uniform->set_debug_name("raygen_ray_uniform");

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

  anyhit_uniform = Buffer::create(sizeof(anyhit_uniform_data_t));
  anyhit_uniform->set_debug_name("anyhit_uniform");
  shadow_ray_light_hit_uniform =
    Buffer::create(sizeof(shadow_ray_light_hit_uniform_data_t));
  shadow_ray_light_hit_uniform->set_debug_name("shadow_ray_light_hit_uniform");

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

  // Shadow Ray Hit
  info.fragment_shader_binary = gpu_3c_shadow_ray_light_hit_fs;
  info.fragment_shader_size = sizeof(gpu_3c_shadow_ray_light_hit_fs) /
                              sizeof(gpu_3c_shadow_ray_light_hit_fs[0]);
  info.fragment_shader_entry_point = "main";
  shadow_ray_light_hit_pipeline =
    Pipeline::create(Pipeline::Type::RasterOpenGL, info);

  // Accumulation
  info.fragment_shader_binary = gpu_4_energy_accumulation_fs;
  info.fragment_shader_size = sizeof(gpu_4_energy_accumulation_fs) /
                              sizeof(gpu_4_energy_accumulation_fs[0]);
  info.fragment_shader_entry_point = "main";
  accumulation_pipeline = Pipeline::create(Pipeline::Type::RasterOpenGL, info);

  // Final Blit
  info.fragment_shader_binary = gpu_5_postprocess_fs;
  info.fragment_shader_size =
    sizeof(gpu_5_postprocess_fs) / sizeof(gpu_5_postprocess_fs[0]);
  info.fragment_shader_entry_point = "main";
  final_blit_pipeline = Pipeline::create(Pipeline::Type::RasterOpenGL, info);
}

std::vector<std::pair<std::string, float>>
RendererGpu::evaluate_metrics()
{
  std::vector<std::pair<std::string, float>> result;

#if !__EMSCRIPTEN__
  using duration_format = std::chrono::duration<float, std::milli>;

  auto get_timestamp =
    [this](uint32_t query_object) -> std::chrono::nanoseconds {
    uint64_t timestamp = 0;
    glGetQueryObjectui64v(query_object, GL_QUERY_RESULT, &timestamp);
    return std::chrono::nanoseconds(timestamp);
  };

  auto get_duration = [](std::chrono::nanoseconds& start,
                         std::chrono::nanoseconds& end) {
    auto nanoseconds = end - start;
    return std::chrono::duration_cast<duration_format>(nanoseconds);
  };

  for (uint8_t i = 0; i < max_recursion_depth; ++i) {
    auto start = get_timestamp(each_intersection_queries[i].start);
    auto main_traversal =
      get_timestamp(each_intersection_queries[i].main_traversal);
    auto closest_hit = get_timestamp(each_intersection_queries[i].any_hit[0]);
    auto miss_all = get_timestamp(each_intersection_queries[i].any_hit[1]);
    auto shadow_ray_traversal =
      get_timestamp(each_intersection_queries[i].shadow_ray_traversal);
    auto shadow_ray_hit =
      get_timestamp(each_intersection_queries[i].shadow_ray_hit);

    auto main_traversal_ms = get_duration(start, main_traversal);
    auto closest_hit_ms = get_duration(main_traversal, closest_hit);
    auto miss_all_ms = get_duration(closest_hit, miss_all);
    auto shadow_ray_traversal_ms = get_duration(miss_all, shadow_ray_traversal);
    auto shadow_ray_hit_ms = get_duration(shadow_ray_traversal, shadow_ray_hit);

    result.emplace_back("[GRAPH:" + std::to_string(i) + "] main_traversal",
                        main_traversal_ms.count());
    result.emplace_back("[GRAPH:" + std::to_string(i) + "] closest_hit",
                        closest_hit_ms.count());
    result.emplace_back("[GRAPH:" + std::to_string(i) + "] miss_all",
                        miss_all_ms.count());
    result.emplace_back("[GRAPH:" + std::to_string(i) +
                          "] shadow_ray_traversal",
                        shadow_ray_traversal_ms.count());
    result.emplace_back("[GRAPH:" + std::to_string(i) + "] shadow_ray_hit",
                        shadow_ray_hit_ms.count());
  }

  auto start = get_timestamp(timestamp_queries.start);
  auto raygen = get_timestamp(timestamp_queries.raygen);
  auto intersections = get_timestamp(timestamp_queries.intersections);
  auto accumulation = get_timestamp(timestamp_queries.accumulation);
  auto final_blit = get_timestamp(timestamp_queries.final_blit);

  auto raygen_ms = get_duration(start, raygen);
  auto intersections_ms = get_duration(raygen, intersections);
  auto accumulation_ms = get_duration(intersections, accumulation);
  auto final_blit_ms = get_duration(accumulation, final_blit);

  result.emplace_back("raygen %.2f ms\n", raygen_ms.count());
  result.emplace_back("intersections %.2f ms\n", intersections_ms.count());
  result.emplace_back("accumulation %.2f ms\n", accumulation_ms.count());
  result.emplace_back("blit %.2f ms\n", final_blit_ms.count());
#endif

  return result;
}

std::vector<std::pair<std::string, uintptr_t>>
RendererGpu::debug_textures()
{
  auto result = std::vector<std::pair<std::string, uintptr_t>>();

  result.emplace_back(
    "ray origin",
    raygen_textures[raygen_framebuffer_active][RG_OUT_RAY_ORIGIN_LOCATION]
      ->get_native_handle());
  result.emplace_back(
    "ray direction",
    raygen_textures[raygen_framebuffer_active][RG_OUT_RAY_DIRECTION_LOCATION]
      ->get_native_handle());
  result.emplace_back("frame energy accumulation",
                      raygen_textures[raygen_framebuffer_active]
                                     [RG_OUT_ENERGY_ACCUMULATION_LOCATION]
                                       ->get_native_handle());
  result.emplace_back("shadow ray direction",
                      raygen_textures[raygen_framebuffer_active]
                                     [RG_OUT_SHADOW_RAY_DIRECTION_LOCATION]
                                       ->get_native_handle());
  result.emplace_back(
    "shadow ray data",
    raygen_textures[raygen_framebuffer_active][RG_OUT_SHADOW_RAY_DATA_LOCATION]
      ->get_native_handle());

  result.emplace_back(
    "hit record (t)",
    scene_traversal_textures[scene_traversal_framebuffer_active][0]
      ->get_native_handle());
  result.emplace_back(
    "hit record (position)",
    scene_traversal_textures[scene_traversal_framebuffer_active][1]
      ->get_native_handle());
  result.emplace_back(
    "hit record (uv)",
    scene_traversal_textures[scene_traversal_framebuffer_active][2]
      ->get_native_handle());
  result.emplace_back(
    "hit record (normal)",
    scene_traversal_textures[scene_traversal_framebuffer_active][3]
      ->get_native_handle());
  result.emplace_back(
    "hit record (tangent)",
    scene_traversal_textures[scene_traversal_framebuffer_active][4]
      ->get_native_handle());
  result.emplace_back(
    "hit record (status, mat_id, bvh_hits)",
    scene_traversal_textures[scene_traversal_framebuffer_active][5]
      ->get_native_handle());

  result.emplace_back(
    "accumulated energy",
    accumulation_texture[accumulation_framebuffer_active]->get_native_handle());

  return result;
}

uint8_t
RendererGpu::get_recursion_depth() const
{
  return max_recursion_depth;
}

void
RendererGpu::set_recursion_depth(uint8_t value)
{
  max_recursion_depth = value;
}
