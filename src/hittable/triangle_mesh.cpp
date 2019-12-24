#include "hittable/triangle_mesh.h"

#include <array>
#include <queue>

#include "hit_record.h"
#include "ray.h"

using Raytracer::Aabb;
using Raytracer::hit_record;
using Raytracer::Ray;
using Raytracer::Hittable::MeshVertexData;
using Raytracer::Hittable::TriangleMesh;
using Raytracer::Math::vec3;

TriangleMesh::TriangleMesh(std::vector<vec3>&& positions,
                           std::vector<MeshVertexData>&& vertex_data,
                           std::vector<uint16_t>&& indices,
                           uint16_t m)
  : positions(std::move(positions))
  , vertex_data(std::move(vertex_data))
  , indices(std::move(indices))
  , mat_id(m)
  , aabb()
{
  bounding_box(aabb);
}

TriangleMesh::~TriangleMesh() = default;

/// Möller–Trumbore intersection algorithm
bool
TriangleMesh::hit(const Ray& r,
                  bool early_out,
                  float t_min,
                  float t_max,
                  hit_record& rec) const
{
  if (bvh.empty()) {
    // No bvh, bruteforce all triangles
    if (!Aabb::hit(aabb, r, t_min, t_max)) {
      return false;
    }
    return ray_triangles_intersect(r,
                                   indices.data(),
                                   static_cast<uint32_t>(indices.size()),
                                   early_out,
                                   t_min,
                                   t_max,
                                   rec);
  } else {
    // Traverse bvh and find indices
    std::queue<uint32_t> nodes_to_visit;
    nodes_to_visit.emplace(0);
    double closest_so_far = t_max;
    bool hit_anything = false;
    while (!nodes_to_visit.empty()) {
      auto& node = bvh[nodes_to_visit.front()];
      if (Aabb::hit(node.bounds, r, t_min, closest_so_far)) {
        rec.bvh_hits++;
        if (node.is_leaf()) {
          hit_record temp_rec;
          if (ray_triangles_intersect(r,
                                      &bvh_optimized_indices[node.index_offset],
                                      node.index_count,
                                      early_out,
                                      t_min,
                                      closest_so_far,
                                      temp_rec)) {
            hit_anything = true;
            // TODO: This might always be true
            if (closest_so_far > temp_rec.t) {
              closest_so_far = temp_rec.t;
              rec = temp_rec;
              if (early_out) {
                break;
              }
            }
          }

        } else {
          // Add children
          assert(node.left_bvh_offset < bvh.size());
          assert(node.right_bvh_offset() < bvh.size());
          nodes_to_visit.emplace(node.left_bvh_offset);
          nodes_to_visit.emplace(node.right_bvh_offset());
        }
      }

      nodes_to_visit.pop();
    }
    return hit_anything;
  }
}

inline bool
ray_triangle_intersect(const Ray& r,
                       const vec3& v0,
                       const vec3& v1,
                       const vec3& v2,
                       float& t,
                       vec3& barycentric_coordinates)
{
  vec3 v0v1 = v1 - v0;
  vec3 v0v2 = v2 - v0;
  vec3 ray_edge_cross = cross(r.direction, v0v2);
  auto det = dot(v0v1, ray_edge_cross);
  if (std::abs(det) < std::numeric_limits<float>::epsilon()) {
    // This ray is parallel to this triangle.
    return false;
  }
  auto det_inv = 1.0f / det;
  auto v0ro = r.origin - v0;
  auto u = det_inv * dot(v0ro, ray_edge_cross);
  if (u < 0.0f || u > 1.0f) {
    return false;
  }
  auto q = cross(v0ro, v0v1);
  auto v = det_inv * dot(r.direction, q);
  if (v < 0.0f || u + v > 1.0f) {
    return false;
  }
  // At this stage we can compute t to find out where the intersection point
  // is on the line.
  t = det_inv * dot(v0v2, q);

  if (t > std::numeric_limits<float>::epsilon() &&
      t < std::numeric_limits<float>::infinity()) {
    barycentric_coordinates.e[0] = u;
    barycentric_coordinates.e[1] = v;
    barycentric_coordinates.e[2] = 1 - u - v;
    return true;
  }
  return false;
}

bool
TriangleMesh::ray_triangles_intersect(const Ray& r,
                                      const uint16_t* index_buffer,
                                      uint32_t index_count,
                                      bool early_out,
                                      float t_min,
                                      float t_max,
                                      hit_record& rec) const
{
  bool hit_anything = false;
  double closest_so_far = t_max;
  for (uint32_t i = 0; i < index_count / 3; ++i) {
    auto& i0 = index_buffer[i * 3];
    auto& i1 = index_buffer[i * 3 + 1];
    auto& i2 = index_buffer[i * 3 + 2];
    const auto& v0 = positions[i0];
    const auto& v1 = positions[i1];
    const auto& v2 = positions[i2];

    float t;
    vec3 barycentric_coordinates;
    if (!ray_triangle_intersect(r, v0, v1, v2, t, barycentric_coordinates)) {
      continue;
    }

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
      rec.normal = normal0 * barycentric_coordinates.e[2] +
                   normal1 * barycentric_coordinates.e[0] +
                   normal2 * barycentric_coordinates.e[1];
      rec.normal.make_unit_vector();
      auto denom_inv =
        1.0f / (uv0uv1.e[0] * uv0uv2.e[1] - uv0uv1.e[1] * uv0uv2.e[0]);
      vec3 v0v1 = v1 - v0;
      vec3 v0v2 = v2 - v0;
      rec.tangent = (v0v1 * uv0uv2.e[1] - v0v2 * uv0uv1.e[1]) * denom_inv;
      rec.tangent.make_unit_vector();
      vec2 uv = uv0 * barycentric_coordinates.e[2] +
                uv1 * barycentric_coordinates.e[0] +
                uv2 * barycentric_coordinates.e[1];
      rec.uv = vec3(uv.e[0], uv.e[1], 0.0f);
      rec.mat_id = mat_id;
      hit_anything = true;
      closest_so_far = t;
      if (early_out) {
        break;
      }
    }
  }
  return hit_anything;
}

bool
TriangleMesh::bounding_box(Aabb& box)
{
  vec3 min = positions[0], max = positions[0];
  float min_x, min_y, min_z;

  for (int i = 1; i < positions.size(); i++) {
    for (int j = 0; j < 3; j++) {
      if (min.e[j] > positions[i].e[j])
        min.e[j] = positions[i].e[j];

	  if (max.e[j] < positions[i].e[j])
        max.e[j] = positions[i].e[j];
    }
  }

  box = Aabb{ min, max };
  return true;
}

template<uint8_t num_bins, uint32_t max_size, uint32_t cost_of_split>
inline bool
split_binned_sah(const std::vector<uint16_t>& indices,
                 const std::vector<vec3>& centroids,
                 const std::vector<Aabb>& bounding_boxes,
                 std::vector<uint16_t>& left_indices,
                 std::vector<uint16_t>& right_indices,
                 Aabb& left_bb,
                 Aabb& right_bb)
{
  if (indices.size() <= max_size) {
    return false;
  }
  // Compute the bounds for all objects/triangles as well as the bounds for
  // all centroids
  Aabb centroid_bb{ std::numeric_limits<vec3>::infinity(),
                    -std::numeric_limits<vec3>::infinity() };

  for (uint16_t i : indices) {
    centroid_bb.min = std::min(centroid_bb.min, centroids[i]);
    centroid_bb.max = std::max(centroid_bb.max, centroids[i]);
  }
  // Split into bins
  // k1 = (K * (1-epsilon)) / (centroid_bb.max.e[k] - centroid_bb.min.e[k])
  // k0 = int(centroid_bb.min.e[k])
  // bin_id[i] = k1 * (centroids[i].e[k] - k0)
  // insert object to bin and grow the bb
  thread_local std::array<std::vector<uint16_t>, num_bins> bins;
  thread_local std::array<Aabb, num_bins> bin_aabbs;
  for (uint8_t i = 0; i < num_bins; ++i) {
    bins[i].clear();
    bins[i].reserve(indices.size());
    bin_aabbs[i].min = std::numeric_limits<vec3>::infinity();
    bin_aabbs[i].max = -std::numeric_limits<vec3>::infinity();
  }
  {
    auto centroid_bb_size = centroid_bb.max - centroid_bb.min;
    uint8_t major_axis = centroid_bb_size.major_axis();

    auto k0 = centroid_bb.min.e[major_axis];
    auto k1 = (num_bins * (1.0f - std::numeric_limits<float>::epsilon())) /
              centroid_bb_size.e[major_axis];
    auto k2 = k0 * k1;
    for (uint16_t i : indices) {
      float bin_id_f32 = k1 * centroids[i].e[major_axis] - k2;
      // float-to-int conversion
      uint8_t bin_id =
        std::clamp(bin_id_f32, 0.0f, static_cast<float>(num_bins) - 1);
      bins[bin_id].push_back(i);
      bin_aabbs[bin_id].min =
        std::min(bin_aabbs[bin_id].min, bounding_boxes[i].min);
      bin_aabbs[bin_id].max =
        std::max(bin_aabbs[bin_id].max, bounding_boxes[i].max);
    }
  }

  // Calculate bin area and sizes for cost analysis
  thread_local std::array<uint32_t, num_bins> bin_sizes;
  thread_local std::array<float, num_bins> bin_areas;
  float total_bin_area = 0.0f;
  for (uint32_t i = 0; i < num_bins; ++i) {
    bin_sizes[i] = bins[i].size();
    auto bin_size = bin_aabbs[i].max - bin_aabbs[i].min;
    // TODO: the times 2 can probably be removed
    if (bin_sizes[i] == 0) {
      bin_areas[i] = 0;
    } else {
      bin_areas[i] =
        2 * (bin_size.x() * bin_size.y() + bin_size.x() * bin_size.z() +
             bin_size.y() * bin_size.z());
    }
    total_bin_area += bin_areas[i];
  }

  // Select bin to split
  uint8_t best_index = 0;
  {
    // TODO: can we pre-multiply area and count?
    // TODO: can we quick-select? 4 steps for 16 bins
    uint32_t left_count = 0;
    uint32_t right_count = indices.size();
    float left_area = 0.0f;
    float right_area = total_bin_area;
    float best_cost = indices.size() * total_bin_area;
    for (uint8_t i = 0; i < num_bins - 1; ++i) {
      auto count = bin_sizes[i];
      if (count == 0) {
        continue;
      }
      auto area = bin_areas[i];
      left_area += area;
      right_area -= area;
      left_count += count;
      right_count -= count;

      auto cost = left_area * left_count + right_area * right_count;
      if (best_cost > cost) {
        best_cost = cost;
        best_index = i + 1;
        // TODO: store best left and right areas
      } else {
        // TODO terminate?
      }
    }
    if (best_cost + cost_of_split > indices.size() * total_bin_area) {
      return false;
    }
  }
  for (uint8_t i = 0; i < best_index; ++i) {
    left_indices.insert(left_indices.cend(), bins[i].cbegin(), bins[i].cend());
    left_bb.min = std::min(left_bb.min, bin_aabbs[i].min);
    left_bb.max = std::max(left_bb.max, bin_aabbs[i].max);
  }
  for (uint8_t i = best_index; i < num_bins; ++i) {
    right_indices.insert(
      right_indices.cend(), bins[i].cbegin(), bins[i].cend());
    right_bb.min = std::min(right_bb.min, bin_aabbs[i].min);
    right_bb.max = std::max(right_bb.max, bin_aabbs[i].max);
  }

  return true;
}

void
Raytracer::Hittable::TriangleMesh::build_bvh()
{
  bvh_optimized_indices.clear();
  bvh_optimized_indices.reserve(indices.size());
  // TODO: reserve estimated amount of nodes
  bvh.clear();

  // queue-base recursion replacement
  struct workload_params_t
  {
    std::vector<uint16_t> indices;
    Aabb mesh_bb;
    uint32_t parent_index = std::numeric_limits<uint32_t>::max();
  };
  std::queue<workload_params_t> workload;

  // Compute each object/triangle bounding box as well as the centroid.
  const uint16_t triangle_count = indices.size() / 3;
  std::vector<vec3> centroids(triangle_count);
  std::vector<Aabb> triangle_bbs(triangle_count);
  std::vector<uint16_t> root_triangle_indices(triangle_count);
  Aabb root_bb{ std::numeric_limits<vec3>::infinity(),
                -std::numeric_limits<vec3>::infinity() };

  for (uint16_t i = 0; i < triangle_count; ++i) {
    centroids[i] = (positions[indices[i * 3]] + positions[indices[i * 3 + 1]] +
                    positions[indices[i * 3 + 2]]) /
                   3.0f;
    triangle_bbs[i] = Aabb{ std::min(std::min(positions[indices[i * 3]],
                                              positions[indices[i * 3 + 1]]),
                                     positions[indices[i * 3 + 2]]),
                            std::max(std::max(positions[indices[i * 3]],
                                              positions[indices[i * 3 + 1]]),
                                     positions[indices[i * 3 + 2]]) };

    // Make sure there is no 0 volume bb
    triangle_bbs[i].max =
      std::max(triangle_bbs[i].max,
               triangle_bbs[i].min + 10 * std::numeric_limits<vec3>::epsilon());

    root_bb.min = std::min(root_bb.min, triangle_bbs[i].min);
    root_bb.max = std::max(root_bb.max, triangle_bbs[i].max);

    root_triangle_indices[i] = i;
  }

  workload.emplace(workload_params_t{ root_triangle_indices, root_bb });

  // TODO: pre-count and reserve
  thread_local std::vector<uint16_t> left_children;
  thread_local std::vector<uint16_t> right_children;
  while (!workload.empty()) {
    auto& params = workload.front();

    left_children.clear();
    right_children.clear();
    constexpr uint8_t num_bins = 16;
    constexpr uint32_t max_child_size = 4;
    constexpr uint32_t cost_of_split = 1;
    Aabb left_bb{ std::numeric_limits<vec3>::infinity(),
                  -std::numeric_limits<vec3>::infinity() };
    Aabb right_bb{ std::numeric_limits<vec3>::infinity(),
                   -std::numeric_limits<vec3>::infinity() };
    bool make_children =
      split_binned_sah<num_bins, max_child_size, cost_of_split>(params.indices,
                                                                centroids,
                                                                triangle_bbs,
                                                                left_children,
                                                                right_children,
                                                                left_bb,
                                                                right_bb);

    // Create bvh node
    if (make_children) {
      // emplace internal node with child id not yet resolved.
      // when loop gets to child, it must set the child id to its own index
      bvh.emplace_back(
        BvhNode{ params.mesh_bb, std::numeric_limits<uint32_t>::max(), 0 });
      workload.emplace(
        workload_params_t{ left_children,
                           left_bb,
                           // left child must later set its id to the parent
                           static_cast<uint32_t>(bvh.size() - 1) });
      workload.emplace(workload_params_t{ right_children, right_bb });
    } else {
      bvh.emplace_back(
        BvhNode{ params.mesh_bb,
                 static_cast<uint32_t>(bvh_optimized_indices.size()),
                 static_cast<uint32_t>(params.indices.size() * 3) });
      for (uint32_t i : params.indices) {
        bvh_optimized_indices.emplace_back(indices[i * 3]);
        bvh_optimized_indices.emplace_back(indices[i * 3 + 1]);
        bvh_optimized_indices.emplace_back(indices[i * 3 + 2]);
      }
    }

    // Set id to parent if requested (only for left children)
    if (params.parent_index != std::numeric_limits<uint32_t>::max()) {
      bvh[params.parent_index].left_bvh_offset = bvh.size() - 1;
    }

    workload.pop();
  }

  // TODO: shorten bvh
}
