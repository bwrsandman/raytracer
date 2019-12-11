#include "materials/lambert.h"

#include "math/mat3.h"
#include "ray.h"
#include "scene.h"
#include "texture.h"

using Raytracer::Materials::Lambert;
using Raytracer::Math::vec3;
using Raytracer::RayPayload;
using Raytracer::Ray;

Lambert::Lambert(const vec3& a,
                 uint16_t albedo_texture_id,
                 uint16_t normal_texture_id)
  : albedo(a)
  , albedo_texture_id(albedo_texture_id)
  , normal_texture_id(normal_texture_id)
{}

void
Lambert::fill_type_data(const Scene& scene,
                        RayPayload& payload,
                        const vec3& texture_coordinates) const
{
  payload.type = RayPayload::Type::Lambert;
  payload.attenuation = albedo;
  if (albedo_texture_id != std::numeric_limits<uint16_t>::max()) {
    payload.attenuation =
      scene.get_texture(albedo_texture_id).sample(texture_coordinates);
  }
  if (normal_texture_id != std::numeric_limits<uint16_t>::max()) {
    auto bitangent = cross(payload.normal, payload.tangent);
    mat3 model_to_tangent_space_matrix = {
      payload.tangent.x(), payload.tangent.y(), payload.tangent.z(),
      bitangent.x(),       bitangent.y(),       bitangent.z(),
      payload.normal.x(),  payload.normal.y(),  payload.normal.z(),
    };

    payload.normal =
      dot(model_to_tangent_space_matrix,
          scene.get_texture(normal_texture_id).sample(texture_coordinates));
    payload.normal.make_unit_vector();
  }
}
