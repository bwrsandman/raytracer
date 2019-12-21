#pragma once

#include <cassert>
#include <cstdint>

#include "aabb.h"

namespace Raytracer {
struct alignas(32) BvhNode
{
  Aabb bounds;
  /// If a leaf, this is the offset into the primitive index buffer
  /// If an internal node, this is the offset of the left-most child
  union
  {
    uint32_t index_offset;
    uint32_t left_bvh_offset;
  };
  /// This is the amount of primitive indices for leaves
  /// If count is 0, then this is an internal node
  union
  {
    uint32_t index_count;
    bool is_leaf;
  };

  uint32_t right_bvh_offset() const
  {
    assert(!is_leaf);
    return left_bvh_offset + 1;
  }
};

static_assert(sizeof(BvhNode) == 32,
              "bvh node should fit two on one 64 byte cache boundary");
} // namespace Raytracer
