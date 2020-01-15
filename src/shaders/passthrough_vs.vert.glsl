#version 460

#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "bridging_header.h"

layout(location = V_SCREEN_COORD_LOCATION) in vec2 v_screen_coord;
layout(location = F_UV_LOCATION) out vec2 f_uv;

void main()
{
    f_uv = v_screen_coord;
    gl_Position = vec4(mix(vec2(-1.0, -1.0), vec2(1.0, 1.0), v_screen_coord), 0.0, 1.0);
}
