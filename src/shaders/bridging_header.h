#ifndef WHITTED_RAYTRACING_BRIDGING_HEADER_H
#define WHITTED_RAYTRACING_BRIDGING_HEADER_H

#if __cplusplus
using namespace Raytracer::Math;
#define ALIGN16 alignas(16)
#else
#define ALIGN16
#define M_PI 3.14159265358979323846       /* pi */
#define M_PI_2 1.57079632679489661923     /* pi/2 */
#define M_PI_4 0.78539816339744830962     /* pi/4 */
#define M_1_PI 0.31830988618379067154     /* 1/pi */
#define M_1_2PI 0.15915494309189533576    /* 1/2pi */
#define M_2_PI 0.63661977236758134308     /* 2/pi */
#define M_2_SQRTPI 1.12837916709551257390 /* 2/sqrt(pi) */
#define M_SQRT2 1.41421356237309504880    /* sqrt(2) */
#define M_SQRT1_2 0.70710678118654752440  /* 1/sqrt(2) */
#endif

/// This is a header which is read by both the gpu shader code and the cpu
/// c++ code.
/// The purpose of this header is to avoid double declarations.

struct camera_uniform_t
{
  ALIGN16 vec3 origin;
  ALIGN16 vec3 lower_left_corner;
  ALIGN16 vec3 horizontal;
  ALIGN16 vec3 vertical;
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
#define ST_RAY_ORIGIN_LOCATION 0
#define ST_RAY_DIRECTION_LOCATION 1

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
