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
layout(binding = MA_IN_COLOR_LOCATION) uniform sampler2D ma_color;

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

    // Ray is inactive
    if (rec.status != HIT_RECORD_STATUS_MISS)
    {
        ah_out_color = texelFetch(ma_color, iid, 0);
        return;
    }

    // Draw sky
    const vec4 top = vec4(0.5, 0.7, 1.0, 1.0f);
    const vec4 bot = vec4(1.0, 1.0, 1.0, 1.0f);

    vec4 ray_direction = texelFetch(ah_incident_ray_direction, iid, 0);

    ah_out_color = mix(top, bot, 0.5f * (-ray_direction.y + 1.0f));
    ah_out_color.a = 1.0f / uniform_block.data.frame_count;
}
