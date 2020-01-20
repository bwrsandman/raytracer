#ifndef RAYTRACER_TRIANGLE_T_H
#define RAYTRACER_TRIANGLE_T_H

#include "ray_t.h"

struct triangle_t
{
  vec3 position[3];
  vec2 uv[3];
  vec3 normal[3];
  uint mat_id;
};

#ifdef __cplusplus
// static void
// triangle_serialize(const triangle_t& triangle, vec4& data0, uvec4& data1)
//{
//  data0.e[0] = sphere.center.e[0];
//  data0.e[1] = sphere.center.e[1];
//  data0.e[2] = sphere.center.e[2];
//  data0.e[3] = sphere.radius;
//  data1.e[0] = sphere.mat_id;
//}

#else
#include "hit_record_t.h"

void
triangle_deserialize(vec4 p0,
                     vec4 p1,
                     vec4 p2,
                     vec4 normal0,
                     vec4 normal1,
                     vec4 normal2,
                     vec2 uv0,
                     vec2 uv1,
                     vec2 uv2,
                     uint mat_id,
                     out triangle_t triangle)
{
  triangle.position[0] = p0.xyz;
  triangle.position[1] = p1.xyz;
  triangle.position[2] = p2.xyz;
  triangle.normal[0] = normal0.xyz;
  triangle.normal[1] = normal1.xyz;
  triangle.normal[2] = normal2.xyz;
  triangle.uv[0] = uv0;
  triangle.uv[1] = uv1;
  triangle.uv[2] = uv2;
  triangle.mat_id = mat_id;
}

void
triangle_hit(ray_t ray,
             triangle_t triangle,
             float t_min,
             float t_max,
             out hit_record_t rec)
{
  vec3 v0v1 = triangle.position[1] - triangle.position[0];
  vec3 v0v2 = triangle.position[2] - triangle.position[0];

  vec3 ray_edge_cross = cross(ray.direction.xyz, v0v2);
  float det = dot(v0v1, ray_edge_cross);
  if (abs(det) < FLT_EPSILON) {
    // This ray is parallel to this triangle.
    rec.status = HIT_RECORD_STATUS_MISS;
    return;
  }
  float det_inv = 1.0f / det;
  vec3 v0ro = ray.origin.xyz - triangle.position[0];
  float u = det_inv * dot(v0ro, ray_edge_cross);
  if (u < 0.0f || u > 1.0f) {
    rec.status = HIT_RECORD_STATUS_MISS;
    return;
  }
  vec3 q = cross(v0ro, v0v1);
  float v = det_inv * dot(ray.direction.xyz, q);
  if (v < 0.0f || u + v > 1.0f) {
    rec.status = HIT_RECORD_STATUS_MISS;
    return;
  }

  // At this stage we can compute t to find out where the intersection point
  // is on the line.
  float t = det_inv * dot(v0v2, q);
  if (t >= t_max || t <= t_min) {
    rec.status = HIT_RECORD_STATUS_MISS;
    return;
  }

  vec3 barycentric_coordinates = vec3(u, v, 1 - u - v);

  vec2 uv0uv1 = triangle.uv[1] - triangle.uv[0];
  vec2 uv0uv2 = triangle.uv[2] - triangle.uv[0];

  float denom_inv = 1.0f / (uv0uv1.x * uv0uv2.y - uv0uv1.y * uv0uv2.x);

  rec.t = t;
  rec.position = ray_point_at(ray, rec.t).xyz;
  rec.normal = normalize(triangle.normal[0] * barycentric_coordinates.z +
                         triangle.normal[1] * barycentric_coordinates.x +
                         triangle.normal[2] * barycentric_coordinates.y);
  rec.tangent = normalize((v0v1 * uv0uv2.y - v0v2 * uv0uv1.y) * denom_inv);
  rec.uv.xy = triangle.uv[0] * barycentric_coordinates.z +
              triangle.uv[1] * barycentric_coordinates.x +
              triangle.uv[2] * barycentric_coordinates.y;
  rec.mat_id = triangle.mat_id;
  rec.status = HIT_RECORD_STATUS_HIT;
}
#endif // __cplusplus

#endif // RAYTRACER_TRIANGLE_T_H
