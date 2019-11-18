#pragma once

#include "renderer.h"
#include <vector>

#include <memory>

typedef void* SDL_GLContext;
struct Pixel
{
  float r;
  float g;
  float b;
  float a;
};

class Pipeline;

struct IndexedMesh
{
  const uint32_t vertex_buffer;
  const uint32_t index_buffer;
  const uint32_t vao;

  IndexedMesh(uint32_t vertex_buffer, uint32_t index_buffer, uint32_t vao);
  virtual ~IndexedMesh();
  void bind() const;
};

class RendererWhitted : public Renderer
{
public:
  explicit RendererWhitted(const Window& window);
  ~RendererWhitted() override;

  void run(std::chrono::microseconds dt) override;
  void set_backbuffer_size(uint16_t w, uint16_t h) override;

private:
  void rebuild_backbuffers();
  void create_geometry();
  void create_pipeline();

  SDL_GLContext context;
  uint16_t width;
  uint16_t height;

  std::vector<Pixel> cpu_buffer;
  uint32_t gpu_buffer;
  uint32_t linear_sampler;
  std::unique_ptr<Pipeline> screen_space_pipeline;
  std::unique_ptr<IndexedMesh> fullscreen_quad;
};
