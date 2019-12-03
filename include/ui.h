#pragma once

#include <chrono>
#include <memory>

struct SDL_Window;
union SDL_Event;
class Scene;

class Ui
{
public:
  explicit Ui(SDL_Window* window);

  void run(std::unique_ptr<Scene>& scene, std::chrono::microseconds& dt) const;
  void draw() const;
  void process_event(const SDL_Event& event);

private:
  SDL_Window* window;
};
