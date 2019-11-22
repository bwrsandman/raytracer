#pragma once 

class vec3;
class Ray;
struct hit_record;

class Material
{
public:
  virtual bool scatter(const Ray& r_in,
                       const hit_record& rec,
                       vec3& attenuation,
                       Ray& scattered) const = 0;
};
