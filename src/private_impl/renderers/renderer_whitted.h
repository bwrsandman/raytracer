#pragma once

#include "renderer.h"
#include <memory>
#include <vector>

#include "vec3.h"
class Ray;
class Object;
class Camera;


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
  struct MeshAttributes
  {
    uint32_t type;
    uint32_t count;
  };
  const uint32_t vertex_buffer;
  const uint32_t index_buffer;
  const uint32_t vao;
  const std::vector<MeshAttributes> attributes;

  IndexedMesh(uint32_t vertex_buffer,
              uint32_t index_buffer,
              uint32_t vao,
              std::vector<MeshAttributes> attributes);
  virtual ~IndexedMesh();
  void bind() const;
};

class RendererWhitted : public Renderer
{
public:
  explicit RendererWhitted(const Window& window);
  ~RendererWhitted() override;

  void run() override;
  void set_backbuffer_size(uint16_t w, uint16_t h) override;

private:
  void rebuild_backbuffers();
  void create_geometry();
  void create_pipeline();
  vec3 color(const Ray& r, Object* world, int depth);
  //vec3 color(const Ray& r, Object* world);


  SDL_GLContext context;
  uint16_t width;
  uint16_t height;

  std::vector<Pixel> cpu_buffer;
  uint32_t gpu_buffer;
  uint32_t linear_sampler;
  std::unique_ptr<Pipeline> screen_space_pipeline;
  std::unique_ptr<IndexedMesh> fullscreen_quad;
};
