#ifndef RAYTRACER_RAYTRACING_SHADER_COMMON_H
#define RAYTRACER_RAYTRACING_SHADER_COMMON_H

#define HIT_RECORD_STATUS_MISS 0
#define HIT_RECORD_STATUS_HIT 1

struct hit_record_t
{
  float t;
  vec3 p;

  vec3 normal;
  vec3 tangent;
  vec2 uv;

  uint status; // 0 no hit, 1 - hit
  uint mat_id;
  uint bvh_hits;
};

void
hit_record_serialize(hit_record_t rec,
                     out vec4 part_0,
                     out vec4 part_1,
                     out vec4 part_2,
                     out uvec4 part_3)
{
  part_0.x = rec.t;
  part_0.yzw = rec.p;
  part_1.xyz = rec.normal;
  part_1.w = rec.uv.x;
  part_2.xyz = rec.tangent;
  part_2.w = rec.uv.y;
  part_3.x = rec.status;
  part_3.y = rec.mat_id;
  part_3.z = rec.bvh_hits;
}

void
hit_record_deserialize(out hit_record_t rec,
                       vec4 part_0,
                       vec4 part_1,
                       vec4 part_2,
                       uvec4 part_3)
{
  rec.t = part_0.x;
  rec.p = part_0.yzw;
  rec.normal = part_1.xyz;
  rec.uv.x = part_1.w;
  rec.tangent = part_2.xyz;
  rec.uv.y = part_2.w;
  rec.status = part_3.x;
  rec.mat_id = part_3.y;
  rec.bvh_hits = part_3.z;
}

#endif // RAYTRACER_RAYTRACING_SHADER_COMMON_H
