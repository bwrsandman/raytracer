#include "renderer_whitted.h"

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

#include "../../shaders/bridging_header.h"
#include "shaders/fullscreen_fs.h"
#include "shaders/passthrough_flip_y_vs.h"

#include "../graphics/indexed_mesh.h"
#include "../graphics/texture.h"
#include "../graphics/framebuffer.h"

using Raytracer::Graphics::IndexedMesh;
using Raytracer::Graphics::RendererWhitted;
using Raytracer::Graphics::Framebuffer;
using Raytracer::Hittable::Point;
using namespace Raytracer::Math;
using namespace Raytracer;

namespace {
void GLAPIENTRY
MessageCallback([[maybe_unused]] GLenum source,
                GLenum type,
                [[maybe_unused]] GLuint id,
                GLenum severity,
                [[maybe_unused]] GLsizei length,
                const GLchar* message,
                [[maybe_unused]] const void* userParam)
{
  fprintf(stderr,
          "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
          (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
          type,
          severity,
          message);
}
} // namespace

RendererWhitted::RendererWhitted(SDL_Window* window)
  : width(0)
  , height(0)
  , debug_bvh(false)
  , debug_bvh_count(100)
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
    create_geometry();
    create_pipeline();
  }
}

RendererWhitted::~RendererWhitted()
{
  SDL_GL_DeleteContext(context);
}

bool
RendererWhitted::trace(RayPayload& payload,
                       const Scene& scene,
                       const Ray& r,
                       bool early_out,
                       float t_min,
                       float t_max) const
{
  auto& object_list = scene.get_world();
  hit_record rec;
  hit_record temp_rec;
  bool hit_anything = false;
  auto closest_so_far = t_max;

  for (auto& object : object_list) {
    if (object->hit(r, early_out, t_min, closest_so_far, temp_rec) &&
        closest_so_far > temp_rec.t) {
      hit_anything = true;
      closest_so_far = temp_rec.t;
      temp_rec.bvh_hits += rec.bvh_hits;
      rec = temp_rec;
      if (early_out) {
        break;
      }
    } else {
      rec.bvh_hits += temp_rec.bvh_hits;
    }
  }
  if (hit_anything) {
    payload.distance = rec.t;
    payload.normal = rec.normal;
    payload.tangent = rec.tangent;
    payload.attenuation = vec3(1.f, 1.f, 1.f);
    auto& mat = scene.get_material(rec.mat_id);
    mat.fill_type_data(scene, payload, rec.uv);
  } else {
    payload.distance = 0;
    payload.type = RayPayload::Type::NoHit;
  }
  payload.bvh_hits = rec.bvh_hits;
  return hit_anything;
}

uint32_t
RendererWhitted::raygen(const Ray& primary_ray,
                        const Scene& scene,
                        bool _debug_bvh,
                        vec3& color) const
{
  struct AttenuatedRay
  {
    Ray ray;
    vec3 attenuation;
  };
  struct AttenuatedRayMaxed
  {
    AttenuatedRay ray;
    uint16_t mat_id;
    float t_max;
  };
  thread_local std::vector<AttenuatedRay> secondary_rays;
  secondary_rays.resize(scene.max_secondary_rays);
  thread_local std::vector<AttenuatedRayMaxed> shadow_rays;
  // TODO: reserve
  shadow_rays.clear();

  // Insert primary ray as first in queue
  secondary_rays[0] = AttenuatedRay{ primary_ray, vec3(1, 1, 1) };
  uint8_t next_secondary = 1;

  RayPayload payload;
  constexpr float t_min = 0.001f;
  constexpr float t_max = std::numeric_limits<float>::max();

  // Primary and Secondary rays
  for (uint8_t i = 0; i < next_secondary && i < scene.max_secondary_rays; ++i) {
    if (dot(secondary_rays[i].attenuation, secondary_rays[i].attenuation) <
        scene.min_attenuation_magnitude) {
      payload.distance = 1.0f;
      payload.type = RayPayload::Type::NoHit;
    } else {
      trace(payload, scene, secondary_rays[i].ray, false, t_min, t_max);
    }

    if (_debug_bvh) {
      float bvh_debug = payload.bvh_hits / static_cast<float>(debug_bvh_count);
      color = vec3(bvh_debug, bvh_debug, bvh_debug);
      if (payload.bvh_hits > debug_bvh_count * 3) {
        color = vec3(1.0f, 0.0f, 0.0f);
      } else if (payload.bvh_hits > debug_bvh_count * 2) {
        color = vec3(0.0f, 1.0f, 0.0f);
      } else if (payload.bvh_hits > debug_bvh_count) {
        color = vec3(0.0f, 0.0f, 1.0f);
      }
      return payload.bvh_hits;
    }

    vec3 hit_pos = secondary_rays[i].ray.origin +
                   secondary_rays[i].ray.direction * payload.distance;

    if (payload.distance < 0.0f) {
      // TODO: Add nothing: internal reflection, this should never happen
      color = vec3(1, 1, 0);
      return next_secondary;
    } else if (payload.type == RayPayload::Type::Lambert) {
      // Add ray to shadow rays
      for (auto& light : scene.get_lights()) {
        auto point_light = dynamic_cast<const Point*>(light.get());
        if (point_light == nullptr) {
          // This is not a point light and is not supported by Whitted
          // TODO: support textured spot lights, directional lights
          continue;
        }
        vec3 target = point_light->position;

        auto& ray = shadow_rays.emplace_back();
        ray.ray.ray.direction = target - hit_pos;
        ray.t_max = (target - hit_pos).length();
        ray.ray.ray.direction /= ray.t_max;
        ray.mat_id = point_light->mat_id;
        ray.ray.ray.origin = hit_pos + ray.ray.ray.direction * t_min;
        ray.ray.attenuation = secondary_rays[i].attenuation *
                              payload.attenuation *
                              dot(payload.normal, ray.ray.ray.direction);
      }
    } else if (payload.type == RayPayload::Type::Metal) {
      // Add ray in secondary ray queue
      if (next_secondary < scene.max_secondary_rays) {
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
      bool inside_dielectric = false;
      // Prevent loss of energy
      if (next_secondary + 2 != scene.max_secondary_rays &&
          refract(secondary_rays[i].ray.direction,
                  payload.normal,
                  payload.dielectric.ni,
                  payload.dielectric.nt,
                  refracted_direction,
                  inside_dielectric)) {
        fraction_refracted =
          1.0f - fresnel_rate(secondary_rays[i].ray.direction,
                              payload.normal,
                              payload.dielectric.ni,
                              payload.dielectric.nt);
      }

      vec3 absorb(1.f, 1.f, 1.f);

      // Beer's law
      if (inside_dielectric) {
        float dist = payload.distance * 15.f;

        absorb = vec3(expf(-payload.attenuation.r() * dist),
                      expf(-payload.attenuation.g() * dist),
                      expf(-payload.attenuation.b() * dist));
      }

      // Add refraction in secondary ray queue
      if (fraction_refracted > 0.001f &&
          next_secondary < scene.max_secondary_rays) {
        auto& new_ray = secondary_rays[next_secondary];
        new_ray.ray.origin = hit_pos - payload.normal * 0.001f;
        new_ray.ray.direction = refracted_direction;

        new_ray.attenuation =
          secondary_rays[i].attenuation * absorb * fraction_refracted;
        next_secondary++;
      }
      // Add reflection in secondary ray queue
      if (fraction_refracted < 0.999f &&
          next_secondary < scene.max_secondary_rays) {
        auto& new_ray = secondary_rays[next_secondary];
        new_ray.ray.origin = hit_pos + payload.normal * 0.001f;
        new_ray.ray.direction =
          reflect(secondary_rays[i].ray.direction, payload.normal);

        new_ray.attenuation =
          secondary_rays[i].attenuation * absorb * (1.0f - fraction_refracted);
        next_secondary++;
      }
    } else {
      // No hit or hit light, add sky
      vec3 unit_direction = normalize(secondary_rays[i].ray.direction);
      float t = 0.5f * (unit_direction.y() + 1.0f);
      static constexpr vec3 top = vec3(0.5f, 0.7f, 1.0f);
      static constexpr vec3 bot = vec3(1.0f, 1.0f, 1.0f);
      color += lerp(top, bot, t) * secondary_rays[i].attenuation;
      if (payload.type == RayPayload::Type::Emissive) {
        // Hit a light, this should happen very rarely
        color += secondary_rays[i].attenuation * payload.emission;
      } else if (payload.type != RayPayload::Type::NoHit) {
        // TODO: Add nothing: this should never happen unless there's a new type
        // of payload
        color = vec3(0, 1, 1);
        return next_secondary;
      }
    }
  }

  // Shadow Rays
  for (auto& ray : shadow_rays) {
    if (!trace(payload, scene, ray.ray.ray, true, t_min, ray.t_max)) {
      auto& mat = scene.get_material(ray.mat_id);
      vec3 uv;
      payload.distance = ray.t_max;
      mat.fill_type_data(scene, payload, uv);
      color += payload.emission * ray.ray.attenuation;
    }
  }

  return next_secondary + static_cast<uint32_t>(shadow_rays.size());
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
  static const vec4 clear_color = { 1.0f, 1.0f, 0.0f, 1.0f };

  auto& cam = scene.get_camera();

  // raytracing
  if (cam.is_dirty()) {
    compute_primary_rays(scene.get_camera());
  }

  std::vector<std::thread> threads;
  uint32_t num_cores = std::thread::hardware_concurrency();
  auto block_height = height / num_cores;
  if (height % num_cores) {
    block_height++;
  }
  for (uint32_t i = 0; i < num_cores; ++i) {
    uint32_t offset = i * block_height * width;
    uint32_t length = block_height * width;

    threads.emplace_back([this, offset, length, &scene]() {
      for (uint32_t i = offset;
           i < offset + length && static_cast<int>(i) < width * height;
           ++i) {
        vec3 color = vec3(0, 0, 0);
        raygen(rays[i], scene, debug_bvh, color);
        cpu_buffer[i] = std::sqrt(color);
      }
    });
  }

  for (auto& t : threads) {
    t.join();
  }

  // binding texture
  if (context) {
    if (gpu_buffer) {
      gpu_buffer->upload(
        cpu_buffer.data(),
        static_cast<uint32_t>(cpu_buffer.size() * sizeof(cpu_buffer[0])));
    }

    // clearing screen
    backbuffer->clear({ clear_color });

    // Actually putting it to the screen?
    screen_space_pipeline->bind();
    if (gpu_buffer) {
      gpu_buffer->bind(0);
    }
    fullscreen_quad->draw();

    glFinish();
  }
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

bool
RendererWhitted::get_debug() const
{
  return debug_bvh;
}
void
RendererWhitted::set_debug(bool value)
{
  debug_bvh = value;
}
void
RendererWhitted::set_debug_data(uint32_t data)
{
  debug_bvh_count = data;
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

  rays.resize(width * height);

  if (context) {
    gpu_buffer = Texture::create(width, height, Texture::Format::rgb32f);
    gpu_buffer->set_debug_name("CPU-GPU buffer");

    glViewport(0, 0, width, height);
  }
}

void
RendererWhitted::create_geometry()
{
  fullscreen_quad = IndexedMesh::create_fullscreen_quad();
}

void
RendererWhitted::create_pipeline()
{
  PipelineCreateInfo info;
  info.vertex_shader_binary = passthrough_flip_y_vs;
  info.vertex_shader_size =
    sizeof(passthrough_flip_y_vs) / sizeof(passthrough_flip_y_vs[0]);
  info.vertex_shader_entry_point = "main";
  info.fragment_shader_binary = fullscreen_fs;
  info.fragment_shader_size = sizeof(fullscreen_fs) / sizeof(fullscreen_fs[0]);
  info.fragment_shader_entry_point = "main";
  screen_space_pipeline = Pipeline::create(Pipeline::Type::RaterOpenGL, info);
}
