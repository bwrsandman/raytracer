#version 460

#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable
#extension GL_ARB_shading_language_420pack : enable

#include "bridging_header.h"
#include "hit_record_t.h"
#include "plane_t.h"

layout(binding = ST_IN_RAY_ORIGIN_LOCATION) uniform sampler2D st_in_ray_origin;
layout(binding = ST_IN_RAY_DIRECTION_LOCATION) uniform sampler2D st_in_ray_direction;
layout(binding = ST_IN_PREVIOUS_HIT_RECORD_0_LOCATION) uniform sampler2D st_in_previous_hit_record_0; // t, position

layout(location = AH_HIT_RECORD_0_LOCATION) out vec4 ah_hit_record_0;  // t (x)
layout(location = AH_HIT_RECORD_1_LOCATION) out vec4 ah_hit_record_1;  // position (xyz)
layout(location = AH_HIT_RECORD_2_LOCATION) out vec4 ah_hit_record_2;  // uv (xy)
layout(location = AH_HIT_RECORD_3_LOCATION) out vec4 ah_hit_record_3;  // normal (xyz)
layout(location = AH_HIT_RECORD_4_LOCATION) out vec4 ah_hit_record_4;  // tangent (xyz)
layout(location = AH_HIT_RECORD_5_LOCATION) out uvec4 ah_hit_record_5;  // status (x), mat_id (y), bvh_hits (z) // TODO: Maybe move bvh_hits to hit_record 0
layout(location = AH_INCIDENT_RAY_ORIGIN_LOCATION) out vec4 ah_incident_ray_origin;
layout(location = AH_INCIDENT_RAY_DIRECTION_LOCATION) out vec4 ah_incident_ray_direction;

layout (binding = ST_OBJECT_BINDING, std140) uniform uniform_block_t {
    scene_traversal_plane_uniform_t objects;
} uniform_block;

void main() {
    ivec2 iid = ivec2(gl_FragCoord.xy);

    ray_t ray;
    ray.origin = texelFetch(st_in_ray_origin, iid, 0);
    ray.direction = texelFetch(st_in_ray_direction, iid, 0);

    if (ray.direction.w == RAY_STATUS_DEAD)
    {
        discard;
    }

    float t_max = texelFetch(st_in_previous_hit_record_0, iid, 0).x;

    hit_record_t rec;
    rec.status = HIT_RECORD_STATUS_MISS;
    rec.t = t_max;

    for (uint i = 0; i < uniform_block.objects.count; ++i)
    {
        plane_t plane;
        plane_deserialize(uniform_block.objects.min[i],
                          uniform_block.objects.max[i],
                          uniform_block.objects.normal[i],
                          uniform_block.objects.materials[i],
                          plane);

        hit_record_t temp_rec;

        plane_hit(ray, plane, T_MIN, rec.t - FLT_EPSILON, temp_rec);
        if (temp_rec.status == HIT_RECORD_STATUS_HIT && rec.t > temp_rec.t) {
            rec = temp_rec;
        }
    }

    if (rec.status == HIT_RECORD_STATUS_MISS)
    {
        discard;
    }

    hit_record_serialize(rec,
                         ah_hit_record_0,
                         ah_hit_record_1,
                         ah_hit_record_2,
                         ah_hit_record_3,
                         ah_hit_record_4,
                         ah_hit_record_5);

    ah_incident_ray_origin = ray.origin;
    ah_incident_ray_direction = ray.direction;
}