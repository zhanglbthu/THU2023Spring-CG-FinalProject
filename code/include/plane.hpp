#ifndef PLANE_H
#define PLANE_H

#include "object3d.hpp"
#include <vecmath.h>
#include <cmath>

// TODO: Implement Plane representing an infinite plane
// function: ax+by+cz=d
// choose your representation , add more fields and fill in the functions

class Plane : public Object3D
{
public:
    Plane()
    {
    }
    Plane(const Vector3f &normal, float d, Material *m) : Object3D(m)
    {
        this->normal = normal.normalized();
        this->d = d;
        //计算u轴和v轴
        this->u_axis = Vector3f::ZERO;
        this->v_axis = Vector3f::ZERO;
        if(this->normal.x() < 1e-3 && this->normal.z() < 1e-3)
        {
            this->u_axis = Vector3f(1, 0, 0);
        }
        else
        {
            this->u_axis = Vector3f::cross(Vector3f::UP, this->normal).normalized();
        }
        this->v_axis = Vector3f::cross(this->normal, this->u_axis).normalized();
    }

    ~Plane() override = default;

    bool intersect(const Ray &r, Hit &h, float tmin) override
    {
        float dirDotNormal = Vector3f::dot(this->normal, r.getDirection()); // cosTheta
        if (dirDotNormal > -1e-6)                                           // 平行
        {
            return false;
        }
        else
        {
            float t = (this->d - Vector3f::dot(this->normal, r.getOrigin())) / dirDotNormal;
            // t非负且t比之前交点t更小
            if (t > tmin && t < h.getT())
            {
                h.set(t, this->material, this->normal, r.pointAtParameter(t));
                set_hit_color(h, r.pointAtParameter(t));
                return true;
            }
            else
            {
                return false;
            }
        }
    }
    void set_hit_color(Hit &h, const Vector3f &position) override
    {
        if(this->material->getTexture())
        {
            Vector2f uv = getUV(position);
            //打印uv坐标
            //printf("uv: %f, %f\n", uv.x(), uv.y());
            //计算纹理颜色
            Vector3f textureColor = this->material->getTextureColor(Vector2f(uv.x(), uv.y()));
            h.setColor(textureColor);
            // h.setColor(Vector3f(1, 1, 1));
        }
        else
        {
            h.setColor(this->material->getColor());
        }
    }
    // 计算包围盒
    AABB getAABB() override
    {
        // printf("Plane getAABB\n");
        return AABB(Vector3f(-INFINITY, -INFINITY, -INFINITY), Vector3f(INFINITY, INFINITY, INFINITY));
    }
    //计算纹理坐标
    Vector2f getUV(Vector3f point)
    {
        float u = Vector3f::dot(point - d * normal, u_axis) / 100;
        float v = Vector3f::dot(point - d * normal, v_axis) / 100;
        return Vector2f(u, 1 - v);
    }
protected:
    Vector3f normal; // 平面法向量
    Vector3f u_axis;
    Vector3f v_axis;  
    float d;         // 平面到原点的距离
};

#endif // PLANE_H
