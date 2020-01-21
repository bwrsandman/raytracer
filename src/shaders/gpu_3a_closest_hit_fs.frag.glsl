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
layout(binding = AH_HIT_RECORD_5_LOCATION) uniform sampler2D ah_hit_record_5;  // status (x), mat_id (y), bvh_hits (z) // TODO: Maybe move bvh_hits to hit_record 0
layout(binding = AH_INCIDENT_RAY_ORIGIN_LOCATION) uniform sampler2D ah_incident_ray_origin;
layout(binding = AH_INCIDENT_RAY_DIRECTION_LOCATION) uniform sampler2D ah_incident_ray_direction;
layout(binding = AH_IN_ENERGY_ACCUMULATION_LOCATION) uniform sampler2D ah_in_energy_accumulation;

layout(location = RG_OUT_RAY_ORIGIN_LOCATION) out vec4 rg_out_ray_origin;
layout(location = RG_OUT_RAY_DIRECTION_LOCATION) out vec4 rg_out_ray_direction;
layout(location = RG_OUT_ENERGY_ACCUMULATION_LOCATION) out vec4 rg_out_energy_accumulation;
layout(location = RG_OUT_SHADOW_RAY_DIRECTION_LOCATION) out vec4 rg_out_shadow_ray_direction;
layout(location = RG_OUT_SHADOW_RAY_DATA_LOCATION) out vec4 rg_out_shadow_ray_data;

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

    vec4 material_data = uniform_block.data.material_data[rec.mat_id];
    uint material_type = uint(material_data.a);

    if (material_type == MATERIAL_TYPE_METAL) {
        rg_out_energy_accumulation.xyz = material_data.xyz * energy_accumulation.xyz;
        rg_out_ray_origin.xyz = rec.position + FLT_EPSILON * rec.normal;
        rg_out_ray_direction.xyz = reflect(ray_direction.xyz, rec.normal);
        rg_out_ray_direction.w = RAY_STATUS_ACTIVE;
    } else if (material_type == MATERIAL_TYPE_LAMBERT) {
        rg_out_energy_accumulation.xyz = material_data.xyz * energy_accumulation.xyz;
        rg_out_ray_origin.xyz = rec.position + FLT_EPSILON * rec.normal;
        rg_out_ray_direction.xyz = random_point_on_unit_hemisphere_wang_hash(seed, rec.normal);
        rg_out_ray_direction.w = RAY_STATUS_ACTIVE;
        if (uniform_block.data.light_count > 0) {
            uint light_index = uint(uniform_block.data.light_count * rand_wang_hash(seed));
            vec4 position = uniform_block.data.light_position_data[light_index];
            rg_out_shadow_ray_direction.xyz = position.xyz;
            rg_out_shadow_ray_direction.w = RAY_STATUS_ACTIVE;
            rg_out_shadow_ray_data.x = length(position.xyz - rec.position) - 2 * FLT_EPSILON;
            rg_out_shadow_ray_data.y = uint(light_index);
        }
    } else if (material_type == MATERIAL_TYPE_DIELECTRIC) {
        float ref_idx = material_data.r;
      bool inside_dielectric = false;
        rg_out_energy_accumulation.xyz = energy_accumulation.xyz;
        rg_out_ray_direction.xyz = material_dielectric_scatter(
        seed, ray_direction.xyz, rec.normal, ref_idx, inside_dielectric);
        rg_out_ray_origin.xyz = rec.position + FLT_EPSILON * rg_out_ray_direction.xyz;
        rg_out_ray_direction.w = RAY_STATUS_ACTIVE;

		// Beer's law
        if (inside_dielectric) {
          vec4 ray_origin = texelFetch(ah_incident_ray_direction, iid, 0);
          float dist = -distance(rg_out_ray_origin.xyz, ray_origin.xyz);

          // unscramble floatshift x*1.000.000 + y*1.000 + z
          vec3 albedo = vec3(float(int(material_data.z) / 1000000), 
                             float(mod(int(material_data.z) / 1000,1000)), 
                             mod(material_data.z ,1000.f));

          vec3 absorb = vec3(exp(albedo.x * dist), exp(albedo.y * dist), exp(albedo.z * dist));
          rg_out_energy_accumulation.xyz = rg_out_energy_accumulation.xyz * absorb.xyz;
		}
    } else {
        rg_out_energy_accumulation = energy_accumulation * vec4(mod(rec.uv.x, 0.1f) > 0.05f ^^ mod(rec.uv.y, 0.1f) > 0.05f);
        rg_out_ray_origin.xyz = rec.position + FLT_EPSILON * rec.normal;
        rg_out_ray_direction.xyz = random_point_on_unit_hemisphere_wang_hash(seed, rec.normal);
        rg_out_ray_direction.w = RAY_STATUS_ACTIVE;
    }
    rg_out_energy_accumulation.a = 1.0f / uniform_block.data.frame_count;




    }
