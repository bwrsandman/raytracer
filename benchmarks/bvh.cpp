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
    triangle_count = mesh->positions.size() / 3;
  }

  void TearDown(::benchmark::State& state) override
  {
    mesh.reset();
    state.counters["triangles_per_second"] = processed_triangle_count;
    ::benchmark::Fixture::TearDown(state);
  }

  inline void build_bvh_test(benchmark::State& state)
  {
    for (auto _ : state) {
      mesh->build_bvh();
      processed_triangle_count += triangle_count;
    }
  }

  std::unique_ptr<TriangleMesh> mesh;

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

    mesh = std::make_unique<TriangleMesh>(
      std::move(positions), std::move(vertex_data), std::move(indices), 0);
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

    mesh = std::make_unique<TriangleMesh>(
      std::move(positions), std::move(vertex_data), std::move(indices), 0);
    TriangleBvhFixture::SetUp(state);
  }
};

BENCHMARK_F(TwoTriangles, BuildBvh)(benchmark::State& state)
{
  build_bvh_test(state);
}

class BoxTextured : public TriangleBvhFixture
{
protected:
  void SetUp(::benchmark::State& state) override
  {
    auto scene = Scene::load_from_gltf("BoxTextured.gltf");
    mesh = std::make_unique<TriangleMesh>(
      *dynamic_cast<TriangleMesh*>(scene->get_world()[0].get()));
    TriangleBvhFixture::SetUp(state);
  }
};

BENCHMARK_F(BoxTextured, BuildBvh)(benchmark::State& state)
{
  build_bvh_test(state);
}

class Duck : public TriangleBvhFixture
{
protected:
  void SetUp(::benchmark::State& state) override
  {
    auto scene = Scene::load_from_gltf("Duck.gltf");
    mesh = std::make_unique<TriangleMesh>(
      *dynamic_cast<TriangleMesh*>(scene->get_world()[0].get()));
    TriangleBvhFixture::SetUp(state);
  }
};

BENCHMARK_F(Duck, BuildBvh)(benchmark::State& state)
{
  build_bvh_test(state);
}

class DamagedHelmet : public TriangleBvhFixture
{
protected:
  void SetUp(::benchmark::State& state) override
  {
    auto scene = Scene::load_from_gltf("DamagedHelmet.gltf");
    mesh = std::make_unique<TriangleMesh>(
      *dynamic_cast<TriangleMesh*>(scene->get_world()[0].get()));
    TriangleBvhFixture::SetUp(state);
  }
};

BENCHMARK_F(DamagedHelmet, BuildBvh)(benchmark::State& state)
{
  build_bvh_test(state);
}
