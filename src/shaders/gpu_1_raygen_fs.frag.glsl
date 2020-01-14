#version 460

#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "bridging_header.h"

layout(location = RG_UV_LOCATION) in vec2 f_uv;

layout(location = RG_OUT_RAY_ORIGIN_LOCATION) out vec4 rg_out_ray_origin;
layout(location = RG_OUT_RAY_DIRECTION_LOCATION) out vec4 rg_out_ray_direction;
layout(location = RG_OUT_ENERGY_ACCUMULATION_LOCATION) out vec4 rg_out_energy_accumulation;

layout (binding = RG_RAY_CAMERA_BINDING, std140) uniform uniform_block_t {
    raygen_uniform_t data;
} uniform_block;

void main() {
    ivec2 iid = ivec2(gl_FragCoord.xy);
    uint seed = rand_seed(iid.x + iid.y * uniform_block.data.width,
                          uniform_block.data.frame_count);

    vec3 direction = uniform_block.data.camera.lower_left_corner +
                     f_uv.x * uniform_block.data.camera.horizontal +
                     f_uv.y * uniform_block.data.camera.vertical -
                     uniform_block.data.camera.origin;

    direction = normalize(direction);
    direction.y *= -1.0f;

    rg_out_ray_origin = vec4(uniform_block.data.camera.origin, 1);
    rg_out_ray_direction = vec4(direction, RAY_STATUS_ACTIVE);
    rg_out_energy_accumulation = vec4(1, 1, 1, 0);
}
