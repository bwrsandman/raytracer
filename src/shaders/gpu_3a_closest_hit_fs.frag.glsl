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

  if (rec.status != HIT_RECORD_STATUS_HIT) {
    discard;
  }

  vec4 ray_origin = texelFetch(ah_incident_ray_origin, iid, 0);
  vec4 ray_direction = texelFetch(ah_incident_ray_direction, iid, 0);
  vec4 energy_accumulation = texelFetch(ah_in_energy_accumulation, iid, 0);
  rg_out_energy_accumulation = energy_accumulation;

  if (ray_direction.w == RAY_STATUS_DEAD) {
    return;
  }

  vec4 energy_attenuation = texelFetch(ah_in_energy_attenuation, iid, 0);
  vec4 material_data = uniform_block.data.material_data[rec.mat_id];
  uint material_type = uint(material_data.a);
  rg_out_ray_origin.w = NORMAL_RAY;

  if (material_type == MATERIAL_TYPE_METAL) 
  {
    rg_out_energy_attenuation.xyz = material_data.xyz * energy_attenuation.xyz;
    rg_out_ray_origin.xyz = rec.position + FLT_EPSILON * rec.normal;
    rg_out_ray_direction.xyz = reflect(ray_direction.xyz, rec.normal);
    rg_out_ray_direction.w = RAY_STATUS_ACTIVE;
  } 
  else if (material_type == MATERIAL_TYPE_LAMBERT) 
  {
    rg_out_energy_attenuation.xyz = material_data.xyz * energy_attenuation.xyz;
    rg_out_ray_origin.xyz = rec.position + FLT_EPSILON * rec.normal;
    rg_out_ray_origin.w = NEE_RANDOM_RAY;
    rg_out_ray_direction.xyz =
      random_point_on_unit_hemisphere_wang_hash(seed, rec.normal);
    rg_out_ray_direction.w = RAY_STATUS_ACTIVE;
    if (uniform_block.data.light_count > 0) {
    
//      uint light_index =
//        uint(uniform_block.data.light_count * rand_wang_hash(seed));
//
//      vec3 point = random_point_on_light(
//        uniform_block.data.lights[light_index], rec.position, seed);
//      vec3 l = point - rec.position;

      uint light_index = 0;
      vec4 random_light =
        random_light_destination(uniform_block.data.lights,
                                 uniform_block.data.light_count,
                                 rec.position,
                                 seed,
                                 light_index);
      rg_out_energy_attenuation.w = random_light.w;
      vec3 l = random_light.xyz - rec.position;

      rg_out_shadow_ray_direction.xyz = normalize(l);
      rg_out_shadow_ray_direction.w = RAY_STATUS_ACTIVE;
      rg_out_shadow_ray_data.x = length(l) - 20 * FLT_EPSILON;
      rg_out_shadow_ray_data.y = uint(light_index);
      rg_out_shadow_ray_data.z = dot(rec.normal, l);
      rg_out_shadow_ray_data.w = dot(l, l);
    }
  } 
  else if (material_type == MATERIAL_TYPE_DIELECTRIC) 
  {
    float ref_idx = material_data.r;
    bool inside_dielectric = false;
    rg_out_energy_attenuation.xyz = energy_attenuation.xyz;
    rg_out_ray_direction.xyz = material_dielectric_scatter(
      seed, ray_direction.xyz, rec.normal, ref_idx, inside_dielectric);
    rg_out_ray_origin.xyz =
      rec.position + FLT_EPSILON * rg_out_ray_direction.xyz;
    rg_out_ray_direction.w = RAY_STATUS_ACTIVE;

    // Beer's law
    if (inside_dielectric) {
      float dist = -distance(rg_out_ray_origin.xyz, ray_origin.xyz);

      // unscramble floatshift x*1.000.000 + y*1.000 + z
      vec3 albedo = vec3(float(int(material_data.z) / 1000000),
                         float(mod(int(material_data.z) / 1000, 1000)),
                         mod(material_data.z, 1000.f));

      vec3 absorb =
        vec3(exp(albedo.x * dist), exp(albedo.y * dist), exp(albedo.z * dist));
      rg_out_energy_attenuation.xyz = energy_attenuation.xyz * absorb.xyz;
    }
  } 
  else if (material_type == MATERIAL_TYPE_EMISSIVE) 
  {
    rg_out_energy_attenuation.xyz = energy_attenuation.xyz;

    if (ray_origin.w == NEE_RANDOM_RAY) {
      rg_out_energy_accumulation.xyz = energy_accumulation.xyz;
      rg_out_ray_direction.w = RAY_STATUS_DEAD;
    } else {

      rg_out_energy_accumulation.xyz =
        energy_accumulation.xyz + energy_attenuation.xyz * material_data.xyz;

	 // vec3 l = normalize(rec.position - ray_origin.xyz);
     // rg_out_energy_accumulation.rgb =
     //   uniform_block.data.light_count * energy_accumulation.rgb +
     //   (dot(rec.normal, l) * energy_attenuation.rgb * material_data.xyz) /
     //     dot(l, l);
      rg_out_ray_direction.w = RAY_STATUS_DEAD;
    }
  } 
  else 
  {
    rg_out_energy_attenuation =
      energy_attenuation *
      vec4(mod(rec.uv.x, 0.1f) > 0.05f ^^mod(rec.uv.y, 0.1f) > 0.05f);
    rg_out_ray_origin.xyz = rec.position + FLT_EPSILON * rec.normal;
    rg_out_ray_direction.xyz =
      random_point_on_unit_hemisphere_wang_hash(seed, rec.normal);
    rg_out_ray_direction.w = RAY_STATUS_ACTIVE;
  }
  rg_out_energy_accumulation.a = 1.0f / uniform_block.data.frame_count;
}
