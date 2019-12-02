#pragma once

#include "object.h"

#include <vector>

#include "vec2.h"
#include "vec3.h"

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
           float tmin,
           float tmax,
           hit_record& rec) const override;

  std::vector<vec3> positions;
  std::vector<MeshVertexData> vertex_data;
  std::vector<uint16_t> indices;
  uint16_t mat_id;
};
