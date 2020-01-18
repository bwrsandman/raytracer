#pragma once

#include <chrono>
#include <memory>
#include <string>
#include <utility>
#include <vector>

struct SDL_Window;
union SDL_Event;

namespace Raytracer {
class Scene;
namespace Graphics {
class Renderer;
}

class Ui
{
public:
  explicit Ui(SDL_Window* window);

  void run(std::unique_ptr<Scene>& scene,
           Graphics::Renderer& renderer,
           const std::vector<std::pair<std::string, float>>& renderer_metrics,
           std::chrono::microseconds& dt);
  void draw() const;
  void process_event(const SDL_Event& event);

private:
  SDL_Window* window;
  bool show_stats;
};
} // namespace Raytracer
