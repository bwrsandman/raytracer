#include "window.h"
#include <SDL.h>

Window::Window(std::string name, uint16_t width, uint16_t height) {
	SDL_Init(SDL_INIT_VIDEO);

	handle = SDL_CreateWindow(name.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_OPENGL);
}

Window::~Window() {

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
