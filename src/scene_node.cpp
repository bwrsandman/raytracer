#include "scene_node.h"

#include "camera.h"

using Raytracer::SceneNode;
using Raytracer::Transform;
using Raytracer::Math::mat4;

Transform::Transform()
  : translation()
  , rotation{ 0.0f, 0.0f, 0.0f, 1.0f }
  , scale(1.0f)
{}

mat4
Transform::matrix() const
{
  mat4 t =
    mat4{ 1.0f, 0.0f, 0.0f, translation.x(), 0.0f, 1.0f, 0.0f, translation.y(),
          0.0f, 0.0f, 1.0f, translation.z(), 0.0f, 0.0f, 0.0f, 1.0f };
  mat4 r = static_cast<mat4>(rotation);
  mat4 s = mat4{ scale, 0.0f, 0.0f,  0.0f, 0.0f, scale, 0.0f, 0.0f,
                 0.0f,  0.0f, scale, 0.0f, 0.0f, 0.0f,  0.0f, 1.0f };
  return dot(t, dot(r, s));
}

mat4
Transform::inverse_matrix() const
{
  mat4 it = mat4{ 1.0f, 0.0f, 0.0f, -translation.x(),
                  0.0f, 1.0f, 0.0f, -translation.y(),
                  0.0f, 0.0f, 1.0f, -translation.z(),
                  0.0f, 0.0f, 0.0f, 1.0f };
  mat4 ir = static_cast<mat4>(rotation.inverse());
  mat4 is = mat4{ 1.0f / scale, 0.0f, 0.0f, 0.0f, 0.0f,         1.0f / scale,
                  0.0f,         0.0f, 0.0f, 0.0f, 1.0f / scale, 0.0f,
                  0.0f,         0.0f, 0.0f, 1.0f };

  return dot(is, dot(ir, it));
}

SceneNode::SceneNode()
  : local_trs()
  , children_id_offset(std::numeric_limits<uint32_t>::max())
  , children_id_length(0)
  , type(Type::Empty)
  , mesh_id(std::numeric_limits<uint32_t>::max())
  , camera()
{}
