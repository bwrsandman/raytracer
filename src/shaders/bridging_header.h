#ifndef WHITTED_RAYTRACING_BRIDGING_HEADER_H
#define WHITTED_RAYTRACING_BRIDGING_HEADER_H

#if __cplusplus
#define out
#define inout
#define REF &
#define mix(a, b, t) lerp((a), (b), (t))
typedef int8_t vec3_snorm[3];
typedef int8_t vec2_snorm[2];
using namespace Raytracer::Math;
struct uvec4
{
  uint32_t e[4];
};

static float
uintBitsToFloat(uint32_t u)
{
  union
  {
    uint32_t uint32;
    float float32;
  } u_to_f;
  u_to_f.uint32 = u;
  return u_to_f.float32;
}
#else
#define uint32_t uint
#define alignas(x)
#define M_PI 3.14159265358979323846       /* pi */
#define M_PI_2 1.57079632679489661923     /* pi/2 */
#define M_PI_4 0.78539816339744830962     /* pi/4 */
#define M_1_PI 0.31830988618379067154     /* 1/pi */
#define M_1_2PI 0.15915494309189533576    /* 1/2pi */
#define M_2_PI 0.63661977236758134308     /* 2/pi */
#define M_2_SQRTPI 1.12837916709551257390 /* 2/sqrt(pi) */
#define M_SQRT2 1.41421356237309504880    /* sqrt(2) */
#define M_SQRT1_2 0.70710678118654752440  /* 1/sqrt(2) */

// From cmath
#ifndef _HUGE_ENUF
#define _HUGE_ENUF 1e+300 // _HUGE_ENUF*_HUGE_ENUF must overflow
#endif
#define INFINITY (float(_HUGE_ENUF * _HUGE_ENUF))
#define HUGE_VAL (double(INFINITY))
#define HUGE_VALF (float(INFINITY))
#define NAN (float(INFINITY * 0.0F))
#define FLT_EPSILON (float(5.96e-08))

#define T_MIN (10e2 * FLT_EPSILON)

#define static
#define REF

#define vec3_snorm vec3
#define vec2_snorm vec2
#endif

#include "plane_t.h"
#include "sphere_t.h"

/// This is a header which is read by both the gpu shader code and the cpu
/// c++ code.
/// The purpose of this header is to avoid double declarations.

struct alignas(16) camera_uniform_t
{
  alignas(16) vec3 origin;
  alignas(16) vec3 lower_left_corner;
  alignas(16) vec3 horizontal;
  alignas(16) vec3 vertical;
  alignas(16) vec3 u;
  alignas(16) vec3 v;
  float lens_radius;
};

struct alignas(16) raygen_uniform_t
{
  camera_uniform_t camera;
  uint32_t frame_count;
  uint32_t width;
  uint32_t height;
};

struct alignas(16) scene_traversal_common_uniform_t
{
  uint32_t early_out;
};

#define MAX_NUM_SPHERES 4

struct alignas(16) scene_traversal_sphere_uniform_t
{
  // x, y, z, radius
  vec4 spheres[MAX_NUM_SPHERES];
  // material_id, unused x3
  uvec4 materials[MAX_NUM_SPHERES];
  uint32_t count;
};

#define MAX_NUM_PLANES 0x10

struct alignas(16) scene_traversal_plane_uniform_t
{
  vec4 min[MAX_NUM_PLANES];
  vec4 max[MAX_NUM_PLANES];
  vec4 normal[MAX_NUM_PLANES];
  // material_id, unused x3
  uvec4 materials[MAX_NUM_PLANES];
  uint32_t count;
};

#define MAX_NUM_VERTICES 0x2000
#define MAX_NUM_TRIANGLES 0x2000
#define MAX_NUM_BVH_NODES 0x2000
#define NODE_TO_VISIT_STACK_SIZE 16

struct alignas(16) scene_traversal_triangle_vertex_t
{
  vec3 position[MAX_NUM_VERTICES];
  vec3_snorm normal[MAX_NUM_VERTICES];
  vec3_snorm tangent[MAX_NUM_VERTICES];
  vec2_snorm uv[MAX_NUM_VERTICES];
};

struct alignas(16) scene_traversal_triangle_bvh_t
{
  vec4 p0[MAX_NUM_BVH_NODES]; // aabb min + offset
  vec4 p1[MAX_NUM_BVH_NODES]; // aabb max + count
};

struct alignas(16) scene_traversal_triangle_uniform_t
{
  uint32_t mat_id;
};

// TODO: Remove concept of material types and use single PBR material
#define MATERIAL_TYPE_UNKNOWN 0
#define MATERIAL_TYPE_LAMBERT 1
#define MATERIAL_TYPE_METAL 2
#define MATERIAL_TYPE_DIELECTRIC 3
#define MATERIAL_TYPE_EMISSIVE 4

#define LIGHT_TYPE_POINT 0
#define LIGHT_TYPE_SPHERE 1
#define LIGHT_TYPE_PLANE 2

#define NORMAL_RAY 0
#define NEE_RANDOM_RAY 1

struct alignas(16) light_t
{
  vec4 p0; // point center, sphere center + radius, plane min
  vec4 p1; // point (unused), sphere (unused), plane max + axis
  uint32_t type;
};

#define MAX_NUM_LIGHTS 16
#define MAX_NUM_MATERIALS 16
struct alignas(16) anyhit_uniform_data_t
{
  /// color (rgb) / refraction index+ni+albedo, type (a)
  vec4 material_data[MAX_NUM_MATERIALS];
  uint32_t material_count;
  light_t lights[MAX_NUM_LIGHTS];
  uint32_t light_count;
  uint32_t frame_count;
  uint32_t width;
};

struct alignas(16) shadow_ray_light_hit_uniform_data_t
{
  vec4 light_color_data[MAX_NUM_LIGHTS];
  uint32_t light_count;
};

/// Construct a float with half-open range [0:1] using low 23 bits.
/// All zeroes yields 0.0, all ones yields the next smallest representable value
/// below 1.0.
/// https://stackoverflow.com/a/17479300/10604387
static float
uint_to_normalized_float(uint32_t u)
{
  const uint32_t ieee_mantissa_mask = 0x007FFFFFu;
  const uint32_t ieee_one_bits = 0x3F800000u; // 1.0 in IEEE binary32

  u &= ieee_mantissa_mask; // Keep only mantissa bits (fractional part)
  u |= ieee_one_bits;      // Add fractional part to 1.0

  float f = uintBitsToFloat(u); // Range [1:2]
  return f - 1.0f;              // Range [0:1]
}

static uint32_t
rand_seed(uint32_t thread_id, uint32_t frame_id)
{
  // For a list of 9-digit primes (will fit in 32-bit)
  // see: http://www.rsok.com/~jrm/9_digit_palindromic_primes.html
  const uint32_t large_prime_0 = 119010911; // chosen by fair die roll.
  const uint32_t large_prime_1 = 125292521; // guaranteed to be random.
  //                                         https://www.xkcd.com/221/

  return (thread_id + frame_id * large_prime_0) * large_prime_1;
}

/// Xorshift by George Marsaglia
/// Less instructions but visible patterns
static float
rand_xor32(inout uint32_t REF seed)
{
  seed ^= seed << 13;
  seed ^= seed >> 17;
  seed ^= seed << 5;
  return uint_to_normalized_float(seed);
}

/// Wang Hash by Thomas Wang
/// A few more instructions but less patterns
/// http://www.burtleburtle.net/bob/hash/integer.html
static float
rand_wang_hash(inout uint32_t REF seed)
{
  seed = (seed ^ 61) ^ (seed >> 16);
  seed *= 9;
  seed = seed ^ (seed >> 4);
  seed *= 0x27d4eb2d;
  seed = seed ^ (seed >> 15);
  return uint_to_normalized_float(seed);
}

static vec2
random_point_in_unit_square_wang_hash(inout uint32_t REF seed)
{
  return vec2(rand_wang_hash(seed), rand_wang_hash(seed));
}

static vec3
random_point_in_unit_cube_wang_hash(inout uint32_t REF seed)
{
  return vec3(rand_wang_hash(seed), rand_wang_hash(seed), rand_wang_hash(seed));
}

static vec3
random_point_in_unit_disk_wang_hash(inout uint32_t REF seed)
{
  vec3 point;
  do {
    point = 2.0f * vec3(rand_wang_hash(seed), rand_wang_hash(seed), 0) -
            vec3(1, 1, 0);
  } while (dot(point, point) >= 1.0f);
  return point;
}

static vec3
random_point_in_unit_sphere_wang_hash(inout uint32_t REF seed)
{
  vec3 point;
  do {
    point = 2.0f * random_point_in_unit_cube_wang_hash(seed) - vec3(1, 1, 1);
  } while (dot(point, point) >= 1.0f);
  return point;
}

static float
schlick(float cosine, float refraction_index)
{
  float r0 = (1.0f - refraction_index) / (1.0f + refraction_index);
  r0 = r0 * r0;
  return r0 + (1.0f - r0) * pow((1.0f - cosine), 5.0f);
}

static bool
refract_(vec3 incident, vec3 normal, float ni_over_nt, out vec3 REF refracted)
{
  float cosine = dot(incident, normal);
  float discriminant =
    1.0f - ni_over_nt * ni_over_nt * (1.0f - cosine * cosine);
  if (discriminant > 0.0f) {
    refracted =
      ni_over_nt * (incident - normal * cosine) - normal * sqrt(discriminant);
    return true;
  }
  return false;
}

static vec3
material_dielectric_scatter(inout uint32_t seed,
                            vec3 direction,
                            vec3 normal,
                            float refraction_index,
                            inout bool inside)
{
  vec3 outward_normal;
  vec3 reflected = reflect(direction, normal);
  float ni_over_nt;
  vec3 refracted;
  float reflect_probability;
  float cosine = dot(direction, normal);
  if (cosine > 0.0f) {
    // inside out
    inside = true;
    outward_normal = -normal;
    ni_over_nt = refraction_index;
    cosine =
      sqrt(1 - refraction_index * refraction_index * (1.0f - cosine * cosine));
  } else {
	// outside in
    outward_normal = normal;
    ni_over_nt = 1.0f / refraction_index;
    cosine = -cosine;
  }
  if (refract_(direction, outward_normal, ni_over_nt, refracted)) {
    reflect_probability = schlick(cosine, refraction_index);
  } else {
    reflect_probability = 1.0f;
  }
  if (rand_wang_hash(seed) < reflect_probability) {
    return reflected;
  } else {
    return refracted;
  }
}

#ifndef __cplusplus
static vec3
random_point_on_unit_hemisphere_wang_hash(inout uint REF seed, vec3 REF normal)
{
  vec3 point = random_point_in_unit_sphere_wang_hash(seed);
  point[1] = abs(point[1]); // transform into point on up vector hemisphere

  // Create rotation matrix from up vector to normal
  float c = normal[1]; // cos of the angle
  if (abs(c + 1) < FLT_EPSILON) {
    point = -point;
  } else {
    mat3 skew_cross_mat =
      mat3(0, -normal[0], 0, normal[0], 0, normal[2], 0, -normal[2], 0);
    mat3 skew_cross_mat2 = skew_cross_mat * skew_cross_mat;
    const mat3 identity = mat3(1, 0, 0, 0, 1, 0, 0, 0, 1);
    mat3 rotation =
      identity + skew_cross_mat + skew_cross_mat2 * (1.0f / (1 + c));
    point = rotation * point;
  }

  return normalize(point); // project onto surface
}

static vec3
random_point_on_light(light_t REF light, vec3 REF origin, inout uint REF seed)
{
  if (light.type == LIGHT_TYPE_POINT) {
    return vec3(light.p0[0], light.p0[1], light.p0[2]);
  } else if (light.type == LIGHT_TYPE_SPHERE) {
    vec3 pos = vec3(light.p0[0], light.p0[1], light.p0[2]);
    vec3 normal = normalize(origin - pos);
    return pos + random_point_on_unit_hemisphere_wang_hash(seed, normal) *
                   light.p0[3];
  } else if (light.type == LIGHT_TYPE_PLANE) {
    vec2 rand = random_point_in_unit_square_wang_hash(seed);
    vec3 extent = light.p1.xyz - light.p0.xyz;
    if (light.p1[3] == 0) { // yz plane
      return light.p0.xyz + extent * vec3(0, rand[0], rand[1]);
    } else if (light.p1[3] == 1) { // xz plane
      return light.p0.xyz + extent * vec3(rand[0], 0, rand[1]);
    } else { // xy plane
      return light.p0.xyz + extent * vec3(rand[0], rand[1], 0);
    }
  }
  return vec3(light.p0[0], light.p0[1], light.p0[2]);
}

static vec4
random_light_destination(light_t REF lights[MAX_NUM_LIGHTS],
                         uint light_count,
                         vec3 REF origin,
                         inout uint REF seed,
                         out uint REF light_index)
{
  vec4 pdfs[MAX_NUM_LIGHTS];
  float pdf_total = 0;
  vec3 direction, normal_light;
  float area;

  for (int i = 0; i < light_count; i++) {
    light_t light = lights[i];
    vec3 dest_point;

    if (light.type == LIGHT_TYPE_POINT) {
      dest_point = vec3(light.p0[0], light.p0[1], light.p0[2]);
      direction = normalize(dest_point - origin);
      normal_light = -direction;
      area = 0.001f;

    } else if (light.type == LIGHT_TYPE_SPHERE) {
      vec3 pos = vec3(light.p0[0], light.p0[1], light.p0[2]);
      vec3 normal = normalize(origin - pos);
      dest_point =
        pos +
        random_point_on_unit_hemisphere_wang_hash(seed, normal) * light.p0[3];

      direction = normalize(dest_point - origin);
      normal_light = -direction;
      area = 2 * M_PI * light.p0[3] * light.p0[3];

    } else if (light.type == LIGHT_TYPE_PLANE) {
      vec2 rand = random_point_in_unit_square_wang_hash(seed);
      vec3 extent = light.p1.xyz - light.p0.xyz;

      if (light.p1[3] == 0) { // yz plane
        dest_point = light.p0.xyz + extent * vec3(0, rand[0], rand[1]);
        area = extent.y * extent.z;
        normal_light = vec3(1.f, 0.f, 0.f);
      } else if (light.p1[3] == 1) { // xz plane
        dest_point = light.p0.xyz + extent * vec3(rand[0], 0, rand[1]);
        area = extent.x * extent.z;
        normal_light = vec3(0.f, 1.f, 0.f);
      } else { // xy plane
        dest_point = light.p0.xyz + extent * vec3(rand[0], rand[1], 0);
        area = extent.x * extent.y;
        normal_light = vec3(0.f, 0.f, 1.f);
      }
      direction = normalize(dest_point - origin);
    }
    float solid_angle =
      (area * dot(normal_light, direction)) / dot(direction, direction);
    pdfs[i] = vec4(dest_point.xyz, 1 / solid_angle);

    pdf_total += pdfs[i].w;
  }

  for (int i = 0; i < light_count; i++) {
    pdfs[i].w = pdfs[i].w / pdf_total;
  }

  float rand = rand_wang_hash(seed);// * pdf_total;
  float min_rand = pdfs[0].w;
  int index = 0;

  while (index < light_count - 1) {
    if (rand <= min_rand) {
      break;
    }
    min_rand += pdfs[++index].w;
  }
  light_index = index;
  return pdfs[index];
}
#endif

// Vertex shader inputs
#define V_SCREEN_COORD_LOCATION 0
// Fragment shader inputs
#define F_UV_LOCATION 0
#define F_IMAGE_SAMPLER_LOCATION 0
// Framebuffer inputs
#define FB_COLOR_LOCATION 0

// Ray Generation inputs
#define RG_RAY_CAMERA_BINDING 0
#define RG_UV_LOCATION 0
// Ray Generation outputs
#define RG_OUT_RAY_ORIGIN_LOCATION 0
#define RG_OUT_RAY_DIRECTION_LOCATION 1
#define RG_OUT_ENERGY_ACCUMULATION_LOCATION 2
#define RG_OUT_ENERGY_ATTENUATION_LOCATION 3
#define RG_OUT_SHADOW_RAY_DIRECTION_LOCATION 4
#define RG_OUT_SHADOW_RAY_DATA_LOCATION 5

// Scene Traversal inputs
#define ST_OBJECT_BINDING 0
#define ST_EARLY_OUT_BINDING 1
#define ST_IN_RAY_ORIGIN_LOCATION 0
#define ST_IN_RAY_DIRECTION_LOCATION 1
#define ST_IN_PREVIOUS_HIT_RECORD_0_LOCATION 2
#define ST_IN_PREVIOUS_HIT_RECORD_1_LOCATION 3
#define ST_IN_PREVIOUS_HIT_RECORD_2_LOCATION 4
#define ST_IN_PREVIOUS_HIT_RECORD_3_LOCATION 5
#define ST_IN_PREVIOUS_HIT_RECORD_4_LOCATION 6
#define ST_IN_PREVIOUS_HIT_RECORD_5_LOCATION 7
#define ST_TRIANGLES_IN_VERTEX_POSITIONS_LOCATION 8
#define ST_TRIANGLES_IN_VERTEX_NORMALS_LOCATION 9
#define ST_TRIANGLES_IN_VERTEX_TANGENTS_LOCATION 10
#define ST_TRIANGLES_IN_VERTEX_UVS_LOCATION 11
#define ST_TRIANGLES_IN_BVH_LOCATION 12
#define ST_TRIANGLES_IN_INDICES_LOCATION 13

// Any Hit inputs
#define AH_HIT_RECORD_0_LOCATION 0
#define AH_HIT_RECORD_1_LOCATION 1
#define AH_HIT_RECORD_2_LOCATION 2
#define AH_HIT_RECORD_3_LOCATION 3
#define AH_HIT_RECORD_4_LOCATION 4
#define AH_HIT_RECORD_5_LOCATION 5
#define AH_INCIDENT_RAY_ORIGIN_LOCATION 6
#define AH_INCIDENT_RAY_DIRECTION_LOCATION 7
#define AH_IN_ENERGY_ACCUMULATION_LOCATION 8
#define AH_IN_ENERGY_ATTENUATION_LOCATION 9
#define AH_UNIFORM_BINDING 0

// Shadow Ray inputs
#define SR_HIT_RECORD_5_LOCATION 0
#define SR_INCIDENT_RAY_ORIGIN_LOCATION 1
#define SR_INCIDENT_RAY_DIRECTION_LOCATION 2
#define SR_NEXT_RAY_DIRECTION_LOCATION 3
#define SR_IN_ENERGY_ACCUMULATION_LOCATION 4
#define SR_IN_ENERGY_ATTENUATION_LOCATION 5
#define SR_IN_DATA_LOCATION 6 
#define SR_UNIFORM_BINDING 0

// Energy Accumulation input
#define EA_IN_CURRENT_ENERGY_LOCATION 0
#define EA_IN_PREVIOUS_ENERGY_LOCATION 1

#endif // WHITTED_RAYTRACING_BRIDGING_HEADER_H
