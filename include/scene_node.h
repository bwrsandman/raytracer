#pragma once

#include <memory>

#include "mat4.h"
#include "quat.h"
#include "vec3.h"

class Camera;

struct alignas(32) Transform
{
  Transform();

  mat4 matrix() const;
  mat4 inverse_matrix() const;

  vec3 translation;
  quat rotation;
  float scale;
};

static_assert(sizeof(Transform) == 32,
              "Transforms should fit 2 per cache line of 64 bytes");

struct alignas(64) SceneNode
{
  enum class Type : uint8_t
  {
    Empty,
    Mesh,
    Camera,
  };

  SceneNode();

  Transform local_trs;
  uint32_t children_id_offset;
  uint32_t children_id_length;
  Type type;
  uint32_t mesh_id;
  std::unique_ptr<Camera> camera;
};
static_assert(sizeof(SceneNode) <= 64,
              "Scene Node should fit on a cache line of 64 bytes");
