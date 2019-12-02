#include "camera.h"
#include <SDL_events.h>

Camera::Camera(vec3 lookfrom, vec3 lookat, vec3 vup, float vfov, float aspect)
  : look_from(lookfrom)
  , look_at(lookat)
  , v_up(vup)
  , v_fov(vfov)
  , screen_aspect(aspect)
  , dirty(true)
{
  calculate_camera();
}

Ray
Camera::get_ray(float s, float t) const
{
  auto direction = lower_left_corner + s * horizontal + t * vertical - origin;
  direction.make_unit_vector();
  return Ray(origin, direction);
}

void
Camera::set_aspect(float aspect)
{
  if (aspect != screen_aspect) {
    screen_aspect = aspect;
    calculate_camera();
  }
}

void
Camera::calculate_camera()
{
  vec3 u, v, w;
  float theta = v_fov * M_PI / 180;
  float half_height = tan(theta / 2);
  float half_width = screen_aspect * half_height;

  w = unit_vector(look_from - look_at);
  u = unit_vector(cross(v_up, w));
  v = -cross(w, u);

  lower_left_corner = origin - half_width * u - half_height * v - w;
  horizontal = 2 * half_width * u;
  vertical = 2 * half_height * v;

  dirty = true;
}

bool
Camera::is_dirty() const
{
  return dirty;
}

void
Camera::set_clean()
{
  dirty = false;
}

void
Camera::process_event(const SDL_Event& event)
{
  float speed = 0.01f;
  switch (event.type) {
    case SDL_KEYDOWN: {
      if (event.key.keysym.mod & KMOD_SHIFT) {
        speed *= 5.0f;
      } else if (event.key.keysym.mod & KMOD_CTRL) {
        speed *= 0.2f;
      }
      switch (event.key.keysym.sym) {
#if __EMSCRIPTEN__
        case SDL_SCANCODE_w: // weird sdl2 emscripten bug
#else
        case SDLK_w:
#endif
        {
          auto direction = (look_at - look_from);
          direction.make_unit_vector();
          origin += direction * speed;
          calculate_camera();
        } break;
#if __EMSCRIPTEN__
        case SDL_SCANCODE_s: // weird sdl2 emscripten bug
#else
        case SDLK_s:
#endif
        {
          auto direction = (look_at - look_from);
          direction.make_unit_vector();
          origin -= direction * speed;
          calculate_camera();
        } break;
#if __EMSCRIPTEN__
        case SDL_SCANCODE_e: // weird sdl2 emscripten bug
#else
        case SDLK_e:
#endif
          origin += v_up * speed;
          calculate_camera();
          break;
#if __EMSCRIPTEN__
        case SDL_SCANCODE_q: // weird sdl2 emscripten bug
#else
        case SDLK_q:
#endif
          origin -= v_up * speed;
          calculate_camera();
          break;
#if __EMSCRIPTEN__
        case SDL_SCANCODE_a: // weird sdl2 emscripten bug
#else
        case SDLK_a:
#endif
        {
          auto direction = cross(v_up, (look_at - look_from));
          direction.make_unit_vector();
          origin += direction * speed;
          calculate_camera();
        } break;
#if __EMSCRIPTEN__
        case SDL_SCANCODE_d: // weird sdl2 emscripten bug
#else
        case SDLK_d:
#endif
        {
          auto direction = cross(v_up, (look_at - look_from));
          direction.make_unit_vector();
          origin -= direction * speed;
          calculate_camera();
        } break;
#if __EMSCRIPTEN__
        case SDL_SCANCODE_UP: // weird sdl2 emscripten bug
#else
        case SDLK_UP:
#endif
          look_at += v_up * speed;
          calculate_camera();
          break;
#if __EMSCRIPTEN__
        case SDL_SCANCODE_DOWN: // weird sdl2 emscripten bug
#else
        case SDLK_DOWN:
#endif
          look_at -= v_up * speed;
          calculate_camera();
          break;

#if __EMSCRIPTEN__
        case SDL_SCANCODE_LEFT: // weird sdl2 emscripten bug
#else
        case SDLK_LEFT:
#endif
        {
          auto direction = cross(v_up, (look_at - look_from));
          direction.make_unit_vector();
          look_at += direction * speed;
          calculate_camera();
        } break;
#if __EMSCRIPTEN__
        case SDL_SCANCODE_RIGHT: // weird sdl2 emscripten bug
#else
        case SDLK_RIGHT:
#endif
        {
          auto direction = cross(v_up, (look_at - look_from));
          direction.make_unit_vector();
          look_at -= direction * speed;
          calculate_camera();
        } break;
      }
    }
  }
}
