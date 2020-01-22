#ifndef RAYTRACER_BVH_NODE_T_H_
#define RAYTRACER_BVH_NODE_T_H_

struct aabb_t
{
  vec3 min;
  vec3 max;
};

float
min_component(vec3 v)
{
  return min(min(v.x, v.y), v.z);
}

float
max_component(vec3 v)
{
  return max(max(v.x, v.y), v.z);
}

/// @article{pagani2018ray,
///  title={A Ray-Box Intersection Algorithm and Efficient Dynamic Voxel
///  Rendering}, author={PAGANI, R and ChIESA, G and TuLLIANI, JM and PANOFSKY,
///  E and PAPAGEORGE, A and SINGANAMALLA, A and COBuZZI, M and JANSEN, Y and
///  DRAGICEVIC, P and FEKETE, JD and others}, journal={Journal of Computer
///  Graphics Techniques}, year={2018}
/// }
bool
slabs(vec3 p0,
      vec3 p1,
      vec3 ray_origin,
      vec3 inv_ray_dir,
      float t_min,
      float t_max)
{
  vec3 t0 = (p0 - ray_origin) * inv_ray_dir;
  vec3 t1 = (p1 - ray_origin) * inv_ray_dir;

  vec3 t_min_new = max(min(t0, t1), t_min);
  vec3 t_max_new = min(max(t0, t1), t_max);

  return max_component(t_min_new) <= min_component(t_max_new);
}

bool
aabb_hit(aabb_t aabb, ray_t ray, float t_min, float t_max)
{
  return slabs(
    aabb.min, aabb.max, ray.origin.xyz, 1.0f / ray.direction.xyz, t_min, t_max);
}

struct bvh_node_t
{
  aabb_t bounds;
  uint offset; // index or left_bvh
  uint count;
};

bool
bvh_node_is_leaf(bvh_node_t node)
{
  return node.count > 0;
}

void
bvh_node_deserialize(vec4 p0, vec4 p1, out bvh_node_t node)
{
  node.bounds.min = p0.xyz;
  node.bounds.max = p1.xyz;
  node.offset = uint(p0.w);
  node.count = uint(p1.w);
}

#endif // RAYTRACER_BVH_NODE_T_H_
