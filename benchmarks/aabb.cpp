#include <benchmark/benchmark.h>

#include <array>

#include <aabb.h>
#include <math/vec3_simd.h>
#include <ray.h>

using Raytracer::Aabb;
using Raytracer::AabbSimd;
using Raytracer::Ray;
using Raytracer::RaySimd;
using Raytracer::Math::float_simd_t;
using Raytracer::Math::random_in_unit_sphere;
using Raytracer::Math::vec3;
using Raytracer::Math::vec3_simd;

// skips y and z
// reciprocal is positive
// returns true
static void
ray_aabb_hit_on_x(benchmark::State& state)
{
  Ray ray(vec3(0, 0, 0), vec3(1, 0, 0));
  Aabb box{ vec3(2, 0, 0), vec3(2.5f, 0.5f, 0.5f) };
  uint32_t intersection_count = 0;
  for (auto _ : state) {
    Aabb::hit(box, ray, 0, 1e10f);
    ++intersection_count;
  }
  state.counters["intersections_per_second"] = intersection_count;
}
BENCHMARK(ray_aabb_hit_on_x);

// skips y and z
// reciprocal is positive
// returns false
static void
ray_aabb_hit_no_hit_on_x(benchmark::State& state)
{
  Ray ray(vec3(0, 0, 0), vec3(1, 0, 0));
  Aabb box{ vec3(-1, 0, 0), vec3(-0.5f, 0.5f, 0.5f) };
  uint32_t intersection_count = 0;
  for (auto _ : state) {
    Aabb::hit(box, ray, 0, 1e10f);
    ++intersection_count;
  }
  state.counters["intersections_per_second"] = intersection_count;
}
BENCHMARK(ray_aabb_hit_no_hit_on_x);

// skips x and y
// reciprocal is negative
// returns false
static void
ray_aabb_hit_z_negative_reciprocal(benchmark::State& state)
{
  Ray ray(vec3(0, 0, 0), vec3(0, 0, -1));
  Aabb box{ vec3(0, 0, 0), vec3(1, 1, 1) };
  uint32_t intersection_count = 0;
  for (auto _ : state) {
    Aabb::hit(box, ray, 0, 1e10f);
    ++intersection_count;
  }
  state.counters["intersections_per_second"] = intersection_count;
}
BENCHMARK(ray_aabb_hit_z_negative_reciprocal);

// doesn't skip x, y, z
// reciprocal is positive and less than 1
// returns true
static void
ray_aabb_hit_diagonal(benchmark::State& state)
{
  Ray ray(vec3(0, 0, 0), vec3(std::sqrt(3), std::sqrt(3), std::sqrt(3)));
  Aabb box{ vec3(0, 0, 0), vec3(1, 1, 1) };
  uint32_t intersection_count = 0;
  for (auto _ : state) {
    Aabb::hit(box, ray, 0, 1e10f);
    ++intersection_count;
  }
  state.counters["intersections_per_second"] = intersection_count;
}
BENCHMARK(ray_aabb_hit_diagonal);

// doesn't skip x, y, z
// reciprocal is positive and less than 1
// returns true
static void
ray_aabb_hit_diagonal_negative_reciprocal(benchmark::State& state)
{
  Ray ray(vec3(0, 0, 0), vec3(std::sqrt(3), std::sqrt(3), -std::sqrt(3)));
  Aabb box{ vec3(-1, -1, -1), vec3(1, 1, 1) };
  uint32_t intersection_count = 0;
  for (auto _ : state) {
    Aabb::hit(box, ray, 0, 1e10f);
    ++intersection_count;
  }
  state.counters["intersections_per_second"] = intersection_count;
}
BENCHMARK(ray_aabb_hit_diagonal_negative_reciprocal);

// doesn't skip x, y, z
// reciprocal is positive and less than 1
// returns true
static void
ray_aabb_hit_diagonal_off_zero(benchmark::State& state)
{
  Ray ray(vec3(1, 1, -1), vec3(std::sqrt(3), std::sqrt(3), std::sqrt(3)));
  Aabb box{ vec3(0, 0, 0), vec3(10, 10, 10) };
  uint32_t intersection_count = 0;
  for (auto _ : state) {
    Aabb::hit(box, ray, 0, 1e10f);
    ++intersection_count;
  }
  state.counters["intersections_per_second"] = intersection_count;
}
BENCHMARK(ray_aabb_hit_diagonal_off_zero);

static void
ray_aabb_hit_random(benchmark::State& state)
{
  struct ray_aabb_combo
  {
    Aabb box;
    Ray ray;
  };
  std::array<ray_aabb_combo, 1000> combos;
  for (uint32_t i = 0; i < combos.size(); ++i) {
    combos[i].ray = Ray(random_in_unit_sphere(), random_in_unit_sphere());
    combos[i].ray.direction.make_unit_vector();
    combos[i].box.min = random_in_unit_sphere();
    combos[i].box.max = random_in_unit_sphere();
  }
  uint32_t intersection_count = 0;
  for (auto _ : state) {
    Aabb::hit(combos[intersection_count % combos.size()].box,
              combos[intersection_count % combos.size()].ray,
              0,
              1e10f);
    ++intersection_count;
  }
  state.counters["intersections_per_second"] = intersection_count;
}
BENCHMARK(ray_aabb_hit_random);

static void
ray_aabb_simd4_hit_random(benchmark::State& state)
{
  constexpr uint8_t simd_multiplier = 4;
  struct ray_aabb_combo
  {
    AabbSimd<simd_multiplier> box;
    Ray ray;
  };
  std::array<ray_aabb_combo, 1000> combos;
  for (uint32_t i = 0; i < combos.size(); ++i) {
    combos[i].ray = Ray(random_in_unit_sphere(), random_in_unit_sphere());
    combos[i].ray.direction.make_unit_vector();
    combos[i].box.min = vec3_simd<simd_multiplier>({ random_in_unit_sphere(),
                                                     random_in_unit_sphere(),
                                                     random_in_unit_sphere(),
                                                     random_in_unit_sphere() });
    combos[i].box.max = vec3_simd<simd_multiplier>({ random_in_unit_sphere(),
                                                     random_in_unit_sphere(),
                                                     random_in_unit_sphere(),
                                                     random_in_unit_sphere() });
  }
  uint32_t intersection_count = 0;
  for (auto _ : state) {
    AabbSimd<simd_multiplier>::hit(
      combos[intersection_count % combos.size()].box,
      combos[intersection_count % combos.size()].ray,
      float_simd_t<simd_multiplier>(0.0f),
      float_simd_t<simd_multiplier>(1e10f));
    ++intersection_count;
  }
  state.counters["intersections_per_second"] =
    intersection_count * simd_multiplier;
}
BENCHMARK(ray_aabb_simd4_hit_random);

static void
ray_aabb_simd8_hit_random(benchmark::State& state)
{
  constexpr uint8_t simd_multiplier = 8;
  struct ray_aabb_combo
  {
    AabbSimd<simd_multiplier> box;
    Ray ray;
  };
  std::array<ray_aabb_combo, 1000> combos;
  for (uint32_t i = 0; i < combos.size(); ++i) {
    combos[i].ray = Ray(random_in_unit_sphere(), random_in_unit_sphere());
    combos[i].ray.direction.make_unit_vector();
    combos[i].box.min = vec3_simd<simd_multiplier>({ random_in_unit_sphere(),
                                                     random_in_unit_sphere(),
                                                     random_in_unit_sphere(),
                                                     random_in_unit_sphere(),
                                                     random_in_unit_sphere(),
                                                     random_in_unit_sphere(),
                                                     random_in_unit_sphere(),
                                                     random_in_unit_sphere() });
    combos[i].box.max = vec3_simd<simd_multiplier>({ random_in_unit_sphere(),
                                                     random_in_unit_sphere(),
                                                     random_in_unit_sphere(),
                                                     random_in_unit_sphere(),
                                                     random_in_unit_sphere(),
                                                     random_in_unit_sphere(),
                                                     random_in_unit_sphere(),
                                                     random_in_unit_sphere() });
  }
  uint32_t intersection_count = 0;
  for (auto _ : state) {
    AabbSimd<simd_multiplier>::hit(
      combos[intersection_count % combos.size()].box,
      combos[intersection_count % combos.size()].ray,
      float_simd_t<simd_multiplier>(0.0f),
      float_simd_t<simd_multiplier>(1e10f));
    ++intersection_count;
  }
  state.counters["intersections_per_second"] =
    intersection_count * simd_multiplier;
}
BENCHMARK(ray_aabb_simd8_hit_random);

static void
ray_simd4_aabb_hit_random(benchmark::State& state)
{
  constexpr uint8_t simd_multiplier = 4;
  struct ray_aabb_combo
  {
    Aabb box;
    RaySimd<simd_multiplier> ray;
  };
  std::array<ray_aabb_combo, 1000> combos;
  for (uint32_t i = 0; i < combos.size(); ++i) {
    combos[i].ray.origin =
      vec3_simd<simd_multiplier>({ random_in_unit_sphere(),
                                   random_in_unit_sphere(),
                                   random_in_unit_sphere(),
                                   random_in_unit_sphere() });
    combos[i].ray.direction =
      vec3_simd<simd_multiplier>({ random_in_unit_sphere(),
                                   random_in_unit_sphere(),
                                   random_in_unit_sphere(),
                                   random_in_unit_sphere() });
    combos[i].ray.direction.make_unit_vector();
    combos[i].box.min = random_in_unit_sphere();
    combos[i].box.max = random_in_unit_sphere();
  }
  uint32_t intersection_count = 0;
  for (auto _ : state) {
    Aabb::hit(combos[intersection_count % combos.size()].box,
              combos[intersection_count % combos.size()].ray,
              float_simd_t<simd_multiplier>(0.0f),
              float_simd_t<simd_multiplier>(1e10f));
    ++intersection_count;
  }
  state.counters["intersections_per_second"] =
    intersection_count * simd_multiplier;
}
BENCHMARK(ray_simd4_aabb_hit_random);

static void
ray_simd8_aabb_hit_random(benchmark::State& state)
{
  constexpr uint8_t simd_multiplier = 8;
  struct ray_aabb_combo
  {
    Aabb box;
    RaySimd<simd_multiplier> ray;
  };
  std::array<ray_aabb_combo, 1000> combos;
  for (uint32_t i = 0; i < combos.size(); ++i) {
    combos[i].ray.origin =
      vec3_simd<simd_multiplier>({ random_in_unit_sphere(),
                                   random_in_unit_sphere(),
                                   random_in_unit_sphere(),
                                   random_in_unit_sphere(),
                                   random_in_unit_sphere(),
                                   random_in_unit_sphere(),
                                   random_in_unit_sphere(),
                                   random_in_unit_sphere() });
    combos[i].ray.direction =
      vec3_simd<simd_multiplier>({ random_in_unit_sphere(),
                                   random_in_unit_sphere(),
                                   random_in_unit_sphere(),
                                   random_in_unit_sphere(),
                                   random_in_unit_sphere(),
                                   random_in_unit_sphere(),
                                   random_in_unit_sphere(),
                                   random_in_unit_sphere() });
    combos[i].ray.direction.make_unit_vector();
    combos[i].box.min = random_in_unit_sphere();
    combos[i].box.max = random_in_unit_sphere();
  }
  uint32_t intersection_count = 0;
  for (auto _ : state) {
    Aabb::hit(combos[intersection_count % combos.size()].box,
              combos[intersection_count % combos.size()].ray,
              float_simd_t<simd_multiplier>(0.0f),
              float_simd_t<simd_multiplier>(1e10f));
    ++intersection_count;
  }
  state.counters["intersections_per_second"] =
    intersection_count * simd_multiplier;
}
BENCHMARK(ray_simd8_aabb_hit_random);
