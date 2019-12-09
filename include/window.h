#pragma once

#include <cstdint>
#include <string>

struct SDL_Window;

namespace Raytracer {
class Window
{
public:
  Window(std::string name, uint16_t width, uint16_t height);
  virtual ~Window();
  SDL_Window* get_native_handle() const;
  void swap() const;
  void get_dimensions(uint16_t& width, uint16_t& height);

private:
  SDL_Window* handle;
};
} // namespace Raytracer
