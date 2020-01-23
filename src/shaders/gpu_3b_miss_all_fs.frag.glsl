#version 460

#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable
#extension GL_ARB_shading_language_420pack : enable

#include "bridging_header.h"
#include "hit_record_t.h"

layout(binding = AH_HIT_RECORD_0_LOCATION) uniform sampler2D
  ah_hit_record_0; // t (x)
layout(binding = AH_HIT_RECORD_1_LOCATION) uniform sampler2D
  ah_hit_record_1; // position (xyz)
layout(binding = AH_HIT_RECORD_2_LOCATION) uniform sampler2D
  ah_hit_record_2; // uv (xy)
layout(binding = AH_HIT_RECORD_3_LOCATION) uniform sampler2D
  ah_hit_record_3; // normal (xyz)
layout(binding = AH_HIT_RECORD_4_LOCATION) uniform sampler2D
  ah_hit_record_4; // tangent (xyz)
layout(binding = AH_HIT_RECORD_5_LOCATION) uniform sampler2D
  ah_hit_record_5; // status (x), mat_id (y), bvh_hits (z) // TODO: Maybe move
                   // bvh_hits to hit_record 0
layout(binding = AH_INCIDENT_RAY_ORIGIN_LOCATION) uniform sampler2D
  ah_incident_ray_origin;
layout(binding = AH_INCIDENT_RAY_DIRECTION_LOCATION) uniform sampler2D
  ah_incident_ray_direction;
layout(binding = AH_IN_ENERGY_ACCUMULATION_LOCATION) uniform sampler2D
  ah_in_energy_accumulation;
layout(binding = AH_IN_ENERGY_ATTENUATION_LOCATION) uniform sampler2D
  ah_in_energy_attenuation;

layout(location = RG_OUT_RAY_ORIGIN_LOCATION) out vec4 rg_out_ray_origin;
layout(location = RG_OUT_RAY_DIRECTION_LOCATION) out vec4 rg_out_ray_direction;
layout(location = RG_OUT_ENERGY_ACCUMULATION_LOCATION) out vec4
  rg_out_energy_accumulation;
layout(location = RG_OUT_ENERGY_ATTENUATION_LOCATION) out vec4
  rg_out_energy_attenuation;
layout(location = RG_OUT_SHADOW_RAY_DIRECTION_LOCATION) out vec4
  rg_out_shadow_ray_direction;
layout(location = RG_OUT_SHADOW_RAY_DATA_LOCATION) out vec4
  rg_out_shadow_ray_data;

layout(binding = AH_UNIFORM_BINDING, std140) uniform uniform_block_t
{
  anyhit_uniform_data_t data;
}
uniform_block;

void
main()
{
  ivec2 iid = ivec2(gl_FragCoord.xy);

  hit_record_t rec;
  hit_record_deserialize(rec,
                         texelFetch(ah_hit_record_0, iid, 0),
                         texelFetch(ah_hit_record_1, iid, 0),
                         texelFetch(ah_hit_record_2, iid, 0),
                         texelFetch(ah_hit_record_3, iid, 0),
                         texelFetch(ah_hit_record_4, iid, 0),
                         texelFetch(ah_hit_record_5, iid, 0));

  // Ray is inactive
  if (rec.status != HIT_RECORD_STATUS_MISS) {
    discard;
  }

  vec4 ray_direction = texelFetch(ah_incident_ray_direction, iid, 0);
  vec4 energy_accumulation = texelFetch(ah_in_energy_accumulation, iid, 0);
  vec4 energy_attenuation = texelFetch(ah_in_energy_attenuation, iid, 0);
  rg_out_energy_attenuation = energy_attenuation;

  if (ray_direction.w == RAY_STATUS_DEAD) {
    rg_out_energy_accumulation = energy_accumulation;
    return;
  }

  // Draw sky
  const vec4 top = vec4(0.5, 0.7, 1.0, 1.0f);
  const vec4 bot = vec4(1.0, 1.0, 1.0, 1.0f);

  rg_out_energy_accumulation = energy_accumulation +
    energy_attenuation * mix(top, bot, 0.5f * (-ray_direction.y + 1.0f));
  rg_out_energy_accumulation.a = 1.0f / uniform_block.data.frame_count;
}
