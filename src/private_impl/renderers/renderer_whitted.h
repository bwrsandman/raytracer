#pragma once

#include "renderer.h"

#include <memory>
#include <vector>

#include "ray.h"

typedef void* SDL_GLContext;

namespace Raytracer {
class Camera;
namespace Graphics {
class Pipeline;
struct IndexedMesh;
struct Texture;
struct Framebuffer;

class RendererWhitted : public Renderer
{
public:
  explicit RendererWhitted(SDL_Window* window);
  ~RendererWhitted() override;

  void compute_primary_rays(const Camera& camera);
  void run(const Scene& scene) override;
  void set_backbuffer_size(uint16_t w, uint16_t h) override;
  bool get_debug() const override;
  void set_debug(bool value) override;
  void set_debug_data(uint32_t data) override;

  std::vector<std::pair<std::string, float>> evaluate_metrics() override { return {}; }

  uint32_t raygen(const Ray& ray,
                  const Scene& scene,
                  bool debug_bvh,
                  vec3& color) const;

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

  SDL_GLContext context;
  uint16_t width;
  uint16_t height;
  bool debug_bvh;
  uint32_t debug_bvh_count;

  std::vector<Ray> rays;
  std::vector<vec3> cpu_buffer;
  std::unique_ptr<Framebuffer> backbuffer;
  std::unique_ptr<Texture> gpu_buffer;
  std::unique_ptr<Pipeline> screen_space_pipeline;
  std::unique_ptr<IndexedMesh> fullscreen_quad;
};
} // namespace Graphics
} // namespace Raytracer
