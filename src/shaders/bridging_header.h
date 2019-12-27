#ifndef WHITTED_RAYTRACING_BRIDGING_HEADER_H
#define WHITTED_RAYTRACING_BRIDGING_HEADER_H

#if __cplusplus
using namespace Raytracer::Math;
struct uvec4
{
  uint e[4];
};
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
#endif

#include "plane_t.h"
#include "sphere_t.h"

/// This is a header which is read by both the gpu shader code and the cpu
/// c++ code.
/// The purpose of this header is to avoid double declarations.

struct alignas(64) camera_uniform_t
{
  alignas(16) vec3 origin;
  alignas(16) vec3 lower_left_corner;
  alignas(16) vec3 horizontal;
  alignas(16) vec3 vertical;
};

#define MAX_NUM_SPHERES 4

struct alignas(64) scene_traversal_sphere_uniform_t
{
  // x, y, z, radius
  vec4 spheres[MAX_NUM_SPHERES];
  // material_id, unused x3
  uvec4 materials[MAX_NUM_SPHERES];
  uint32_t count;
};

#define MAX_NUM_PLANES 4

struct alignas(64) scene_traversal_plane_uniform_t
{
  vec4 min[MAX_NUM_PLANES];
  vec4 max[MAX_NUM_PLANES];
  vec4 normal[MAX_NUM_PLANES];
  // material_id, unused x3
  uvec4 materials[MAX_NUM_PLANES];
  uint32_t count;
};

struct anyhit_uniform_data_t
{
  uint32_t frame_count;
};

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

// Scene Traversal inputs
#define ST_OBJECT_BINDING 0
#define ST_RAY_ORIGIN_LOCATION 0
#define ST_RAY_DIRECTION_LOCATION 1
#define ST_PREVIOUS_HIT_RECORD_0_LOCATION 2

// Any Hit inputs
#define AH_HIT_RECORD_0_LOCATION 0
#define AH_HIT_RECORD_1_LOCATION 1
#define AH_HIT_RECORD_2_LOCATION 2
#define AH_HIT_RECORD_3_LOCATION 3
#define AH_INCIDENT_RAY_ORIGIN_LOCATION 4
#define AH_INCIDENT_RAY_DIRECTION_LOCATION 5
#define AH_UNIFORM_BINDING 0
// Any Hit outputs
#define AH_OUT_COLOR_LOCATION 0

// Miss All inputs
#define MA_IN_COLOR_LOCATION 7

// Energy Accumulation input
#define EA_COLOR_LOCATION 0

#endif // WHITTED_RAYTRACING_BRIDGING_HEADER_H
