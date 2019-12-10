#include "input.h"

#include <SDL.h>

#include "camera.h"
#include "scene.h"
#include "ui.h"

#if USE_SPNAV
#include <spnav.h>
#endif

using Raytracer::Input;

Input::Input()
  : quit(false)
{
#if USE_SPNAV
  if (spnav_open() == -1) {
    std::fprintf(stderr, "failed to connect to the space navigator daemon\n");
  }
#endif
}

Input::~Input()
{
#if USE_SPNAV
  spnav_close();
#endif
}

void
Input::run(Ui& ui, Scene& scene, std::chrono::microseconds& dt)
{
  SDL_Event event;

#if USE_SPNAV
  thread_local int last_x = 0;
  thread_local int last_y = 0;
  thread_local int last_z = 0;
  thread_local int last_rx = 0;
  thread_local int last_ry = 0;
  thread_local int last_rz = 0;
  spnav_event sev;

  while (spnav_poll_event(&sev)) {
    if (sev.type == SPNAV_EVENT_MOTION) {
      if (last_x != sev.motion.x) {
        event.jaxis.type = SDL_JOYAXISMOTION;
        event.jaxis.timestamp = SDL_GetTicks();
        event.jaxis.which = 0;
        event.jaxis.axis = 0;
        event.jaxis.value = sev.motion.x;
        SDL_PushEvent(&event);
        last_x = sev.motion.x;
      }
      if (last_y != sev.motion.y) {
        event.jaxis.type = SDL_JOYAXISMOTION;
        event.jaxis.timestamp = SDL_GetTicks();
        event.jaxis.which = 0;
        event.jaxis.axis = 1;
        event.jaxis.value = sev.motion.y;
        SDL_PushEvent(&event);
        last_y = sev.motion.y;
      }
      if (last_z != sev.motion.z) {
        event.jaxis.type = SDL_JOYAXISMOTION;
        event.jaxis.timestamp = SDL_GetTicks();
        event.jaxis.which = 0;
        event.jaxis.axis = 2;
        event.jaxis.value = sev.motion.z;
        SDL_PushEvent(&event);
        last_z = sev.motion.z;
      }
      if (last_rx != sev.motion.rx) {
        event.jaxis.type = SDL_JOYAXISMOTION;
        event.jaxis.timestamp = SDL_GetTicks();
        event.jaxis.which = 0;
        event.jaxis.axis = 3;
        event.jaxis.value = sev.motion.rx;
        SDL_PushEvent(&event);
        last_rx = sev.motion.rx;
      }
      if (last_ry != sev.motion.ry) {
        event.jaxis.type = SDL_JOYAXISMOTION;
        event.jaxis.timestamp = SDL_GetTicks();
        event.jaxis.which = 0;
        event.jaxis.axis = 4;
        event.jaxis.value = sev.motion.ry;
        SDL_PushEvent(&event);
        last_ry = sev.motion.ry;
      }
      if (last_rz != sev.motion.rz) {
        event.jaxis.type = SDL_JOYAXISMOTION;
        event.jaxis.timestamp = SDL_GetTicks();
        event.jaxis.which = 0;
        event.jaxis.axis = 5;
        event.jaxis.value = sev.motion.rz;
        SDL_PushEvent(&event);
        last_rz = sev.motion.rz;
      }
    } else if (sev.type == SPNAV_EVENT_BUTTON) {
      event.jbutton.type =
        sev.button.press ? SDL_JOYBUTTONDOWN : SDL_JOYBUTTONUP;
      event.jbutton.timestamp = SDL_GetTicks();
      event.jbutton.which = 0;
      event.jbutton.button = sev.button.bnum;
      event.jbutton.state = sev.button.press ? SDL_PRESSED : SDL_RELEASED;
      SDL_PushEvent(&event);
    }
  }
#endif

  while (SDL_PollEvent(&event)) {
    ui.process_event(event);
    scene.get_camera().process_event(event, dt);
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

bool
Input::should_quit() const
{
  return quit;
}
