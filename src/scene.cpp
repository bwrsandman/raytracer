#include "scene.h"

#define _USE_MATH_DEFINES
#include <cassert>
#include <math.h>
#include <sdf.h>

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_USE_CPP14
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include <materials/emissive.h>
#include <queue>
#include <tiny_gltf.h>

#include "camera.h"
#include "hittable/functional_geometry.h"
#include "hittable/instance.h"
#include "hittable/line_segment.h"
#include "hittable/plane.h"
#include "hittable/point.h"
#include "hittable/sphere.h"
#include "hittable/triangle_mesh.h"
#include "materials/dielectric.h"
#include "materials/emissive_linear_drop_off.h"
#include "materials/emissive_quadratic_drop_off.h"
#include "materials/lambert.h"
#include "materials/metal.h"
#include "math/vec4.h"
#include "scene_node.h"
#include "texture.h"

using namespace Raytracer::Hittable;
using namespace Raytracer::Materials;
using namespace Raytracer;

namespace __details {
template<typename dstT,
         typename srcT,
         typename = std::enable_if<std::is_same<dstT, srcT>::type>>
void
copy_buffer_view(dstT* dst, const uint8_t* src, size_t count)
{
  for (size_t j = 0; j < count; ++j) {
    dst[j] = static_cast<dstT>(reinterpret_cast<const srcT*>(src)[j]);
  }
}

template<typename dstT, typename srcT>
void
copy_buffer_view(dstT* dst, const uint8_t* src, size_t count)
{
  memcpy(dst, src, count * sizeof(dstT));
}
} // namespace __details

template<typename desT>
void
copy_buffer_view(desT* dst,
                 const uint8_t* src,
                 size_t count,
                 uint32_t componentType)
{
  switch (componentType) {
    case TINYGLTF_COMPONENT_TYPE_BYTE:
      __details::copy_buffer_view<desT, int8_t>(dst, src, count);
      break;
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
      __details::copy_buffer_view<desT, uint8_t>(dst, src, count);
      break;
    case TINYGLTF_COMPONENT_TYPE_SHORT:
      __details::copy_buffer_view<desT, int16_t>(dst, src, count);
      break;
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
      __details::copy_buffer_view<desT, uint16_t>(dst, src, count);
      break;
    case TINYGLTF_COMPONENT_TYPE_INT:
      __details::copy_buffer_view<desT, int32_t>(dst, src, count);
      break;
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
      __details::copy_buffer_view<desT, uint32_t>(dst, src, count);
      break;
    case TINYGLTF_COMPONENT_TYPE_FLOAT:
      __details::copy_buffer_view<desT, float>(dst, src, count);
      break;
    case TINYGLTF_COMPONENT_TYPE_DOUBLE:
      __details::copy_buffer_view<desT, double>(dst, src, count);
      break;
    default:
      // "Unsupported component type"
      assert(false);
  }
}

std::unique_ptr<Scene>
Scene::load_from_gltf(const std::string& file_name)
{
  tinygltf::TinyGLTF loader;
  tinygltf::Model gltf;
  std::string err;
  std::string warn;
  bool ret = loader.LoadASCIIFromFile(
    &gltf,
    &err,
    &warn,
    file_name,
    tinygltf::REQUIRE_VERSION | tinygltf::REQUIRE_ACCESSORS |
      tinygltf::REQUIRE_BUFFERS | tinygltf::REQUIRE_BUFFER_VIEWS);

  if (!warn.empty()) {
    std::cerr << "Warn: " << warn << std::endl;
  }

  if (!err.empty()) {
    std::cerr << "Err: " << err << std::endl;
    return nullptr;
  }

  if (!ret) {
    std::cerr << "Err: Failed to parse glTF." << std::endl;
    return nullptr;
  }

  if (gltf.meshes.empty()) {
    std::cerr << "Err: There are no meshes in glTF." << std::endl;
    return nullptr;
  }

  if (gltf.defaultScene < 0) {
    std::cerr << "Err: No default scene specified in glTF." << std::endl;
    return nullptr;
  }

  std::vector<std::unique_ptr<Texture>> textures;
  std::vector<std::unique_ptr<Material>> materials;
  std::vector<std::unique_ptr<Object>> meshes;
  bool found_camera = false;
  bool use_default_material = false;

  if (gltf.materials.empty()) {
    std::cerr
      << "Warn: No textures or materials from glTF, using default lambert."
      << std::endl;
    textures.emplace_back(Texture::load_from_file("whitted_floor.png")); // 0
    materials.emplace_back(
      std::make_unique<Lambert>(vec3(1.0, 1.0, 1.0), 0)); // 0
    use_default_material = true;
  } else {
    for (auto& t : gltf.textures) {
      if (!t.name.empty()) {
        // std::printf("glTF loader: Loading texture %s\n", t.name.c_str());
      }
      auto& image = gltf.images[t.source];
      if (!image.name.empty()) {
        // std::printf("glTF loader: Loading image %s\n", t.name.c_str());
      }
      textures.emplace_back(Texture::load_from_gltf_image(image));
    }
    for (auto& m : gltf.materials) {
      if (!m.name.empty()) {
        // std::printf("glTF loader: Loading material %s\n", m.name.c_str());
      }
      auto color_texture = std::numeric_limits<uint32_t>::max();
      auto normal_texture = std::numeric_limits<uint32_t>::max();
      if (m.pbrMetallicRoughness.baseColorTexture.index >= 0) {
        color_texture = m.pbrMetallicRoughness.baseColorTexture.index;
      }
      if (m.normalTexture.index >= 0) {
        normal_texture = m.normalTexture.index;
      }
      //      if (m.pbrMetallicRoughness.metallicFactor > 0.5f) {
      //        materials.emplace_back(std::make_unique<Metal>(
      //          vec3(1.0, 1.0, 1.0), color_texture, normal_texture));
      //      } else {
      materials.emplace_back(std::make_unique<Lambert>(
        vec3(1.0, 1.0, 1.0), color_texture, normal_texture));
      //      }
    }
  }

  bool found_lights = false;
  std::vector<std::unique_ptr<Object>> light_list;

  // Construct scene graph
  std::vector<SceneNode> nodes;
  uint32_t camera_index = 0;

  // BFS on children to have children contiguous
  std::queue<std::pair<int, std::vector<int>>> bfs_gltf_nodes_queue;
  std::vector<std::pair<int, std::vector<int>>> bfs_gltf_nodes;
  for (auto gltf_node_index : gltf.scenes[gltf.defaultScene].nodes) {
    bfs_gltf_nodes_queue.push(
      std::make_pair(gltf_node_index, std::vector<int>()));
  }
  while (!bfs_gltf_nodes_queue.empty()) {
    auto [v, p] = bfs_gltf_nodes_queue.front();
    bfs_gltf_nodes_queue.pop();
    for (auto c : gltf.nodes[v].children) {
      auto new_p = p;
      new_p.push_back(v);
      bfs_gltf_nodes_queue.emplace(c, std::move(new_p));
    }
    bfs_gltf_nodes.emplace_back(v, p);
  }

  for (auto [i, ancestor_ids] : bfs_gltf_nodes) {

    mat4 matrix;
    // TODO Full scene graph is necessary to do this properly
    ancestor_ids.push_back(i);
    for (auto p : ancestor_ids) {
      auto& parent_node = gltf.nodes[p];
      if (!parent_node.matrix.empty()) {
        matrix = dot(matrix,
                     mat4(parent_node.matrix[0],
                          parent_node.matrix[4],
                          parent_node.matrix[8],
                          parent_node.matrix[12],
                          parent_node.matrix[1],
                          parent_node.matrix[5],
                          parent_node.matrix[9],
                          parent_node.matrix[13],
                          parent_node.matrix[2],
                          parent_node.matrix[6],
                          parent_node.matrix[10],
                          parent_node.matrix[14],
                          parent_node.matrix[3],
                          parent_node.matrix[7],
                          parent_node.matrix[11],
                          parent_node.matrix[15]));
      } else {
        Transform transform;
        if (!parent_node.translation.empty()) {
          transform.translation = vec3(parent_node.translation[0],
                                       parent_node.translation[1],
                                       parent_node.translation[2]);
        }
        if (!parent_node.rotation.empty()) {
          transform.rotation =
            quat{ static_cast<float>(parent_node.rotation[0]),
                  static_cast<float>(parent_node.rotation[1]),
                  static_cast<float>(parent_node.rotation[2]),
                  static_cast<float>(parent_node.rotation[3]) };
        }
        if (!parent_node.scale.empty()) {
          transform.scale = (parent_node.scale[0] + parent_node.scale[1] +
                             parent_node.scale[2]) /
                            3.0f;
        }
        matrix = dot(matrix, transform.matrix());
      }
    }

    auto& gltf_node = gltf.nodes[i];
    auto& node = nodes.emplace_back();
    if (gltf_node.translation.size() == 3) {
      node.local_trs.translation.e[0] = gltf_node.translation[0];
      node.local_trs.translation.e[1] = gltf_node.translation[1];
      node.local_trs.translation.e[2] = gltf_node.translation[2];
    } else {
      node.local_trs.translation = vec3();
    }

    if (gltf_node.rotation.size() == 4) {
      node.local_trs.rotation.x = gltf_node.rotation[0];
      node.local_trs.rotation.y = gltf_node.rotation[1];
      node.local_trs.rotation.z = gltf_node.rotation[2];
      node.local_trs.rotation.w = gltf_node.rotation[3];
    } else {
      node.local_trs.rotation = quat();
    }
    if (gltf_node.scale.size() == 3) {
      node.local_trs.scale =
        (gltf_node.scale[0] + gltf_node.scale[1] + gltf_node.scale[2]) / 3.0f;
    } else {
      node.local_trs.scale = 1;
    }
    if (gltf_node.children.size()) {
      auto children_id_offset = 0;
      for (auto j : bfs_gltf_nodes) // TODO: Find a linear time way to do this
      {
        if (j.first == gltf_node.children[j.first]) {
          node.children_id_offset = children_id_offset;
          break;
        }
        children_id_offset++;
      }
      assert(node.children_id_offset > 0);
    }
    node.children_id_length = gltf_node.children.size();
    if (gltf_node.camera >= 0 &&
        gltf.cameras[gltf_node.camera].type == "perspective" && !found_camera) {
      node.type = SceneNode::Type::Camera;

      auto origin4 = dot(matrix, vec4(0, 0, 0, 1));
      auto direction4 = dot(matrix, vec4(0, 0, -1, 0));
      auto origin = vec3(origin4.e[0], origin4.e[1], origin4.e[2]);
      auto direction = vec3(direction4.e[0], direction4.e[1], direction4.e[2]);
      direction.make_unit_vector();
      auto up = vec3(0, 1, 0);
      auto& gltf_camera = gltf.cameras[gltf_node.camera].perspective;
      node.camera = std::make_unique<Camera>(origin,
                                             direction,
                                             up,
                                             180.0f * gltf_camera.yfov / M_PI,
                                             gltf_camera.aspectRatio);
      camera_index = nodes.size() - 1;
      found_camera = true;
    } else if (gltf_node.extensions.count("KHR_lights_punctual") > 0) {
      int light_id =
        gltf_node.extensions["KHR_lights_punctual"].GetNumberAsInt();
      if (light_id >= 0) {
        auto& light = gltf.lights[light_id];

        vec4 position = dot(matrix, vec4(0, 0, 0, 1));

        materials.emplace_back(std::make_unique<EmissiveQuadraticDropOff>(
          light.intensity *
            vec3(light.color[0], light.color[1], light.color[2]),
          1.0f));
        light_list.emplace_back(std::make_unique<Point>(
          vec3(position.e[0], position.e[1], position.e[2]),
          materials.size() - 1));

        found_lights = true;
      }
    } else if (gltf_node.mesh >= 0) {
      auto& gltf_mesh = gltf.meshes[gltf_node.mesh];
      // TODO Support multiple primitives
      if (!gltf_mesh.primitives.empty()) {
        node.type = SceneNode::Type::Mesh;
        node.mesh_id = meshes.size();

        for (auto& gltf_primitive : gltf_mesh.primitives) {
          // Indices
          std::vector<uint16_t> indices;
          {
            auto& accessor = gltf.accessors[gltf_primitive.indices];
            auto& buffer_view = gltf.bufferViews[accessor.bufferView];
            auto& buffer = gltf.buffers[buffer_view.buffer];
            indices.resize(accessor.count);
            auto offset = buffer_view.byteOffset + accessor.byteOffset;
            copy_buffer_view(indices.data(),
                             buffer.data.data() + offset,
                             indices.size(),
                             accessor.componentType);
          }

          // Positions
          std::vector<vec3> positions;
          {
            auto& accessor =
              gltf.accessors[gltf_primitive.attributes["POSITION"]];
            assert(accessor.type == TINYGLTF_TYPE_VEC3);
            auto& buffer_view = gltf.bufferViews[accessor.bufferView];
            auto& buffer = gltf.buffers[buffer_view.buffer];
            positions.resize(accessor.count);
            auto offset = buffer_view.byteOffset + accessor.byteOffset;
            copy_buffer_view(positions.data(),
                             buffer.data.data() + offset,
                             positions.size(),
                             accessor.componentType);
          }

          // UV
          std::vector<vec2> uv;
          {
            auto& accessor =
              gltf.accessors[gltf_primitive.attributes["TEXCOORD_0"]];
            assert(accessor.type == TINYGLTF_TYPE_VEC2);
            auto& buffer_view = gltf.bufferViews[accessor.bufferView];
            auto& buffer = gltf.buffers[buffer_view.buffer];
            uv.resize(accessor.count);
            auto offset = buffer_view.byteOffset + accessor.byteOffset;
            copy_buffer_view(uv.data(),
                             buffer.data.data() + offset,
                             uv.size(),
                             accessor.componentType);
          }
          // Normals
          std::vector<vec3> normals;
          {
            auto& accessor =
              gltf.accessors[gltf_primitive.attributes["NORMAL"]];
            assert(accessor.type == TINYGLTF_TYPE_VEC3);
            auto& buffer_view = gltf.bufferViews[accessor.bufferView];
            auto& buffer = gltf.buffers[buffer_view.buffer];
            normals.resize(accessor.count);
            auto offset = buffer_view.byteOffset + accessor.byteOffset;
            copy_buffer_view(normals.data(),
                             buffer.data.data() + offset,
                             normals.size(),
                             accessor.componentType);
          }

          assert(uv.size() == normals.size());
          std::vector<MeshVertexData> data;
          data.resize(uv.size());
          for (uint32_t j = 0; j < uv.size(); ++j) {
            data[j].uv = uv[j];
            data[j].normal = normals[j];
            data[j].tangent =
              cross(data[j].normal,
                    vec3(0, 1, 0)); // FIXME: This is not a good approximation
          }

          for (auto& p : positions) {
            auto moved = dot(matrix, vec4(p.e[0], p.e[1], p.e[2], 1.0f));
            p.e[0] = moved.e[0];
            p.e[1] = moved.e[1];
            p.e[2] = moved.e[2];
          }

          uint32_t material = 0;
          if (gltf_primitive.material >= 0) {
            material = static_cast<uint32_t>(gltf_primitive.material);
          }
          auto mesh = std::make_unique<TriangleMesh>(std::move(positions),
                                                     std::move(data),
                                                     std::move(indices),
                                                     material);
          mesh->build_bvh();
          meshes.emplace_back(std::move(mesh));
        }
      }
    }
  }

  // Default light
  if (!found_lights) {
    // Lights
    // std::cerr << "Warn: No lights from glTF, using default point light."
    //           << std::endl;
    materials.emplace_back(
      std::make_unique<Emissive>(vec3(1.0, 1.0, 1.0))); // 1
    light_list.emplace_back(std::make_unique<Point>(vec3(5000.0f, 0, 0), 1));
  }

  // Default camera
  if (!found_camera) {
    // std::cerr
    //   << "Warn: There are no perspective cameras in glTF, using a default
    //   one."
    //   << std::endl;
    camera_index = nodes.size();
    SceneNode& camera_node = nodes.emplace_back();
    camera_node.camera = std::make_unique<Camera>(
      vec3(0, 0.0f, 2.5f), vec3(0, 0, -1), vec3(0, 1, 0), 90, 1);
    camera_node.type = SceneNode::Type::Camera;
  }

  // std::printf("glTF loader: Loaded %zu nodes, %zu textures, %zu materials,
  // %zu "
  //             "meshes, %zu lights\n",
  //             nodes.size(),
  //             textures.size(),
  //             materials.size(),
  //             meshes.size(),
  //             light_list.size());

  // TODO: remember to apply scene graph transforms on objects and camera
  return std::unique_ptr<Scene>(new Scene(std::move(nodes),
                                          camera_index,
                                          std::move(textures),
                                          std::move(materials),
                                          std::move(meshes),
                                          std::move(light_list),
                                          0.01f,
                                          10));
}

std::unique_ptr<Scene>
Scene::load_whitted_scene()
{
  std::vector<std::unique_ptr<Texture>> textures;
  textures.emplace_back(Texture::load_from_file("whitted_floor.png")); // 0
  std::vector<std::unique_ptr<Material>> materials;
  materials.emplace_back(
    std::make_unique<Lambert>(vec3(1.0, 1.0, 1.0), 0));                 // 0
  materials.emplace_back(std::make_unique<Metal>(vec3(0.8, 0.8, 0.8))); // 1
  materials.emplace_back(
    std::make_unique<Dielectric>(vec3(0.f, 0.f, 0.f) ,1.5f, 1.0f)); // 2
  materials.emplace_back(std::make_unique<EmissiveQuadraticDropOff>(
    vec3(2000.0, 2000.0, 2000.0), 1.0f)); // 3

  std::vector<std::unique_ptr<Object>> list;
  list.emplace_back(std::make_unique<Plane>(vec3(-5.0f, -2.0f, -5.0f),
                                            vec3(5.0f, -2.0f, 5.0f),
                                            vec3(0.f, 1.f, 0.f),
                                            0));

  list.emplace_back(std::make_unique<Sphere>(vec3(0, -0.8f, -2.5f), 1.0, 1));
  vec3 bubble_center(-0.75f, 0.5f, -1.0f);
  list.emplace_back(std::make_unique<Sphere>(bubble_center, 1.0f, 2));
  list.emplace_back(std::make_unique<Sphere>(bubble_center, -0.95f, 2));

  // Construct scene graph
  std::vector<SceneNode> nodes;
  // Root node, only parent in graph
  SceneNode& root_node = nodes.emplace_back();
  root_node.children_id_offset = nodes.size();
  // Camera and camera node
  SceneNode& camera_node = nodes.emplace_back();
  camera_node.camera = std::make_unique<Camera>(
    vec3(1, 0, 2.0f), vec3(0, 0, -1), vec3(0, 1, 0), 90, 1);
  camera_node.type = SceneNode::Type::Camera;
  root_node.children_id_length++;

  // Lights
  std::vector<std::unique_ptr<Object>> light_list;
  light_list.emplace_back(std::make_unique<Point>(vec3(0, 50.0f, 0), 3));

  return std::unique_ptr<Scene>(new Scene(std::move(nodes),
                                          1,
                                          std::move(textures),
                                          std::move(materials),
                                          std::move(list),
                                          std::move(light_list),
                                          0.001f,
                                          16));
}

std::unique_ptr<Scene>
Scene::load_cornell_box()
{
  std::vector<std::unique_ptr<Texture>> textures;
  std::vector<std::unique_ptr<Material>> materials;

  textures.emplace_back(Texture::load_from_file("earth_albedo.jpg"));     // 0
  textures.emplace_back(Texture::load_from_file("earth_normal_map.tga")); // 1

  materials.emplace_back(
    std::make_unique<Lambert>(vec3(1.0, 1.0, 1.0), 0, 1));                 // 0
  materials.emplace_back(std::make_unique<Lambert>(vec3(0.6, 0.6, 0.6))); // 1
  materials.emplace_back(std::make_unique<Metal>(
    vec3(0.8, 0.6, 0.2), std::numeric_limits<uint16_t>::max(), 1));     // 2
  materials.emplace_back(std::make_unique<Metal>(vec3(0.8, 0.8, 0.8))); // 3
  materials.emplace_back(
    std::make_unique<EmissiveQuadraticDropOff>(vec3(0.8, 0, 0), 1.0f)); // 4
  materials.emplace_back(
    std::make_unique<EmissiveQuadraticDropOff>(vec3(0, 0, 0.8), 1.0f)); // 5
  materials.emplace_back(
    std::make_unique<EmissiveQuadraticDropOff>(vec3(2.0, 2.0, 2.0), 1.0f)); // 6
  materials.emplace_back(
    std::make_unique<Dielectric>(vec3(0.0, 2.0, 2.0), 1.5f, 1.0f));       // 7
  materials.emplace_back(std::make_unique<Lambert>(vec3(1.0, 0.0, 0.0))); // 8
  materials.emplace_back(std::make_unique<Lambert>(vec3(0.0, 0.5, 1.0))); // 9

  std::vector<std::unique_ptr<Object>> list;
  list.emplace_back(std::make_unique<Sphere>(vec3(0, -0.5, -2), 0.5, 0));
  list.emplace_back(std::make_unique<Sphere>(vec3(-1.5, -0.5, -2.1), 0.5, 7));
  list.emplace_back(std::make_unique<Sphere>(vec3(-1.5, -0.5, -2.1), -0.45, 7));

  // Outside box
  list.emplace_back(std::make_unique<Plane>(vec3(-2.6f, -1.5f, -4.0f),
                                            vec3(2.6f, 4.0f, -4.0f),
                                            vec3(0.f, 0.f, 1.f),
                                            1));

  list.emplace_back(std::make_unique<Plane>(
    vec3(2.5f, -1.5f, -4.0f), vec3(2.5f, 4.0f, 0.0f), vec3(-1.f, 0.f, 0.f), 8));

  list.emplace_back(std::make_unique<Plane>(vec3(-2.5f, -1.5f, -4.0f),
                                            vec3(-2.5f, 4.0f, 0.0f),
                                            vec3(1.f, 0.f, 0.f),
                                            9));

  list.emplace_back(std::make_unique<Plane>(
    vec3(-2.6, -1.5f, -4.f), vec3(2.6, -1.5f, 0.0), vec3(0.f, 1.f, 0.f), 1));

  // small box rotated
  vec3 translation(1.5f, -0.5f, -1.1f);
  list.emplace_back(
    std::make_unique<Translate>(new Rotate_y(new Plane(vec3(0.f, -0.5f, -1.25f),
                                                       vec3(0.f, 0.5f, -.75f),
                                                       vec3(-1.f, 0.f, 0.f),
                                                       3),
                                             20),
                                translation));

  list.emplace_back(
    std::make_unique<Translate>(new Rotate_y(new Plane(vec3(1.f, -0.5f, -1.25f),
                                                       vec3(1.f, 0.5f, -.75f),
                                                       vec3(1.f, 0.f, 0.f),
                                                       3),
                                             20),
                                translation));

  list.emplace_back(
    std::make_unique<Translate>(new Rotate_y(new Plane(vec3(0.f, -0.5f, -1.25f),
                                                       vec3(1.f, -0.5f, -.75f),
                                                       vec3(0.f, -1.f, 0.f),
                                                       3),
                                             20),
                                translation));

  list.emplace_back(
    std::make_unique<Translate>(new Rotate_y(new Plane(vec3(0.f, 0.5f, -1.25f),
                                                       vec3(1.f, 0.5f, -.75f),
                                                       vec3(0.f, 1.f, 0.f),
                                                       3),
                                             20),
                                translation));

  list.emplace_back(
    std::make_unique<Translate>(new Rotate_y(new Plane(vec3(0.f, -0.5f, -1.25f),
                                                       vec3(1.f, 0.5f, -1.25f),
                                                       vec3(0.f, 0.f, -1.f),
                                                       3),
                                             20),
                                translation));

  list.emplace_back(
    std::make_unique<Translate>(new Rotate_y(new Plane(vec3(0.f, -0.5f, -.75f),
                                                       vec3(1.f, 0.5f, -.75f),
                                                       vec3(0.f, 0.f, 1.f),
                                                       3),
                                             20),
                                translation));

  std::vector<std::unique_ptr<Object>> light_list;
  light_list.emplace_back(std::make_unique<Point>(vec3(1, 1.5, -2), 4));
  light_list.emplace_back(std::make_unique<Point>(vec3(-1, 1.5, -2), 5));
  light_list.emplace_back(std::make_unique<Point>(vec3(0, 2, -1.5), 6));

  // Construct scene graph
  std::vector<SceneNode> nodes;
  // Root node, only parent in graph
  SceneNode& root_node = nodes.emplace_back();
  root_node.children_id_offset = nodes.size();
  // Camera and camera node
  SceneNode& camera_node = nodes.emplace_back();
  camera_node.camera = std::make_unique<Camera>(
    vec3(0, 0, 0), vec3(0, 0, -1), vec3(0, 1, 0), 90, 1);
  camera_node.type = SceneNode::Type::Camera;
  root_node.children_id_length++;
  // Meshes
  for (uint32_t i = 0; i < list.size(); ++i) {
    auto& node = nodes.emplace_back();
    node.type = SceneNode::Type::Mesh;
    node.mesh_id = i;
    root_node.children_id_length++;
  }

  return std::unique_ptr<Scene>(new Scene(std::move(nodes),
                                          1,
                                          std::move(textures),
                                          std::move(materials),
                                          std::move(list),
                                          std::move(light_list),
                                          0.001f,
                                          20));
}

std::unique_ptr<Scene>
Scene::load_mandrelbulb()
{
  std::vector<std::unique_ptr<Texture>> textures;
  std::vector<std::unique_ptr<Material>> materials;
  materials.emplace_back(std::make_unique<Lambert>(
    vec3(0.235294118, 0.701960784, 0.443137255))); // 0
  materials.emplace_back(std::make_unique<EmissiveQuadraticDropOff>(
    vec3(1000.0f, 1000.0f, 1000.0f), 1.0f));                            // 1
  materials.emplace_back(std::make_unique<Metal>(vec3(0.8, 0.6, 0.2))); // 2

  std::vector<std::unique_ptr<Object>> list;
  list.emplace_back(
    FunctionalGeometry::mandrelbulb(vec3(0.0f, 0.0f, -1), 10, 5.0f, 8.0f, 0));

  auto complex_shape = [](const vec3& position) -> float {
    auto sphere = sdf::sphere(position, 0.6f);
    auto box = sdf::box(position, vec3(0.4f, 0.4f, 0.4f));
    auto sphere_box = sdf::intersect(sphere, box);
    float cylinder_length = 1.0f;
    float radius = 0.35f;
    auto cylinder_y = sdf::cylinder(position, radius, cylinder_length);
    auto cylinder_x = sdf::cylinder(
      vec3(position.z(), position.x(), position.y()), radius, cylinder_length);
    auto cylinder_z = sdf::cylinder(
      vec3(position.x(), position.z(), position.y()), radius, cylinder_length);
    auto cylinder_cross =
      sdf::combine(cylinder_x, sdf::combine(cylinder_y, cylinder_z));
    return sdf::difference(sphere_box, cylinder_cross);
  };
  list.emplace_back(std::make_unique<FunctionalGeometry>(
    vec3(-2.f, 1.5, -2.5), 100, complex_shape, 2));

  // Construct scene graph
  std::vector<SceneNode> nodes;
  // Root node, only parent in graph
  SceneNode& root_node = nodes.emplace_back();
  root_node.children_id_offset = nodes.size();
  // Camera and camera node
  SceneNode& camera_node = nodes.emplace_back();
  camera_node.camera = std::make_unique<Camera>(
    vec3(0, 0, 0.0f), vec3(0, 0, -1), vec3(0, 1, 0), 90, 1);
  camera_node.type = SceneNode::Type::Camera;
  root_node.children_id_length++;

  // Lights
  std::vector<std::unique_ptr<Object>> light_list;
  float distance = 100.0f;
  light_list.emplace_back(std::make_unique<Point>(vec3(distance, 0, 0), 1));
  light_list.emplace_back(std::make_unique<Point>(vec3(-distance, 0, 0), 1));
  light_list.emplace_back(std::make_unique<Point>(vec3(0, distance, 0), 1));
  light_list.emplace_back(std::make_unique<Point>(vec3(0, -distance, 0), 1));
  light_list.emplace_back(std::make_unique<Point>(vec3(0, 0, distance), 1));
  light_list.emplace_back(std::make_unique<Point>(vec3(0, 0, -distance), 1));

  return std::unique_ptr<Scene>(new Scene(std::move(nodes),
                                          1,
                                          std::move(textures),
                                          std::move(materials),
                                          std::move(list),
                                          std::move(light_list),
                                          0.001f,
                                          16));
}

Scene::Scene(std::vector<SceneNode>&& nodes,
             uint32_t camera_index,
             std::vector<std::unique_ptr<Texture>>&& textures,
             std::vector<std::unique_ptr<Material>>&& materials,
             std::vector<std::unique_ptr<Object>>&& world_objects,
             std::vector<std::unique_ptr<Object>>&& lights,
             float min_attenuation_magnitude,
             uint8_t max_secondary_rays)
  : nodes(std::move(nodes))
  , camera_index(camera_index)
  , textures(std::move(textures))
  , materials(std::move(materials))
  , world_objects(std::move(world_objects))
  , lights(std::move(lights))
  , min_attenuation_magnitude(min_attenuation_magnitude)
  , max_secondary_rays(max_secondary_rays)
{}

Scene::~Scene() = default;

void
Scene::run(float width, float height)
{
  auto& camera = get_camera();
  camera.set_clean();
  camera.set_aspect(width / height);
}

Camera&
Scene::get_camera()
{
  assert(nodes[camera_index].type == SceneNode::Type::Camera);
  assert(nodes[camera_index].camera);
  return *nodes[camera_index].camera;
}

const Camera&
Scene::get_camera() const
{
  assert(nodes[camera_index].type == SceneNode::Type::Camera);
  assert(nodes[camera_index].camera);
  return *nodes[camera_index].camera;
}

const std::vector<std::unique_ptr<Object>>&
Scene::get_world() const
{
  return world_objects;
}

std::vector<std::unique_ptr<Object>>&
Scene::get_world()
{
  return world_objects;
}

const Material&
Scene::get_material(uint16_t id) const
{
  return *materials[id];
}

std::vector<std::unique_ptr<Material>>&
Scene::get_material_list()
{
  return materials;
}

const std::vector<std::unique_ptr<Object>>&
Scene::get_lights() const
{
  return lights;
}

std::vector<std::unique_ptr<Object>>&
Scene::get_lights()
{
  return lights;
}

const Texture&
Scene::get_texture(uint16_t id) const
{
  return *textures[id];
}
