#pragma once 

class vec3;
struct Ray;
class Scene;
struct hit_record;

class Material
{
public:
  virtual ~Material() = default;
  virtual bool scatter(const Scene& scene,
                       const Ray& r_in,
                       const hit_record& rec,
                       vec3& attenuation,
                       Ray (&scattered)[2]) const = 0;
};
