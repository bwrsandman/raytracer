#version 460

#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable
#extension GL_ARB_shading_language_420pack : enable

#include "bridging_header.h"
#include "hit_record_t.h"

layout(binding = SR_HIT_RECORD_5_LOCATION) uniform sampler2D
  sr_hit_record_5; // status (x)
layout(binding = SR_INCIDENT_RAY_ORIGIN_LOCATION) uniform sampler2D
  sr_incident_ray_origin;
layout(binding = SR_INCIDENT_RAY_DIRECTION_LOCATION) uniform sampler2D
  sr_incident_ray_direction;
layout(binding = SR_NEXT_RAY_DIRECTION_LOCATION) uniform sampler2D
  sr_next_ray_direction;
layout(binding = SR_IN_ENERGY_ACCUMULATION_LOCATION) uniform sampler2D
  sr_in_energy_accumulation;
layout(binding = SR_IN_ENERGY_ATTENUATION_LOCATION) uniform sampler2D
  sr_in_energy_attenuation;
layout(binding = SR_IN_DATA_LOCATION) uniform sampler2D sr_in_data;

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

layout(binding = SR_UNIFORM_BINDING, std140) uniform uniform_block_t
{
  shadow_ray_light_hit_uniform_data_t uniform_data;
}
uniform_block;

void
main()
{
  ivec2 iid = ivec2(gl_FragCoord.xy);

  vec4 ray_direction = texelFetch(sr_incident_ray_direction, iid, 0);
  vec4 energy_accumulation = texelFetch(sr_in_energy_accumulation, iid, 0);
  vec4 energy_attenuation = texelFetch(sr_in_energy_attenuation, iid, 0);
  vec4 data = texelFetch(sr_hit_record_5, iid, 0);

  rg_out_ray_origin = texelFetch(sr_incident_ray_origin, iid, 0);
  rg_out_ray_direction = texelFetch(sr_next_ray_direction, iid, 0);

  // View to light is not obscured
  if (data.x == HIT_RECORD_STATUS_MISS && ray_direction.w != RAY_STATUS_DEAD) {
    vec4 data = texelFetch(sr_in_data, iid, 0);
    float t = data.x;
    uint light_index = uint(data.y);

    vec4 color = uniform_block.uniform_data.light_color_data[light_index];

    float recip_pdf = energy_attenuation.w; // uniform_block.uniform_data.light_count; //

    rg_out_energy_accumulation.rgb =
      energy_accumulation.rgb +
      (data.z * recip_pdf * energy_attenuation.rgb * color.rgb) / data.w;
  } else {
    rg_out_energy_accumulation = energy_accumulation;
  }
  rg_out_energy_attenuation = energy_attenuation;
}
