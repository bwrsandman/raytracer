#include "input.h"
#include <SDL.h>

#include "vec3.h"
#include "camera.h"
#include "ui.h"

Input::Input()
  : quit(false)
{}

Input::~Input() = default;

void
Input::run()
{
  SDL_Event event;

  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_QUIT:
        quit = true;
        break;
      case SDL_KEYDOWN:
        switch (event.key.keysym.sym) {
#if __EMSCRIPTEN__
          case SDL_SCANCODE_ESCAPE: // weird sdl2 emscripten bug
#else
          case SDLK_ESCAPE:
#endif
            quit = true;
            break;
          //case SDLK_LEFT:

        }
        break;
    }
  }
}

void
Input::update_camera(Camera& camera) const
{
  //camera.change_camera(vec3(-2, 2, 1), vec3(0, 0, -1), vec3(0, 1, 0), 90, (width / height));
}

void
Input::update_ui(Ui& ui) const
{}

bool
Input::should_quit() const
{
  return quit;
}
