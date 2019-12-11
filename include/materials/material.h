#pragma once

namespace Raytracer {
struct RayPayload;
class Scene;
namespace Math {
class vec3;
}
} // namespace Raytracer

namespace Raytracer::Materials {
using Raytracer::RayPayload;
using Raytracer::Scene;
using Raytracer::Math::vec3;
struct Material
{
  virtual ~Material() = default;
  virtual void fill_type_data(const Scene& scene,
                              RayPayload& payload,
                              const vec3& texture_coordinates) const = 0;
};
} // namespace Raytracer::Materials
