#pragma once

#include <chrono>
#include <memory>

struct SDL_Window;

namespace Raytracer {
class Scene;
struct Ray;
namespace Math {
class vec3;
}
}

namespace Raytracer::Graphics {
using Raytracer::Math::vec3;

/// Abstract Base Class of renderer
///
/// This class should only provide interface as a pure virtual functions.
/// This class should also have no member variables, only getters and setters.
/// Different renderer styles should inherit from this ABC and implement
/// the pure virtual functions and add themselves to the implementation of
/// the create static function.
class Renderer
{
public:
  enum class Type
  {
    Whitted,
    Gpu,
  };
  virtual ~Renderer() = default;
  virtual void run(const Scene& world) = 0;
  virtual void set_backbuffer_size(uint16_t width, uint16_t height) = 0;

  /// Factory function from which all types of renderers can be created
  static std::unique_ptr<Renderer> create(Type type, SDL_Window* window);

  virtual bool get_debug() const = 0;
  virtual void set_debug(bool value) = 0;
  virtual void set_debug_data(uint32_t data) = 0;

protected:
  Renderer() = default;
};
} // namespace Raytracer::Graphics
