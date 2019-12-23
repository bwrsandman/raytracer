#version 460

#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable
#extension GL_ARB_shading_language_420pack : enable

#include "bridging_header.h"
#include "hit_record_t.h"
#include "sphere_t.h"

layout(binding = ST_RAY_ORIGIN_LOCATION) uniform sampler2D st_ray_origin;
layout(binding = ST_RAY_DIRECTION_LOCATION) uniform sampler2D st_ray_direction;

layout(location = AH_HIT_RECORD_0_LOCATION) out vec4 ah_hit_record_0;  // t, position
layout(location = AH_HIT_RECORD_1_LOCATION) out vec4 ah_hit_record_1;  // normal, u
layout(location = AH_HIT_RECORD_2_LOCATION) out vec4 ah_hit_record_2;  // tangent, v
layout(location = AH_HIT_RECORD_3_LOCATION) out uvec4 ah_hit_record_3;  // status, mat_id, bvh_hits
layout(location = AH_INCIDENT_RAY_ORIGIN_LOCATION) out vec4 ah_incident_ray_origin;
layout(location = AH_INCIDENT_RAY_DIRECTION_LOCATION) out vec4 ah_incident_ray_direction;

void main() {
    ivec2 iid = ivec2(gl_FragCoord.xy);

    ray_t ray;
    ray.origin = texelFetch(st_ray_origin, iid, 0);
    ray.direction = texelFetch(st_ray_direction, iid, 0);

    sphere_t sphere;
    sphere.center = vec4(0, 0, -1, 1);
    sphere.radius = 0.5f;

    hit_record_t rec;

    sphere_hit(ray, sphere, 0, 10000000, rec);
    hit_record_serialize(rec, ah_hit_record_0, ah_hit_record_1, ah_hit_record_2, ah_hit_record_3);

    ah_incident_ray_origin = ray.origin;
    ah_incident_ray_direction = ray.direction;
}
