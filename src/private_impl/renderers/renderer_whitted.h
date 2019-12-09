#pragma once

#include "renderer.h"
#include <memory>
#include <vector>

#include "ray.h"
//#include "vec3.h"

class vec3;
class Object;
class Camera;
class Pipeline;

typedef void* SDL_GLContext;

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
  explicit RendererWhitted(SDL_Window* window);
  ~RendererWhitted() override;

  void compute_primary_rays(const Camera& camera);
  void run(const Scene& scene) override;
  void set_backbuffer_size(uint16_t w, uint16_t h) override;

private:
  void rebuild_backbuffers();
  void create_geometry();
  void create_pipeline();
  bool trace(RayPayload& payload,
             const Scene& scene,
             const Ray& r,
             bool early_out,
             float t_min,
             float t_max) const;
  vec3 raygen(Ray ray, const Scene& scene) const;

  SDL_GLContext context;
  uint16_t width;
  uint16_t height;

  std::vector<Ray> rays;
  std::vector<vec3> cpu_buffer;
  uint32_t gpu_buffer;
  uint32_t linear_sampler;
  std::unique_ptr<Pipeline> screen_space_pipeline;
  std::unique_ptr<IndexedMesh> fullscreen_quad;
};
