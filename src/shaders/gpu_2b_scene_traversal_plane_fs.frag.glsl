#version 460

#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable
#extension GL_ARB_shading_language_420pack : enable

#include "gpu_2_layout.h"
#include "hit_record_t.h"
#include "plane_t.h"

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
        ah_hit_record_0 = vec4(t_max, 0, 0, 1);
        ah_hit_record_1 = texelFetch(st_in_previous_hit_record_1, iid, 0);
        ah_hit_record_2 = texelFetch(st_in_previous_hit_record_2, iid, 0);
        ah_hit_record_3 = texelFetch(st_in_previous_hit_record_3, iid, 0);
        ah_hit_record_4 = texelFetch(st_in_previous_hit_record_4, iid, 0);
        ah_hit_record_5 = texelFetch(st_in_previous_hit_record_5, iid, 0);
        return;
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
