#version 460

#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable
#extension GL_ARB_shading_language_420pack : enable

#include "gpu_2_layout.h"
#include "bvh_node_t.h"
#include "hit_record_t.h"
#include "triangle_t.h"

layout(binding = ST_TRIANGLES_IN_VERTEX_POSITIONS_LOCATION) uniform sampler2D
st_triangles_in_vertex_positions;
layout(binding = ST_TRIANGLES_IN_VERTEX_NORMALS_LOCATION) uniform sampler2D
st_triangles_in_vertex_normals;
layout(binding = ST_TRIANGLES_IN_VERTEX_TANGENTS_LOCATION) uniform sampler2D
st_triangles_in_vertex_tangents;
layout(binding = ST_TRIANGLES_IN_VERTEX_UVS_LOCATION) uniform sampler2D
st_triangles_in_vertex_uvs;
layout(binding = ST_TRIANGLES_IN_BVH_LOCATION) uniform sampler2D
st_triangles_in_bvh;
layout(binding = ST_TRIANGLES_IN_INDICES_LOCATION) uniform usampler2D
st_triangles_in_indices;

layout (binding = ST_OBJECT_BINDING, std140) uniform uniform_block_t {
    scene_traversal_triangle_uniform_t objects;
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
    vec4 old_ah_hit_record_5 = texelFetch(st_in_previous_hit_record_5, iid, 0);

    hit_record_t rec;
    rec.status = HIT_RECORD_STATUS_MISS;
    rec.t = t_max;
    rec.bvh_hits = uint(old_ah_hit_record_5.z);

    bvh_node_t root_node;
    bvh_node_deserialize(texelFetch(st_triangles_in_bvh, ivec2(0, 0), 0),
                         texelFetch(st_triangles_in_bvh, ivec2(0, 1), 0),
                         root_node);

    if (root_node.offset != 0 || bvh_node_is_leaf(root_node)) {
        bvh_node_t nodes_to_visit[MAX_NUM_BVH_NODES];
        uint start = 0;
        uint end = 0;
        nodes_to_visit[end] = root_node;
        end += 1;

        bool break_out = false;

        while (start < end) {
            bvh_node_t node = nodes_to_visit[start];
            start += 1;
            if (aabb_hit(node.bounds, ray, T_MIN, rec.t - FLT_EPSILON)) {
                rec.bvh_hits += 1;
                if (bvh_node_is_leaf(node)) {
                    for (uint i = 0; i < node.count / 3; i++)
                    {
                        uvec3 indices = texelFetch(st_triangles_in_indices, ivec2(node.offset / 3 + i, 0), 0).xyz;

                        vec4 position0 = texelFetch(st_triangles_in_vertex_positions, ivec2(indices[0], 0), 0);
                        vec4 position1 = texelFetch(st_triangles_in_vertex_positions, ivec2(indices[1], 0), 0);
                        vec4 position2 = texelFetch(st_triangles_in_vertex_positions, ivec2(indices[2], 0), 0);
                        vec4 normal0 = texelFetch(st_triangles_in_vertex_normals, ivec2(indices[0], 0), 0);
                        vec4 normal1 = texelFetch(st_triangles_in_vertex_normals, ivec2(indices[1], 0), 0);
                        vec4 normal2 = texelFetch(st_triangles_in_vertex_normals, ivec2(indices[2], 0), 0);
                        vec2 uv0 = texelFetch(st_triangles_in_vertex_uvs, ivec2(indices[0], 0), 0).xy;
                        vec2 uv1 = texelFetch(st_triangles_in_vertex_uvs, ivec2(indices[1], 0), 0).xy;
                        vec2 uv2 = texelFetch(st_triangles_in_vertex_uvs, ivec2(indices[2], 0), 0).xy;

                        triangle_t triangle;
                        triangle_deserialize(position0, position1, position2,
                                             normal0, normal1, normal2,
                                             uv0, uv1, uv2,
                                             uniform_block.objects.mat_id,
                                             triangle);

                        hit_record_t temp_rec;

                        triangle_hit(triangle, ray, T_MIN, rec.t - FLT_EPSILON, temp_rec);
                        if (temp_rec.status == HIT_RECORD_STATUS_HIT && rec.t > temp_rec.t) {
                            rec = temp_rec;
                            if (common_uniform_block.data.early_out == 1) {
                                break_out = true;
                                break;
                            }
                        }
                    }
                } else {
                    bvh_node_deserialize(texelFetch(st_triangles_in_bvh, ivec2(node.offset, 0), 0),
                                         texelFetch(st_triangles_in_bvh, ivec2(node.offset, 1), 0),
                                         nodes_to_visit[end]);
                    bvh_node_deserialize(texelFetch(st_triangles_in_bvh, ivec2(node.offset + 1, 0), 0),
                                         texelFetch(st_triangles_in_bvh, ivec2(node.offset + 1, 1), 0),
                                         nodes_to_visit[end + 1]);
                    end += 2;
                }
            }
            if (break_out) {
                break;
            }
        }
    }

    old_ah_hit_record_5.z = float(rec.bvh_hits);

    if (rec.status == HIT_RECORD_STATUS_MISS)
    {
        ah_hit_record_0 = vec4(t_max, 0, 0, 1);
        ah_hit_record_1 = texelFetch(st_in_previous_hit_record_1, iid, 0);
        ah_hit_record_2 = texelFetch(st_in_previous_hit_record_2, iid, 0);
        ah_hit_record_3 = texelFetch(st_in_previous_hit_record_3, iid, 0);
        ah_hit_record_4 = texelFetch(st_in_previous_hit_record_4, iid, 0);
        ah_hit_record_5 = old_ah_hit_record_5;
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
