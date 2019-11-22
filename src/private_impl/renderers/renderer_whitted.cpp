#include "renderer_whitted.h"

#include <SDL_video.h>

#include <glad/glad.h>
#include <scene.h>

#if __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#endif

#include "pipeline.h"
#include "window.h"

#include "camera.h"
#include "hit_record.h"
#include "hittable/object_list.h"
#include "hittable/sphere.h"
#include "material.h"
#include "materials/lambert.h"
#include "materials/metal.h"
#include "ray.h"

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
  hit_record rec;
  if (scene.get_world().hit(r, 0.001, std::numeric_limits<float>::max(), rec)) {
    Ray scattered;
    vec3 attenuation;
    if (depth < 50 && scene.get_material(rec.mat_id)
                        .scatter(r, rec, attenuation, scattered)) {
      return attenuation * color(scattered, scene, depth + 1);
    } else {
      return vec3(0, 0, 0);
    }

  } else {
    vec3 unit_direction = unit_vector(r.direction());
    float t = 0.5 * (unit_direction.y() + 1.0);
    return (1.0 - t) * vec3(1.0, 1.0, 1.0) + t * vec3(0.5, 0.7, 1.0);
  }
}

void
RendererWhitted::run(const Scene& scene)
{
  static const float clear_color[4] = { 1.0f, 1.0f, 0.0f, 1.0f };

  vec3 lower_left_corner(-4.0, 3.0, -3.0);
  vec3 horizontal(8.0, 0.0, 0.0);
  vec3 vertical(0.0, -6.0, 0.0);
  vec3 origin(0.0, 0.0, 0.0);

  Camera cam;

  // raytracing
  for (uint32_t y = 0; y < height; ++y) {
    for (uint32_t x = 0; x < width; ++x) {

      float u = static_cast<float>(x) / width; // 
      float v = static_cast<float>(y) / height;

      //Ray r(origin, lower_left_corner + u * horizontal + v * vertical);
      Ray r = cam.get_ray(u, v);

      //vec3 p = r.point_at_parameter(2.0);
      vec3 col = color(r, scene, 0);

      cpu_buffer[y * width + x] = { sqrt(col.r()), sqrt(col.g()), sqrt(col.b()), 1.0f };
     
    }
  }

  // binding texture
  glBindTexture(GL_TEXTURE_2D, gpu_buffer);
  glTexSubImage2D(GL_TEXTURE_2D,
                  0,
                  0,
                  0,
                  width,
                  height,
                  GL_RGBA,
                  GL_FLOAT,
                  cpu_buffer.data());

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
      cpu_buffer[y * width + x].r =
        ((y / 4) % 2) * static_cast<float>(x) / width;
      cpu_buffer[y * width + x].g =
        ((y / 4) % 2) * static_cast<float>(y) / height;
      cpu_buffer[y * width + x].b = 0;
      cpu_buffer[y * width + x].a = 1.0f;
    }
  }

  glBindTexture(GL_TEXTURE_2D, gpu_buffer);
  glTexImage2D(GL_TEXTURE_2D,
               0,
               GL_RGBA32F,
               width,
               height,
               0,
               GL_RGBA,
               GL_FLOAT,
               cpu_buffer.data());
#if !__EMSCRIPTEN__
  glObjectLabel(GL_TEXTURE, gpu_buffer, -1, "CPU-GPU buffer");
#endif

  glViewport(0, 0, width, height);
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
