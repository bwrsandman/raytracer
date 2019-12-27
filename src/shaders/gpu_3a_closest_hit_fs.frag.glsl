#version 460

#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable
#extension GL_ARB_shading_language_420pack : enable

#include "bridging_header.h"
#include "hit_record_t.h"

layout(binding = AH_HIT_RECORD_0_LOCATION) uniform sampler2D ah_hit_record_0;  // t, position
layout(binding = AH_HIT_RECORD_1_LOCATION) uniform sampler2D ah_hit_record_1;  // normal, u
layout(binding = AH_HIT_RECORD_2_LOCATION) uniform sampler2D ah_hit_record_2;  // tangent, v
layout(binding = AH_HIT_RECORD_3_LOCATION) uniform usampler2D ah_hit_record_3;  // status, mat_id, bvh_hits
layout(binding = AH_INCIDENT_RAY_ORIGIN_LOCATION) uniform sampler2D ah_incident_ray_origin;
layout(binding = AH_INCIDENT_RAY_DIRECTION_LOCATION) uniform sampler2D ah_incident_ray_direction;

layout(location = AH_OUT_COLOR_LOCATION) out vec4 ah_out_color;

layout (binding = AH_UNIFORM_BINDING, std140) uniform uniform_block_t {
    anyhit_uniform_data_t data;
} uniform_block;

void main() {
    ivec2 iid = ivec2(gl_FragCoord.xy);

    hit_record_t rec;
    hit_record_deserialize(rec,
                           texelFetch(ah_hit_record_0, iid, 0),
                           texelFetch(ah_hit_record_1, iid, 0),
                           texelFetch(ah_hit_record_2, iid, 0),
                           texelFetch(ah_hit_record_3, iid, 0));

    if (rec.status != HIT_RECORD_STATUS_HIT)
    {
        discard;
    }

    if (rec.mat_id == 0) {
        ah_out_color.xyz = vec3(1, 0, 0);
    } else if (rec.mat_id == 1) {
        ah_out_color.xyz = vec3(0, 1, 0);
    } else if (rec.mat_id == 7) {
        ah_out_color.xyz = vec3(0, 0, 1);
    } else if (rec.mat_id == 8) {
        ah_out_color.xyz = vec3(0, 1, 1);
    } else if (rec.mat_id == 9) {
        ah_out_color.xyz = vec3(1, 1, 0);
    } else if (rec.mat_id == 10) {
        ah_out_color.xyz = vec3(1, 0, 1);
    } else {
        ah_out_color = vec4(mod(rec.uv.x, 0.1f) > 0.05f ^^ mod(rec.uv.y, 0.1f) > 0.05f);
    }
    ah_out_color.a = 1.0f / uniform_block.data.frame_count;
}
