#include "window.h"
#include <SDL.h>
#include <SDL_video.h>

using namespace Raytracer;

Window::Window(std::string name, uint16_t width, uint16_t height)
{
  SDL_Init(SDL_INIT_VIDEO);

  handle = SDL_CreateWindow(name.c_str(),
                            SDL_WINDOWPOS_UNDEFINED,
                            SDL_WINDOWPOS_UNDEFINED,
                            width,
                            height,
                            SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
}

Window::~Window()
{

  SDL_DestroyWindow(handle);
  SDL_Quit();
}

SDL_Window*
Window::get_native_handle() const
{
  return handle;
}

void
Window::swap() const
{
  SDL_GL_SwapWindow(handle);
}

void
Window::get_dimensions(uint16_t& width, uint16_t& height)
{
  int w, h;
#if _WIN32
  w = SDL_GetWindowSurface(handle)->w;
  h = SDL_GetWindowSurface(handle)->h;
#else
  SDL_GetWindowSize(handle, &w, &h);
#endif
  width = w;
  height = h;
}
