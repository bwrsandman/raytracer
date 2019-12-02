#include "renderer_whitted.h"

#include <thread>

#include <SDL_video.h>

#include <glad/glad.h>

#if __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#endif

#include "camera.h"
#include "hit_record.h"
#include "hittable/object_list.h"
#include "hittable/sphere.h"
#include "material.h"
#include "materials/lambert_scatter.h"
#include "materials/metal.h"
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

vec3
RendererWhitted::color(const Ray& r, const Scene& scene, int depth)
{
  vec3 col = vec3(0.0f, 0.0f, 0.0f);

  hit_record rec;
  if (scene.get_world().hit(r, 0.001, std::numeric_limits<float>::max(), rec)) {
    // Ray scattered{};
    vec3 attenuation = vec3(0.0f, 0.0f, 0.0f);
    Ray scattered[2];
    //
    // if (depth > 10)
    //  printf("Depth: %i \n", depth);

    if (depth < 10) {
      if (scene.get_material(rec.mat_id)
            .scatter(scene, r, rec, attenuation, scattered)) {
        if (rec.mat_id == 7) { // hot fix for glass
          vec3 min_att = vec3(1.f, 1.f, 1.f) - attenuation;

          if ((attenuation.r() + min_att.r()) != 1.0f) {
            printf("whut %f \n", (attenuation.r() + min_att.r()));
          }
          if (attenuation.r() < 0.f || min_att.r() < 0.f) {
            printf("whut %f, %f \n", attenuation.r(), min_att.r());
          }

          vec3 col_reflect =
            attenuation * color(scattered[0], scene, depth + 1);
          vec3 col_refract = min_att * color(scattered[1], scene, depth + 1);
          col = col_reflect + col_refract;
        } else {
          col = attenuation * color(scattered[0], scene, depth + 1);
        }
      } else {
        col = attenuation;
      }
    } else {
      col = attenuation;
    }
  } else {
    vec3 unit_direction = unit_vector(r.direction);
    float t = 0.5f * (unit_direction.y() + 1.0f);
    col = (1.0f - t) * vec3(1.0f, 1.0f, 1.0f) + t * vec3(0.5f, 0.7f, 1.0f);
  }

  return col;
}

void
RendererWhitted::ray_gen(const Camera& camera)
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
    ray_gen(scene.get_camera());
  }

  std::vector<std::thread> threads;
  uint32_t num_cores = std::thread::hardware_concurrency();
  auto block_height = height / num_cores;
  for (uint32_t i = 0; i < num_cores; ++i) {
    auto offset = i * block_height * width;
    auto length = block_height * width;

    threads.emplace_back([this, offset, length, &scene]() {
      for (uint32_t i = 0; i < length; ++i) {
        cpu_buffer[offset + i] = color(rays[offset + i], scene, 0);
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
