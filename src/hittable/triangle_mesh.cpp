#include "hittable/triangle_mesh.h"

#include "hit_record.h"
#include "ray.h"

TriangleMesh::TriangleMesh(std::vector<vec3>&& positions,
                           std::vector<MeshVertexData>&& vertex_data,
                           std::vector<uint16_t>&& indices,
                           uint16_t m)
  : positions(std::move(positions))
  , vertex_data(std::move(vertex_data))
  , indices(std::move(indices))
  , mat_id(m)
{}

TriangleMesh::~TriangleMesh() = default;

/// Möller–Trumbore intersection algorithm
bool
TriangleMesh::hit(const Ray& r,
                  bool early_out,
                  float t_min,
                  float t_max,
                  hit_record& rec) const
{
  bool hit_anything = false;
  double closest_so_far = t_max;
  for (uint32_t i = 0; i < indices.size() / 3; ++i) {
    auto& i0 = indices[i * 3];
    auto& i1 = indices[i * 3 + 1];
    auto& i2 = indices[i * 3 + 2];
    const auto& v0 = positions[i0];
    vec3 v0v1 = positions[i1] - v0;
    vec3 v0v2 = positions[i2] - v0;
    vec3 ray_edge_cross = cross(r.direction, v0v2);
    auto det = dot(v0v1, ray_edge_cross);
    if (std::abs(det) < std::numeric_limits<float>::epsilon()) {
      // This ray is parallel to this triangle.
      continue;
    }
    auto det_inv = 1.0f / det;
    auto v0ro = r.origin - v0;
    auto u = det_inv * dot(v0ro, ray_edge_cross);
    if (u < 0.0f || u > 1.0f) {
      continue;
    }
    auto q = cross(v0ro, v0v1);
    auto v = det_inv * dot(r.direction, q);
    if (v < 0.0f || u + v > 1.0f) {
      continue;
    }
    // At this stage we can compute t to find out where the intersection point
    // is on the line.
    float t = det_inv * dot(v0v2, q);
    if (t > std::numeric_limits<float>::epsilon() &&
        t < std::numeric_limits<float>::infinity()) {

      auto w = 1 - u - v;

      auto& uv0 = vertex_data[i0].uv;
      auto& uv1 = vertex_data[i1].uv;
      auto& uv2 = vertex_data[i2].uv;

      auto& normal0 = vertex_data[i0].normal;
      auto& normal1 = vertex_data[i1].normal;
      auto& normal2 = vertex_data[i2].normal;

      vec2 uv0uv1 = uv1 - uv0;
      vec2 uv0uv2 = uv2 - uv0;

      if (closest_so_far > t) {
        rec.t = t;
        rec.p = r.origin + r.direction * t;
        rec.normal = normal0 * w + normal1 * u + normal2 * v;
        rec.normal.make_unit_vector();
        auto denom_inv =
          1.0f / (uv0uv1.e[0] * uv0uv2.e[1] - uv0uv1.e[1] * uv0uv2.e[0]);
        rec.tangent = (v0v1 * uv0uv2.e[1] - v0v2 * uv0uv1.e[1]) * denom_inv;
        rec.tangent.make_unit_vector();
        vec2 uv = uv0 * w + uv1 * u + uv2 * v;
        rec.uv = vec3(uv.e[0], uv.e[1], 0.0f);
        rec.mat_id = mat_id;
        hit_anything = true;
        closest_so_far = t;
        if (early_out) {
          break;
        }
      }
    }
  }
  return hit_anything;
}
