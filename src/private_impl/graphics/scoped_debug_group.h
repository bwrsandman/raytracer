#pragma once

#include <string_view>

namespace Raytracer::Graphics {
class ScopedDebugGroup
{
public:
  explicit ScopedDebugGroup(const std::string_view& label) noexcept;
  ~ScopedDebugGroup();
};
} // namespace Raytracer::Graphics
