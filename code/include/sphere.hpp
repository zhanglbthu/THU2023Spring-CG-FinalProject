#ifndef SPHERE_H
#define SPHERE_H

#include "object3d.hpp"
#include <vecmath.h>
#include <cmath>

// TODO: Implement functions and add more fields as necessary

class Sphere : public Object3D
{
public:
    Sphere()
    {
        // unit ball at the center
        center = Vector3f(0, 0, 0);
        radius = 1.0f;
    }

    Sphere(const Vector3f &center, float radius, Material *material) : Object3D(material)
    {
        //
        this->center = center;
        this->radius = radius;
        area = 4 * PI * radius * radius;
    }

    ~Sphere() override = default;

    bool intersect(const Ray &r, Hit &h, float tmin) override
    {
        //
        Vector3f oc = r.getOrigin() - center;
        float a = r.getDirection().squaredLength();
        float b = 2 * Vector3f::dot(oc, r.getDirection());
        float c = oc.squaredLength() - radius * radius;
        float delta = b * b - 4 * a * c;
        if (delta > 0)
        {
            float t1 = (-b - std::sqrt(delta)) / (2 * a);
            float t2 = (-b + std::sqrt(delta)) / (2 * a);
            float t = t1;
            if (t < tmin || t > h.getT())
            {
                t = t2;
            }
            if (t >= tmin && t < h.getT())
            {
                Vector3f p = r.pointAtParameter(t);
                Vector3f n = (p - center).normalized();
                h.set(t, material, n, p);
                set_hit_color(h, p);
                return true;
            }
        }
        return false;
    }
    //计算包围盒
    AABB getAABB() override
    {
        return AABB(center - Vector3f(radius, radius, radius), center + Vector3f(radius, radius, radius));
    }
    void set_hit_color(Hit &h, const Vector3f &position) override
    {
        if (this->material->getTexture())
        {
            Vector3f p = position - center;
            float u = 0.5 + atan2(p.x(), p.z()) / (2 * PI);
            float v = 0.5 - asin(p.y() / radius) / PI; 
            //打印纹理坐标.x.y
            // printf("%f %f\n", u, v);
            Vector3f color = this->material->getTextureColor(Vector2f(u, v));
            h.setColor(color);
        }
        else
        {
            h.setColor(material->getColor());
        }
    }
    //返回area
    float getArea() override
    {
        return area;
    }
    //采样
    void sample(Hit &hit, float &pdf) override
    {
        float theta = 2 * PI * RND2;
        float phi = PI * RND2;
        Vector3f dir = Vector3f(std::cos(phi), std::sin(phi)*std::cos(theta), std::sin(phi)*std::sin(theta));
        hit.coords = center + radius * dir;
        hit.normal = dir;
        hit.emission = material->getEmission();
        pdf = 1 / area;
    }
    //返回中心
    Vector3f getCenter()
    {
        return center;
    }
    //返回半径
    float getRadius()
    {
        return radius;
    }
protected:
    Vector3f center;
    float radius;
    float area;
};

#endif
