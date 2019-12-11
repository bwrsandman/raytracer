#include <benchmark/benchmark.h>

#include <hit_record.h>
#include <hittable/aabb.h>
#include <ray.h>

using Raytracer::Ray;
using Aabb = Raytracer::Hittable::AABB;
using Raytracer::Math::random_in_unit_sphere;
using Raytracer::Math::vec3;

// skips y and z
// reciprocal is positive
// returns true
static void
ray_aabb_hit_on_x(benchmark::State& state)
{
  Ray ray(vec3(0, 0, 0), vec3(1, 0, 0));
  Aabb box{ vec3(2, 0, 0), vec3(2.5f, 0.5f, 0.5f) };
  Raytracer::hit_record rec;
  uint32_t intersection_count = 0;
  for (auto _ : state) {
    box.hit(ray, false, 0, 1e10f, rec);
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
  Raytracer::hit_record rec;
  uint32_t intersection_count = 0;
  for (auto _ : state) {
    box.hit(ray, false, 0, 1e10f, rec);
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
  Raytracer::hit_record rec;
  uint32_t intersection_count = 0;
  for (auto _ : state) {
    box.hit(ray, false, 0, 1e10f, rec);
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
  Raytracer::hit_record rec;
  uint32_t intersection_count = 0;
  for (auto _ : state) {
    box.hit(ray, false, 0, 1e10f, rec);
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
  Raytracer::hit_record rec;
  uint32_t intersection_count = 0;
  for (auto _ : state) {
    box.hit(ray, false, 0, 1e10f, rec);
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
  Raytracer::hit_record rec;
  uint32_t intersection_count = 0;
  for (auto _ : state) {
    box.hit(ray, false, 0, 1e10f, rec);
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
    Aabb box = Aabb(vec3(0.0f, 0.0f, 0.0f), vec3(1.0f, 1.0f, 1.0f));
    Ray ray;
  };
  std::array<ray_aabb_combo, 1000> combos;
  for (uint32_t i = 0; i < combos.size(); ++i) {
    combos[i].ray = Ray(random_in_unit_sphere() * 10, random_in_unit_sphere());
    combos[i].box = Aabb(random_in_unit_sphere() - vec3(2.0f, 2.0f, 2.0f),
                         random_in_unit_sphere() + vec3(2.0f, 2.0f, 2.0f));
  }
  Raytracer::hit_record rec;
  uint32_t intersection_count = 0;
  for (auto _ : state) {
    combos[intersection_count % combos.size()].box.hit(
      combos[intersection_count % combos.size()].ray, false, 0, 1e10f, rec);
    ++intersection_count;
  }
  state.counters["intersections_per_second"] = intersection_count;
}
BENCHMARK(ray_aabb_hit_random);
