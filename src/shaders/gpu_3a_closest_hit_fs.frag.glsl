#version 460

#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable
#extension GL_ARB_shading_language_420pack : enable

#include "bridging_header.h"
#include "hit_record_t.h"

layout(binding = AH_HIT_RECORD_0_LOCATION) uniform sampler2D ah_hit_record_0;  // t (x)
layout(binding = AH_HIT_RECORD_1_LOCATION) uniform sampler2D ah_hit_record_1;  // position (xyz)
layout(binding = AH_HIT_RECORD_2_LOCATION) uniform sampler2D ah_hit_record_2;  // uv (xy)
layout(binding = AH_HIT_RECORD_3_LOCATION) uniform sampler2D ah_hit_record_3;  // normal (xyz)
layout(binding = AH_HIT_RECORD_4_LOCATION) uniform sampler2D ah_hit_record_4;  // tangent (xyz)
layout(binding = AH_HIT_RECORD_5_LOCATION) uniform usampler2D ah_hit_record_5;  // status (x), mat_id (y), bvh_hits (z) // TODO: Maybe move bvh_hits to hit_record 0
layout(binding = AH_INCIDENT_RAY_ORIGIN_LOCATION) uniform sampler2D ah_incident_ray_origin;
layout(binding = AH_INCIDENT_RAY_DIRECTION_LOCATION) uniform sampler2D ah_incident_ray_direction;
layout(binding = AH_IN_ENERGY_ACCUMULATION_LOCATION) uniform sampler2D ah_in_energy_accumulation;

layout(location = RG_OUT_RAY_ORIGIN_LOCATION) out vec4 rg_out_ray_origin;
layout(location = RG_OUT_RAY_DIRECTION_LOCATION) out vec4 rg_out_ray_direction;
layout(location = RG_OUT_ENERGY_ACCUMULATION_LOCATION) out vec4 rg_out_energy_accumulation;

layout (binding = AH_UNIFORM_BINDING, std140) uniform uniform_block_t {
    anyhit_uniform_data_t data;
} uniform_block;

void main() {
    ivec2 iid = ivec2(gl_FragCoord.xy);
    uint seed = rand_seed(iid.x + iid.y * uniform_block.data.width,
                          uniform_block.data.frame_count);

    hit_record_t rec;
    hit_record_deserialize(rec,
                           texelFetch(ah_hit_record_0, iid, 0),
                           texelFetch(ah_hit_record_1, iid, 0),
                           texelFetch(ah_hit_record_2, iid, 0),
                           texelFetch(ah_hit_record_3, iid, 0),
                           texelFetch(ah_hit_record_4, iid, 0),
                           texelFetch(ah_hit_record_5, iid, 0));

    if (rec.status != HIT_RECORD_STATUS_HIT)
    {
        discard;
    }

    vec4 ray_direction = texelFetch(ah_incident_ray_direction, iid, 0);
    vec4 energy_accumulation = texelFetch(ah_in_energy_accumulation, iid, 0);

    if (ray_direction.w == RAY_STATUS_DEAD)
    {
        rg_out_energy_accumulation = energy_accumulation;
        return;
    }

    if (rec.mat_id == 0) {
        rg_out_energy_accumulation.xyz = energy_accumulation.xyz;
        rg_out_ray_origin.xyz = rec.position + FLT_EPSILON * rec.normal;
        rg_out_ray_direction.xyz = reflect(ray_direction.xyz, rec.normal);
        rg_out_ray_direction.w = RAY_STATUS_ACTIVE;
    } else if (rec.mat_id == 1) {
        rg_out_energy_accumulation.xyz = energy_accumulation.xyz * vec3(0.9, 0.9, 0.9);
        rg_out_ray_origin.xyz = rec.position + FLT_EPSILON * rec.normal;
        rg_out_ray_direction.xyz = random_point_on_unit_hemisphere_wang_hash(seed, rec.normal);
        rg_out_ray_direction.w = RAY_STATUS_ACTIVE;
    } else if (rec.mat_id == 7) {
        const float ref_idx = 1.5f; // TODO: Store in material
        rg_out_energy_accumulation.xyz = energy_accumulation.xyz;
        rg_out_ray_direction.xyz = material_dielectric_scatter(seed, ray_direction.xyz, rec.normal, ref_idx);
        rg_out_ray_origin.xyz = rec.position + FLT_EPSILON * rg_out_ray_direction.xyz;
        rg_out_ray_direction.w = RAY_STATUS_ACTIVE;
    } else if (rec.mat_id == 8) {
        rg_out_energy_accumulation.xyz = energy_accumulation.xyz * vec3(0, 1, 1);
    } else {
        rg_out_energy_accumulation = energy_accumulation * vec4(mod(rec.uv.x, 0.1f) > 0.05f ^^ mod(rec.uv.y, 0.1f) > 0.05f);
        rg_out_ray_origin.xyz = rec.position + FLT_EPSILON * rec.normal;
        rg_out_ray_direction.xyz = random_point_on_unit_hemisphere_wang_hash(seed, rec.normal);
        rg_out_ray_direction.w = RAY_STATUS_ACTIVE;
    }
    rg_out_energy_accumulation.a = 1.0f / uniform_block.data.frame_count;
}
