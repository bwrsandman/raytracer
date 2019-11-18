#include "renderer_whitted.h"

#include <string_view>

#include <SDL.h>
#include <SDL_video.h>

#include <glad/glad.h>

#include "window.h"

#include "pipeline.h"

#include "shaders/passthrough_vs.h"
#include "shaders/fullscreen_fs.h"

#include "../../shaders/bridging_header.h"

namespace
{
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

IndexedMesh::IndexedMesh(uint32_t vertex_buffer, uint32_t index_buffer, uint32_t vao)
    : vertex_buffer(vertex_buffer)
    , index_buffer(index_buffer)
    , vao(vao)
{
}

IndexedMesh::~IndexedMesh()
{
    glDeleteBuffers(2, reinterpret_cast<uint32_t*>(this));
    glDeleteVertexArrays(1, &vao);
}

void IndexedMesh::bind() const
{
    glBindVertexArray(vao);
}

RendererWhitted::RendererWhitted(const Window& window)
  : width(0)
  , height(0)
{
  context = SDL_GL_CreateContext(window.get_native_handle());
  gladLoadGL();

  glGenTextures(1, &gpu_buffer);
  create_geometry();
  create_pipeline();
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

void
RendererWhitted::create_geometry()
{
  uint32_t buffers[2];
  uint32_t vao;
  glCreateBuffers(2, buffers);
  glGenVertexArrays(1, &vao);
  fullscreen_quad = std::make_unique<IndexedMesh>(buffers[0], buffers[1], vao);
  glBindVertexArray(fullscreen_quad->vao);
  glBindBuffer(GL_ARRAY_BUFFER, fullscreen_quad->vertex_buffer);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fullscreen_quad->index_buffer);
  glBufferStorage(GL_ARRAY_BUFFER,
                  sizeof(fullscreen_quad_vertices),
                  fullscreen_quad_vertices,
                  GL_MAP_READ_BIT);
  glBufferStorage(GL_ELEMENT_ARRAY_BUFFER,
                  sizeof(fullscreen_quad_indices),
                  fullscreen_quad_indices,
                  GL_MAP_READ_BIT);
}

void
RendererWhitted::create_pipeline()
{
  PipelineCreateInfo info;
  info.vertex_shader_binary = passthrough_vs;
  info.vertex_shader_size = sizeof(passthrough_vs);
  info.vertex_shader_entry_point = "main";
  info.fragment_shader_binary = fullscreen_fs;
  info.fragment_shader_size = sizeof(fullscreen_fs);
  info.fragment_shader_entry_point = "main";
  screen_space_pipeline = Pipeline::create(Pipeline::Type::RaterOpenGL, info);
}
