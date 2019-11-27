#pragma once

struct SDL_Window;
union SDL_Event;
class Scene;

class Ui
{
public:
  explicit Ui(SDL_Window* window);

  void run(Scene& scene) const;
  void draw() const;
  void process_event(const SDL_Event& event);

private:
  SDL_Window* window;
};
