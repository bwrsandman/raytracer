#pragma once

#include "object.h"

#include <vector>

#include "aabb.h"
#include "bvh.h"
#include "math/vec2.h"
#include "math/vec3.h"

namespace Raytracer::Hittable {
using Raytracer::Aabb;
using Raytracer::BvhNode;
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
  bool ray_triangles_intersect(const Ray& r,
                               const uint16_t* indices,
                               uint32_t index_count,
                               bool early_out,
                               float t_min,
                               float t_max,
                               hit_record& rec) const;
  bool bounding_box(Aabb& box);
  void build_bvh();

  std::vector<vec3> positions;
  std::vector<MeshVertexData> vertex_data;
  std::vector<uint16_t> indices;
  uint16_t mat_id;
  Aabb aabb;
  std::vector<BvhNode> bvh;
  std::vector<uint16_t> bvh_optimized_indices;
};
} // namespace Raytracer::Hittable
