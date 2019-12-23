#version 460

#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "bridging_header.h"

layout(location = RG_UV_LOCATION) in vec2 f_uv;
layout(location = RG_OUT_RAY_ORIGIN_LOCATION) out vec4 rg_out_ray_origin;
layout(location = RG_OUT_RAY_DIRECTION_LOCATION) out vec4 rg_out_ray_direction;

layout (binding = RG_RAY_CAMERA_BINDING, std140) uniform uniform_block_t {
    camera_uniform_t camera;
} uniform_block;

void main() {
    vec3 direction = uniform_block.camera.lower_left_corner +
                     f_uv.x * uniform_block.camera.horizontal +
                     f_uv.y * uniform_block.camera.vertical -
                     uniform_block.camera.origin;

    direction = normalize(direction);
    direction.y *= -1.0f;

    rg_out_ray_origin = vec4(uniform_block.camera.origin, 1);
    rg_out_ray_direction = vec4(direction, 0);
}
