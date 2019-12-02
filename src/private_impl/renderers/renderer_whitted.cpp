#include "renderer_whitted.h"

#include <array>
#include <thread>
#include <vector>

#include <SDL_video.h>

#include <glad/glad.h>

#if __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#endif

#include "camera.h"
#include "hit_record.h"
#include "hittable/point.h"
#include "materials/material.h"
#include "pipeline.h"
#include "ray.h"
#include "scene.h"
#include "window.h"

#include "shaders/fullscreen_fs.h"
#include "shaders/passthrough_vs.h"

#include "../../shaders/bridging_header.h"

namespace {
void GLAPIENTRY
MessageCallback(GLenum source,
                GLenum type,
                GLuint id,
                GLenum severity,
                GLsizei length,
                const GLchar* message,
                const void* userParam)
{
  fprintf(stderr,
          "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
          (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
          type,
          severity,
          message);
}

static const float fullscreen_quad_vertices[8] = {
  // Top left
  0.0f,
  0.0f,
  // Top right
  1.0f,
  0.0f,
  // Bottom left
  1.0f,
  1.0f,
  // Bottom right
  0.0f,
  1.0f,
};
static const uint16_t fullscreen_quad_indices[6] = { 0, 1, 2, 2, 3, 0 };
}

IndexedMesh::IndexedMesh(uint32_t vertex_buffer,
                         uint32_t index_buffer,
                         uint32_t vao,
                         std::vector<MeshAttributes> attributes)
  : vertex_buffer(vertex_buffer)
  , index_buffer(index_buffer)
  , vao(vao)
  , attributes(std::move(attributes))
{}

IndexedMesh::~IndexedMesh()
{
  glDeleteBuffers(2, reinterpret_cast<uint32_t*>(this));
  glDeleteVertexArrays(1, &vao);
}

void
IndexedMesh::bind() const
{
  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
  uint32_t attr_index = 0;
  uintptr_t attr_offset = 0;
  for (auto& attr : attributes) {
    auto size = attr.count;
    switch (attr.type) {
      case GL_FLOAT:
        size *= sizeof(float);
        break;
      default:
        printf("unsupported type\n");
        return;
    }
    glEnableVertexAttribArray(attr_index);
    glVertexAttribPointer(attr_index,
                          attr.count,
                          attr.type,
                          GL_FALSE,
                          size,
                          reinterpret_cast<const void*>(attr_offset));
    attr_index++;
    attr_offset += size;
  }
}

RendererWhitted::RendererWhitted(const Window& window)
  : width(0)
  , height(0)
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

  context = SDL_GL_CreateContext(window.get_native_handle());
  gladLoadGLES2Loader(SDL_GL_GetProcAddress);

#if !__EMSCRIPTEN__
  glEnable(GL_DEBUG_OUTPUT);
  glDebugMessageCallback(MessageCallback, this);
#endif

  glGenTextures(1, &gpu_buffer);

  glGenSamplers(1, &linear_sampler);
  int32_t linear = GL_LINEAR;
  int32_t clamp_to_edge = GL_CLAMP_TO_EDGE;
  glSamplerParameteriv(linear_sampler, GL_TEXTURE_WRAP_S, &clamp_to_edge);
  glSamplerParameteriv(linear_sampler, GL_TEXTURE_WRAP_T, &clamp_to_edge);
  glSamplerParameteriv(linear_sampler, GL_TEXTURE_MIN_FILTER, &linear);
  glSamplerParameteriv(linear_sampler, GL_TEXTURE_MAG_FILTER, &linear);
#if !__EMSCRIPTEN__
  glObjectLabel(GL_SAMPLER, linear_sampler, -1, "Linear Sampler");
#endif

  create_geometry();
  create_pipeline();
}

RendererWhitted::~RendererWhitted()
{
  glDeleteTextures(1, &gpu_buffer);
  glDeleteSamplers(1, &linear_sampler);
}

void
RendererWhitted::trace(RayPayload& payload,
                       const Scene& scene,
                       const Ray& r,
                       float t_min,
                       float t_max) const
{
  auto& object_list = scene.get_world();
  hit_record rec;
  hit_record temp_rec;
  bool hit_anything = false;
  double closest_so_far = t_max;

  for (auto& object : object_list) {
    if (object->hit(r, t_min, closest_so_far, temp_rec)) {
      if (!hit_anything || closest_so_far > temp_rec.t) {
        hit_anything = true;
        closest_so_far = temp_rec.t;
        rec = temp_rec;
      }
    }
  }
  if (hit_anything) {
    payload.distance = rec.t;
    payload.normal = rec.normal;
    payload.tangent = rec.tangent;
    auto& mat = scene.get_material(rec.mat_id);
    mat.fill_type_data(scene, payload, rec.uv);
  } else {
    payload.distance = 0;
    payload.type = RayPayload::Type::NoHit;
  }
}

constexpr uint8_t MAX_SECONDARY = 20;
constexpr uint8_t MIN_ATTENUATION_MAGNITIUDE = 0.01f;

vec3
RendererWhitted::raygen(Ray primary_ray, const Scene& scene) const
{
  struct RayPP
  {
    Ray ray;
    vec3 attenuation;
  };
  thread_local std::array<RayPP, MAX_SECONDARY> secondary_rays;
  thread_local std::vector<RayPP> shadow_rays;
  // TODO: reserve
  shadow_rays.clear();

  // Insert primary ray as first in queue
  secondary_rays[0] = RayPP{ primary_ray, vec3(1, 1, 1) };
  uint8_t next_secondary = 1;

  RayPayload payload;
  constexpr float t_min = 0.001f;
  constexpr float t_max = std::numeric_limits<float>::max();

  vec3 color = { 0, 0, 0 };

  // Primary and Secondary rays
  for (uint8_t i = 0; i < next_secondary && i < MAX_SECONDARY; ++i) {
    if (dot(secondary_rays[i].attenuation, secondary_rays[i].attenuation) <
        MIN_ATTENUATION_MAGNITIUDE) {
      payload.distance = 1.0f;
      payload.type = RayPayload::Type::NoHit;
    } else {
      trace(payload, scene, secondary_rays[i].ray, t_min, t_max);
    }

    vec3 hit_pos = secondary_rays[i].ray.origin +
                   secondary_rays[i].ray.direction * payload.distance;

    if (payload.distance < 0.0f) {
      // TODO: Add nothing: internal reflection, this should never happen
      return vec3(1, 1, 0);
    } else if (payload.type == RayPayload::Type::Lambert) {
      // Add ray to shadow rays
      auto& geometry_list = scene.get_world();
      for (auto index : scene.get_light_indices()) {
        auto point_light =
          dynamic_cast<const Point*>(geometry_list[index].get());
        if (point_light == nullptr) {
          // This is not a point light and is not supported by Whitted
          // TODO: support textured spot lights, directional lights
          continue;
        }
        vec3 target = point_light->position;

        auto& ray = shadow_rays.emplace_back();
        ray.ray.direction = target - hit_pos;
        ray.ray.direction.make_unit_vector();
        ray.ray.origin = hit_pos + ray.ray.direction * t_min;
        ray.attenuation = secondary_rays[i].attenuation * payload.attenuation *
                          dot(payload.normal, ray.ray.direction);
      }
    } else if (payload.type == RayPayload::Type::Metal) {
      // Add ray in secondary ray queue
      if (next_secondary < MAX_SECONDARY) {
        // fully reflective per light)
        auto& ray = secondary_rays[next_secondary];
        ray.ray.origin = hit_pos + payload.normal * 0.001f;
        ray.ray.direction =
          reflect(secondary_rays[i].ray.direction, payload.normal);
        ray.attenuation = secondary_rays[i].attenuation * payload.attenuation;
        next_secondary++;
      }
    } else if (payload.type == RayPayload::Type::Dielectric) {
      float fraction_refracted = 0.0f;
      thread_local vec3 refracted_direction;
      // Prevent loss of energy
      if (next_secondary + 2 != MAX_SECONDARY &&
          refract(secondary_rays[i].ray.direction,
                  payload.normal,
                  payload.dielectric.ni,
                  payload.dielectric.nt,
                  refracted_direction)) {
        fraction_refracted =
          1.0f - fresnel_rate(secondary_rays[i].ray.direction,
                              payload.normal,
                              payload.dielectric.ni,
                              payload.dielectric.nt);
      }

      // Add refraction in secondary ray queue
      if (fraction_refracted > 0.001f && next_secondary < MAX_SECONDARY) {
        auto& new_ray = secondary_rays[next_secondary];
        new_ray.ray.origin = hit_pos - payload.normal * 0.001f;
        new_ray.ray.direction = refracted_direction;
        new_ray.attenuation =
          secondary_rays[i].attenuation * fraction_refracted;
        next_secondary++;
      }
      // Add reflection in secondary ray queue
      if (fraction_refracted < 0.999f && next_secondary < MAX_SECONDARY) {
        auto& new_ray = secondary_rays[next_secondary];
        new_ray.ray.origin = hit_pos + payload.normal * 0.001f;
        new_ray.ray.direction =
          reflect(secondary_rays[i].ray.direction, payload.normal);
        new_ray.attenuation =
          secondary_rays[i].attenuation * (1.0f - fraction_refracted);
        next_secondary++;
      }
    } else {
      // No hit or hit light, add sky
      vec3 unit_direction = unit_vector(secondary_rays[i].ray.direction);
      float t = 0.5f * (unit_direction.y() + 1.0f);
      static constexpr vec3 top = vec3(0.5, 0.7, 1.0);
      static constexpr vec3 bot = vec3(1.0, 1.0, 1.0);
      color += lerp(top, bot, t) * secondary_rays[i].attenuation;
      if (payload.type == RayPayload::Type::Emissive) {
        // Hit a light, this should happen very rarely
        color += secondary_rays[i].attenuation * payload.emission;
      } else if (payload.type != RayPayload::Type::NoHit) {
        // TODO: Add nothing: this should never happen unless there's a new type
        // of payload
        return vec3(0, 1, 1);
      }
    }
  }

  // Shadow Rays
  for (auto& ray : shadow_rays) {
    trace(payload, scene, ray.ray, t_min, t_max);
    if (payload.type == RayPayload::Type::Emissive) {
      // Accumulate color
      color += payload.emission * ray.attenuation;
    }
  }

  return color;
}

void
RendererWhitted::compute_primary_rays(const Camera& camera)
{
  for (uint32_t y = 0; y < height; ++y) {
    for (uint32_t x = 0; x < width; ++x) {
      float u = static_cast<float>(x) / width;
      float v = static_cast<float>(y) / height;
      rays[x + y * width] = camera.get_ray(u, v);
    }
  }
}

void
RendererWhitted::run(const Scene& scene)
{
  static const float clear_color[4] = { 1.0f, 1.0f, 0.0f, 1.0f };

  auto& cam = scene.get_camera();

  // raytracing
  if (cam.is_dirty()) {
    compute_primary_rays(scene.get_camera());
  }

  std::vector<std::thread> threads;
  uint32_t num_cores = std::thread::hardware_concurrency();
  auto block_height = height / num_cores;
  for (uint32_t i = 0; i < num_cores; ++i) {
    auto offset = i * block_height * width;
    auto length = block_height * width;

    threads.emplace_back([this, offset, length, &scene]() {
      for (uint32_t i = 0; i < length; ++i) {
        cpu_buffer[offset + i] = std::sqrt(raygen(rays[offset + i], scene));
      }
    });
  }

  for (auto& t : threads) {
    t.join();
  }

  // binding texture
  glBindTexture(GL_TEXTURE_2D, gpu_buffer);
  glTexSubImage2D(
    GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_FLOAT, cpu_buffer.data());

  // clearing screen
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glClearBufferfv(GL_COLOR, 0, clear_color);

  // Actually putting it to the screen?
  screen_space_pipeline->bind();
  fullscreen_quad->bind();
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, gpu_buffer);
  glBindSampler(0, linear_sampler);
  glDrawElements(GL_TRIANGLES,
                 sizeof(fullscreen_quad_indices) /
                   sizeof(fullscreen_quad_indices[0]),
                 GL_UNSIGNED_SHORT,
                 nullptr);

  glFinish();
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

  for (uint32_t y = 0; y < height; ++y) {
    for (uint32_t x = 0; x < width; ++x) {
      cpu_buffer[y * width + x][0] =
        ((y / 4) % 2) * static_cast<float>(x) / width;
      cpu_buffer[y * width + x][1] =
        ((y / 4) % 2) * static_cast<float>(y) / height;
      cpu_buffer[y * width + x][2] = 0;
    }
  }

  glBindTexture(GL_TEXTURE_2D, gpu_buffer);
  glTexImage2D(GL_TEXTURE_2D,
               0,
               GL_RGB32F,
               width,
               height,
               0,
               GL_RGB,
               GL_FLOAT,
               cpu_buffer.data());
#if !__EMSCRIPTEN__
  glObjectLabel(GL_TEXTURE, gpu_buffer, -1, "CPU-GPU buffer");
#endif

  glViewport(0, 0, width, height);

  rays.resize(width * height);
}

void
RendererWhitted::create_geometry()
{
  uint32_t buffers[2];
  uint32_t vao;
  glGenBuffers(2, buffers);
  glGenVertexArrays(1, &vao);
  fullscreen_quad = std::make_unique<IndexedMesh>(
    buffers[0],
    buffers[1],
    vao,
    std::vector<IndexedMesh::MeshAttributes>{
      IndexedMesh::MeshAttributes{ GL_FLOAT, 2 } });
  fullscreen_quad->bind();
  glBufferData(GL_ARRAY_BUFFER,
               sizeof(fullscreen_quad_vertices),
               fullscreen_quad_vertices,
               GL_STATIC_DRAW);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               sizeof(fullscreen_quad_indices),
               fullscreen_quad_indices,
               GL_STATIC_DRAW);
}

void
RendererWhitted::create_pipeline()
{
  PipelineCreateInfo info;
  info.vertex_shader_binary = passthrough_vs;
  info.vertex_shader_size = sizeof(passthrough_vs) / sizeof(passthrough_vs[0]);
  info.vertex_shader_entry_point = "main";
  info.fragment_shader_binary = fullscreen_fs;
  info.fragment_shader_size = sizeof(fullscreen_fs) / sizeof(fullscreen_fs[0]);
  info.fragment_shader_entry_point = "main";
  screen_space_pipeline = Pipeline::create(Pipeline::Type::RaterOpenGL, info);
}
