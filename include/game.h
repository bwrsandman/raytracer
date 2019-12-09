#pragma once

#include <chrono>
#include <memory>

namespace Raytracer {
class Window;
class Input;
class Scene;
class Ui;

namespace Graphics {
class Renderer;
}

using Graphics::Renderer;

class Game
{
public:
  Game();
  virtual ~Game();

  void run();
  bool main_loop();

private:
  void take_timestamp();
  std::chrono::microseconds get_delta_time() const;
  std::unique_ptr<Window> window;
  std::unique_ptr<Input> input;
  std::unique_ptr<Renderer> renderer;
  std::unique_ptr<Ui> ui;
  std::unique_ptr<Scene> scene;
  std::chrono::high_resolution_clock::time_point frame_begin;
  std::chrono::high_resolution_clock::time_point frame_end;
};
} // namespace Raytracer
