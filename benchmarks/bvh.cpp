#include <benchmark/benchmark.h>

#include <memory>

#include <hittable/triangle_mesh.h>
#include <scene.h>

using Raytracer::Aabb;
using Raytracer::Scene;
using namespace Raytracer::Hittable;
using namespace Raytracer::Math;

/// Base fixture: This sets up the renderer and generates rays
class TriangleBvhFixture : public ::benchmark::Fixture
{
protected:
  void SetUp(::benchmark::State& state) override
  {
    ::benchmark::Fixture::SetUp(state);
    processed_triangle_count = 0;
    triangle_count = 0;
    for (auto& mesh : meshes) {
      triangle_count += mesh->indices.size() / 3;
    }
  }

  void TearDown(::benchmark::State& state) override
  {
    meshes.clear();
    state.counters["triangles_per_second"] = processed_triangle_count;
    state.counters["triangles"] = triangle_count;
    ::benchmark::Fixture::TearDown(state);
  }

  inline void build_bvh_test(benchmark::State& state)
  {
    for (auto _ : state) {
      for (auto& mesh : meshes) {
        mesh->build_bvh();
      }
      processed_triangle_count += triangle_count;
    }
  }

  std::vector<std::unique_ptr<TriangleMesh>> meshes;

  uint32_t triangle_count;
  uint32_t processed_triangle_count;
};

class SingleTriangle : public TriangleBvhFixture
{
protected:
  void SetUp(::benchmark::State& state) override
  {
    // TODO: Check for area 0 AABBs such as this triangle
    std::vector<vec3> positions = {
      vec3(-1, 0, 0),
      vec3(1, 0, 0),
      vec3(0, 1, 0),
    };
    std::vector<MeshVertexData> vertex_data = {
      MeshVertexData{ vec2{ 0, 1 }, vec3{ 0, 0, -1 }, vec3{ 1, 0, 0 } },
      MeshVertexData{ vec2{ 1, 1 }, vec3{ 0, 0, -1 }, vec3{ 1, 0, 0 } },
      MeshVertexData{ vec2{ 0.5, 0 }, vec3{ 0, 0, -1 }, vec3{ 1, 0, 0 } },
    };
    std::vector<uint16_t> indices = { 0, 1, 2 };

    meshes.emplace_back(std::make_unique<TriangleMesh>(
      std::move(positions), std::move(vertex_data), std::move(indices), 0));
    TriangleBvhFixture::SetUp(state);
  }
};

BENCHMARK_F(SingleTriangle, BuildBvh)(benchmark::State& state)
{
  build_bvh_test(state);
}

class TwoTriangles : public TriangleBvhFixture
{
protected:
  void SetUp(::benchmark::State& state) override
  {
    std::vector<vec3> positions = {
      vec3(-1, 0, 0), vec3(1, 0, 0), vec3(0, 1, 0),
      vec3(-1, 0, 5), vec3(1, 0, 5), vec3(0, 1, 5),
    };
    std::vector<MeshVertexData> vertex_data = {
      MeshVertexData{ vec2{ 0, 1 }, vec3{ 0, 0, -1 }, vec3{ 1, 0, 0 } },
      MeshVertexData{ vec2{ 1, 1 }, vec3{ 0, 0, -1 }, vec3{ 1, 0, 0 } },
      MeshVertexData{ vec2{ 0.5, 0 }, vec3{ 0, 0, -1 }, vec3{ 1, 0, 0 } },
    };
    std::vector<uint16_t> indices = { 0, 1, 2, 3, 4, 5 };

    meshes.emplace_back(std::make_unique<TriangleMesh>(
      std::move(positions), std::move(vertex_data), std::move(indices), 0));
    TriangleBvhFixture::SetUp(state);
  }
};

BENCHMARK_F(TwoTriangles, BuildBvh)(benchmark::State& state)
{
  build_bvh_test(state);
}

class NTriangles : public TriangleBvhFixture
{
protected:
  void SetUp(::benchmark::State& state) override
  {
    std::vector<vec3> positions;
    std::vector<MeshVertexData> vertex_data;
    std::vector<uint16_t> indices;
    for (uint8_t i = 0; i < state.range(0); ++i) {
      positions.emplace_back(-1, 0, i);
      positions.emplace_back(1, 0, i);
      positions.emplace_back(0, 1, i);
      vertex_data.emplace_back(
        MeshVertexData{ vec2{ 0, 1 }, vec3{ 0, 0, -1 }, vec3{ 1, 0, 0 } });
      vertex_data.emplace_back(
        MeshVertexData{ vec2{ 1, 1 }, vec3{ 0, 0, -1 }, vec3{ 1, 0, 0 } });
      vertex_data.emplace_back(
        MeshVertexData{ vec2{ 0.5, 0 }, vec3{ 0, 0, -1 }, vec3{ 1, 0, 0 } });
      indices.emplace_back(i * 3);
      indices.emplace_back(i * 3 + 1);
      indices.emplace_back(i * 3 + 2);
    }

    meshes.emplace_back(std::make_unique<TriangleMesh>(
      std::move(positions), std::move(vertex_data), std::move(indices), 0));
    TriangleBvhFixture::SetUp(state);
  }
};

BENCHMARK_DEFINE_F(NTriangles, BuildBvh)(benchmark::State& state)
{
  build_bvh_test(state);
}

BENCHMARK_REGISTER_F(NTriangles, BuildBvh)->Arg(100);

class GltfFixture : public TriangleBvhFixture
{
protected:
  virtual std::string_view get_filename() const = 0;

  void SetUp(::benchmark::State& state) override
  {
    auto scene = Scene::load_from_gltf(get_filename().data());
    for (auto& object : scene->get_world()) {
      auto triangles = dynamic_cast<TriangleMesh*>(object.get());
      if (triangles) {
        meshes.emplace_back(std::make_unique<TriangleMesh>(
          *dynamic_cast<TriangleMesh*>(scene->get_world()[0].get())));
      }
    }
    TriangleBvhFixture::SetUp(state);
  }
};

class BoxTextured : public GltfFixture
{
protected:
  std::string_view get_filename() const override { return "BoxTextured.gltf"; }
};

BENCHMARK_F(BoxTextured, BuildBvh)(benchmark::State& state)
{
  build_bvh_test(state);
}

class Duck : public GltfFixture
{
protected:
  std::string_view get_filename() const override { return "Duck.gltf"; }
};

BENCHMARK_F(Duck, BuildBvh)(benchmark::State& state)
{
  build_bvh_test(state);
}

class DamagedHelmet : public GltfFixture
{
protected:
  std::string_view get_filename() const override
  {
    return "DamagedHelmet.gltf";
  }
};

BENCHMARK_F(DamagedHelmet, BuildBvh)(benchmark::State& state)
{
  build_bvh_test(state);
}

class Sponza : public GltfFixture
{
protected:
  std::string_view get_filename() const override { return "Sponza.gltf"; }
};

BENCHMARK_F(Sponza, BuildBvh)(benchmark::State& state)
{
  build_bvh_test(state);
}
