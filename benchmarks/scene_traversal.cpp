#include <random>

#include <benchmark/benchmark.h>

#include <camera.h>
#include <ray.h>
#include <renderer.h>
#include <scene.h>

using Raytracer::Camera;
using Raytracer::Ray;
using Raytracer::Scene;
using Raytracer::Graphics::Renderer;
using Raytracer::Math::random_double;
using Raytracer::Math::vec3;

constexpr uint32_t ray_count = 0x1000;

/// Base fixture: This sets up the renderer and generates rays
class BaseSceneFixture : public ::benchmark::Fixture
{
protected:
  void SetUp(::benchmark::State& state) override
  {
    ray_index = 0;
    total_rays = 0;

    renderer = Renderer::create(Renderer::Type::Whitted, nullptr);
    for (auto& r : rays) {
      r = scene->get_camera().get_ray(random_double(), random_double());
    }
  }

  void TearDown(::benchmark::State& state) override
  {
    renderer.reset();
    scene.reset();

    state.counters["primary rays"] = ray_index;
    state.counters["primary_rays_per_second"] = ::benchmark::Counter(
      static_cast<double>(ray_index), benchmark::Counter::kIsRate);
    state.counters["rays"] = total_rays;
    state.counters["rays_per_second"] = ::benchmark::Counter(
      static_cast<double>(total_rays), benchmark::Counter::kIsRate);
  }

  inline void raygen_test(benchmark::State& state)
  {
    for (auto _ : state) {
      vec3 color = vec3(0.0f, 0.0f, 0.0f);
      auto ray = rays[ray_index % ray_count];
      total_rays += renderer->raygen(ray, *scene, color);
      ray_index++;
    }
  }

  std::unique_ptr<Renderer> renderer;
  std::unique_ptr<Scene> scene;
  Ray rays[ray_count];
  uint32_t ray_index;
  uint32_t total_rays;
};

class Whitted : public BaseSceneFixture
{
protected:
  void SetUp(::benchmark::State& state) override
  {
    scene = Scene::load_whitted_scene();
    BaseSceneFixture::SetUp(state);
  }
};

BENCHMARK_F(Whitted, PrimaryRayTraverse)(benchmark::State& state)
{
  raygen_test(state);
}

class Cornell : public BaseSceneFixture
{
protected:
  void SetUp(::benchmark::State& state) override
  {
    scene = Scene::load_cornell_box();
    BaseSceneFixture::SetUp(state);
  }
};

BENCHMARK_F(Cornell, PrimaryRayTraverse)(benchmark::State& state)
{
  raygen_test(state);
}

class Mandelbulb : public BaseSceneFixture
{
protected:
  void SetUp(::benchmark::State& state) override
  {
    scene = Scene::load_mandrelbulb();
    BaseSceneFixture::SetUp(state);
  }
};

BENCHMARK_F(Mandelbulb, PrimaryRayTraverse)(benchmark::State& state)
{
  raygen_test(state);
}

class glTFBox : public BaseSceneFixture
{
protected:
  void SetUp(::benchmark::State& state) override
  {
    scene = Scene::load_from_gltf("BoxTextured.gltf");
    BaseSceneFixture::SetUp(state);
  }
};

BENCHMARK_F(glTFBox, PrimaryRayTraverse)(benchmark::State& state)
{
  raygen_test(state);
}

class glTFDuck : public BaseSceneFixture
{
protected:
  void SetUp(::benchmark::State& state) override
  {
    scene = Scene::load_from_gltf("Duck.gltf");
    BaseSceneFixture::SetUp(state);
  }
};

BENCHMARK_F(glTFDuck, PrimaryRayTraverse)(benchmark::State& state)
{
  raygen_test(state);
}

class glTFDamagedHelmet : public BaseSceneFixture
{
protected:
  void SetUp(::benchmark::State& state) override
  {
    scene = Scene::load_from_gltf("DamagedHelmet.gltf");
    BaseSceneFixture::SetUp(state);
  }
};

BENCHMARK_F(glTFDamagedHelmet, PrimaryRayTraverse)(benchmark::State& state)
{
  raygen_test(state);
}
