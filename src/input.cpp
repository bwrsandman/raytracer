#include "input.h"

#include <SDL.h>

#include "ui.h"

Input::Input()
  : quit(false)
{}

Input::~Input() = default;

void
Input::run(Ui& ui)
{
  SDL_Event event;

  while (SDL_PollEvent(&event)) {
    ui.process_event(event);
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
        }
        break;
    }
  }

}

void
Input::update_camera(Camera& camera) const
{}

bool
Input::should_quit() const
{
  return quit;
}
