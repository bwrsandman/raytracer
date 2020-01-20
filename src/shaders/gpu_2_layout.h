#ifndef RAYTRACER_GPU_SHADERS_GPU_2_COMMON_H_
#define RAYTRACER_GPU_SHADERS_GPU_2_COMMON_H_

#include "bridging_header.h"

layout(binding = ST_IN_RAY_ORIGIN_LOCATION) uniform sampler2D st_in_ray_origin;
layout(binding = ST_IN_RAY_DIRECTION_LOCATION) uniform sampler2D
  st_in_ray_direction;
layout(binding = ST_IN_PREVIOUS_HIT_RECORD_0_LOCATION) uniform sampler2D
  st_in_previous_hit_record_0;
layout(binding = ST_IN_PREVIOUS_HIT_RECORD_1_LOCATION) uniform sampler2D
  st_in_previous_hit_record_1;
layout(binding = ST_IN_PREVIOUS_HIT_RECORD_2_LOCATION) uniform sampler2D
  st_in_previous_hit_record_2;
layout(binding = ST_IN_PREVIOUS_HIT_RECORD_3_LOCATION) uniform sampler2D
  st_in_previous_hit_record_3;
layout(binding = ST_IN_PREVIOUS_HIT_RECORD_4_LOCATION) uniform sampler2D
  st_in_previous_hit_record_4;
layout(binding = ST_IN_PREVIOUS_HIT_RECORD_5_LOCATION) uniform sampler2D
  st_in_previous_hit_record_5;

layout(location = AH_HIT_RECORD_0_LOCATION) out vec4 ah_hit_record_0; // t (x)
layout(location = AH_HIT_RECORD_1_LOCATION) out vec4
  ah_hit_record_1; // position (xyz)
layout(location = AH_HIT_RECORD_2_LOCATION) out vec4 ah_hit_record_2; // uv (xy)
layout(location = AH_HIT_RECORD_3_LOCATION) out vec4
  ah_hit_record_3; // normal (xyz)
layout(location = AH_HIT_RECORD_4_LOCATION) out vec4
  ah_hit_record_4; // tangent (xyz)
layout(location = AH_HIT_RECORD_5_LOCATION) out vec4
  ah_hit_record_5; // status (x), mat_id (y), bvh_hits (z) // TODO: Maybe move
                   // bvh_hits to hit_record 0
layout(location = AH_INCIDENT_RAY_ORIGIN_LOCATION) out vec4
  ah_incident_ray_origin;
layout(location = AH_INCIDENT_RAY_DIRECTION_LOCATION) out vec4
  ah_incident_ray_direction;

#endif // RAYTRACER_GPU_SHADERS_GPU_2_COMMON_H_
