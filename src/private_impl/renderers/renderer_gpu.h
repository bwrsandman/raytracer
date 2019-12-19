#pragma once

#include "renderer.h"

#include <array>

typedef void* SDL_GLContext;

namespace Raytracer {
class Camera;
}

namespace Raytracer::Graphics {
struct Buffer;
struct IndexedMesh;
struct Texture;
struct Framebuffer;
class Pipeline;

class RendererGpu : public Renderer
{
public:
  explicit RendererGpu(SDL_Window* window);
  ~RendererGpu() override;

  void run(const Scene& world) override;
  void set_backbuffer_size(uint16_t width, uint16_t height) override;

  bool get_debug() const override;
  void set_debug(bool value) override;
  void set_debug_data(uint32_t data) override;

private:
  void upload_camera_uniforms(const Camera& camera);
  void upload_uniforms(const Scene& world);

  void rebuild_raygen_buffers();
  void rebuild_backbuffers();

  void create_pipelines();

  void encode_raygen();
  void encode_final_blit();

  SDL_GLContext context;
  uint16_t width;
  uint16_t height;

  std::unique_ptr<Framebuffer> backbuffer;

  std::unique_ptr<Pipeline> raygen_pipeline;
  std::unique_ptr<Buffer> raygen_ray_camera;
  std::array<std::unique_ptr<Texture>, 2> raygen_textures;
  std::unique_ptr<Framebuffer> raygen_framebuffer;

  std::unique_ptr<Pipeline> final_blit_pipeline;

  std::unique_ptr<IndexedMesh> fullscreen_quad;
};
} // namespace Raytracer::Graphics
