#pragma once

#include "renderer.h"

#include <cstdint>

#include <array>
#include <vector>

typedef void* SDL_GLContext;

namespace Raytracer {
class Camera;
namespace Hittable {
struct Object;
}
}

namespace Raytracer::Graphics {
struct Buffer;
struct IndexedMesh;
struct Texture;
struct Framebuffer;
class Pipeline;

using Raytracer::Hittable::Object;

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
  void upload_scene(const std::vector<std::unique_ptr<Object>>& objects);
  void upload_anyhit_uniforms();
  void upload_uniforms(const Scene& world);

  void rebuild_raygen_buffers();
  void rebuild_scene_traversal();
  void rebuild_backbuffers();

  void create_pipelines();

  void encode_raygen();
  void encode_scene_traversal();
  void encode_any_hit();
  void encode_final_blit();

  SDL_GLContext context;
  uint32_t frame_count;
  uint16_t width;
  uint16_t height;

  std::unique_ptr<Framebuffer> backbuffer;

  std::unique_ptr<Pipeline> raygen_pipeline;
  std::unique_ptr<Buffer> raygen_ray_camera;
  std::array<std::unique_ptr<Texture>, 2> raygen_textures;
  std::unique_ptr<Framebuffer> raygen_framebuffer;

  std::unique_ptr<Texture> scene_traversal_textures_ah_hit_record_0[2];
  std::array<std::unique_ptr<Texture>, 5> scene_traversal_textures;
  std::unique_ptr<Framebuffer> scene_traversal_framebuffer[2];
  uint8_t scene_traversal_framebuffer_active;
  std::unique_ptr<Pipeline> scene_traversal_sphere_pipeline;
  std::unique_ptr<Buffer> scene_traversal_spheres;

  std::unique_ptr<Buffer> anyhit_uniform;
  std::unique_ptr<Pipeline> closest_hit_pipeline;
  std::unique_ptr<Texture> closest_hit_textures[1];
  std::unique_ptr<Framebuffer> closest_hit_framebuffer;
  std::unique_ptr<Pipeline> miss_all_pipeline;
  std::unique_ptr<Texture> miss_all_textures[1];
  std::unique_ptr<Framebuffer> miss_all_framebuffer;

  std::unique_ptr<Pipeline> final_blit_pipeline;

  std::unique_ptr<IndexedMesh> fullscreen_quad;
};
} // namespace Raytracer::Graphics
