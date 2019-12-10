#pragma once

#include "object.h"

#include <vector>

#include "math/vec2.h"
#include "math/vec3.h"

namespace Raytracer::Hittable {
using Raytracer::Math::vec2;
using Raytracer::Math::vec3;

struct MeshVertexData
{
  vec2 uv;
  vec3 normal;
  vec3 tangent;
};

struct TriangleMesh : Object
{
  TriangleMesh(std::vector<vec3>&& positions,
               std::vector<MeshVertexData>&& vertex_data,
               std::vector<uint16_t>&& indices,
               uint16_t m);
  ~TriangleMesh() override;
  bool hit(const Ray& r,
           bool early_out,
           float t_min,
           float t_max,
           hit_record& rec) const override;

  std::vector<vec3> positions;
  std::vector<MeshVertexData> vertex_data;
  std::vector<uint16_t> indices;
  uint16_t mat_id;
};
} // namespace Raytracer::Hittable
