#ifndef WHITTED_RAYTRACING_BRIDGING_HEADER_H
#define WHITTED_RAYTRACING_BRIDGING_HEADER_H

#if __cplusplus
using namespace Raytracer::Math;
#define ALIGN16 alignas(16)
#else
#define ALIGN16
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

// Ray Generation inputs
#define RG_RAY_CAMERA_BINDING 0
#define RG_UV_LOCATION 0
// Ray Generation outputs
#define RG_OUT_RAY_ORIGIN_LOCATION 0
#define RG_OUT_RAY_DIRECTION_LOCATION 1

// Framebuffer inputs
#define FB_COLOR_LOCATION 0

#endif // WHITTED_RAYTRACING_BRIDGING_HEADER_H
