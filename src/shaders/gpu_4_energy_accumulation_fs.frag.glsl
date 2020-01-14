#version 460

#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "bridging_header.h"

layout(location = F_UV_LOCATION) in vec2 f_uv;
layout(binding = EA_IN_CURRENT_ENERGY_LOCATION) uniform sampler2D ea_in_current_energy;
layout(binding = EA_IN_PREVIOUS_ENERGY_LOCATION) uniform sampler2D ea_in_previous_energy;
layout(location = FB_COLOR_LOCATION) out vec4 fb_color;

void main() {
    ivec2 iid = ivec2(gl_FragCoord.xy);

    vec4 current_energy = texelFetch(ea_in_current_energy, iid, 0);
    vec4 previous_energy = texelFetch(ea_in_previous_energy, iid, 0);
    fb_color = mix(previous_energy, current_energy, current_energy.w);
    fb_color.w = 1.0f;
}
