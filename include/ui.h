#pragma once

struct SDL_Window;

class Ui
{
public:
  explicit Ui(SDL_Window* window);

  void run() const;

 private:
    SDL_Window* window;
};
