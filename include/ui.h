#pragma once

struct SDL_Window;
union SDL_Event;

class Ui
{
public:
  explicit Ui(SDL_Window* window);

  void run() const;
  void draw() const;
  void process_event(const SDL_Event& event);

private:
  SDL_Window* window;
};
