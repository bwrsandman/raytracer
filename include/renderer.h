#pragma once

#include <chrono>

class Window;

/// Abstract Base Class of renderer
///
/// This class should only provide interface as a pure virtual functions.
/// This class should also have no member variables, only getters and setters.
/// Different renderer styles should inherit from this ABC and implement
/// the pure virtual functions.
class Renderer
{
public:
  explicit Renderer(const Window& window) {}
  virtual ~Renderer() = default;
  virtual void run(std::chrono::microseconds dt) = 0;
};
