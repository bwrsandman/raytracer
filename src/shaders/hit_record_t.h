#ifndef RAYTRACER_RAYTRACING_SHADER_COMMON_H
#define RAYTRACER_RAYTRACING_SHADER_COMMON_H

#define HIT_RECORD_STATUS_MISS 0
#define HIT_RECORD_STATUS_HIT 1

struct hit_record_t
{
  float t;

  vec3 position;
  vec2 uv;
  vec3 normal;
  vec3 tangent;

  uint status; // 0 no hit, 1 - hit
  uint mat_id;
  uint bvh_hits;
};

void
hit_record_serialize(hit_record_t rec,
                     out vec4 part_0,
                     out vec4 part_1,
                     out vec4 part_2,
                     out vec4 part_3,
                     out vec4 part_4,
                     out vec4 part_5)
{
  part_0.x = rec.t;
  part_1.xyz = rec.position;
  part_2.xy = rec.uv;
  part_3.xyz = rec.normal;
  part_4.xyz = rec.tangent;
  part_5.x = rec.status;
  part_5.y = rec.mat_id;
  part_5.z = rec.bvh_hits;

  part_0.yz = vec2(0, 0);
  part_0.w = 1.0f;
  part_1.w = 1.0f;
  part_2.w = 1.0f;
  part_3.w = 1.0f;
  part_4.w = 1.0f;
  part_5.w = 1.0f;
}

void
hit_record_deserialize(out hit_record_t rec,
                       vec4 part_0,
                       vec4 part_1,
                       vec4 part_2,
                       vec4 part_3,
                       vec4 part_4,
                       vec4 part_5)
{
  rec.t = part_0.x;
  rec.position = part_1.xyz;
  rec.uv = part_2.xy;
  rec.normal = part_3.xyz;
  rec.tangent = part_4.xyz;
  rec.status = uint(part_5.x);
  rec.mat_id = uint(part_5.y);
  rec.bvh_hits = uint(part_5.z);
}

#endif // RAYTRACER_RAYTRACING_SHADER_COMMON_H
