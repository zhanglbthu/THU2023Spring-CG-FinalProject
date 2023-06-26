#ifndef OBJECT3D_H
#define OBJECT3D_H

#include "ray.hpp"
#include "hit.hpp"
#include "material.hpp"

// AABB包围盒结构体
class AABB
{
public:
    Vector3f min;
    Vector3f max;
    Vector3f bounds[2];
    AABB() {}
    AABB(const Vector3f &min, const Vector3f &max) : min(min), max(max) {}
    inline void fit(const Vector3f &point)
    {
        for (int i = 0; i < 3; i++)
        {
            min[i] = std::min(min[i], point[i]);
            max[i] = std::max(max[i], point[i]);
        }
    }
    void set(const Vector3f &min, const Vector3f &max)
    {
        bounds[0] = min;
        bounds[1] = max;
    }
    inline void reset()
    {
        min = Vector3f(INFINITY, INFINITY, INFINITY);
        max = Vector3f(-INFINITY, -INFINITY, -INFINITY);
    }
    // 打印AABB包围盒
    void print()
    {
        printf("AABB: min: (%f, %f, %f), max: (%f, %f, %f)\n", min.x(), min.y(), min.z(), max.x(), max.y(), max.z());
    }
    // 判断射线是否与包围盒相交
    bool intersect(const Ray &ray, float tmin, float tmax) const
    {
        // 打印包围盒信息
        //  printf("AABB: min: (%f, %f, %f), max: (%f, %f, %f)\n", min.x(), min.y(), min.z(), max.x(), max.y(), max.z());
        for (int i = 0; i < 3; i++)
        {
            float invD = 1.0f / ray.getDirection()[i];
            float t0 = (min[i] - ray.getOrigin()[i]) * invD;
            float t1 = (max[i] - ray.getOrigin()[i]) * invD;
            if (invD < 0.0f)
            {
                std::swap(t0, t1);
            }
            tmin = t0 > tmin ? t0 : tmin;
            tmax = t1 < tmax ? t1 : tmax;
        }
        float t_enter = tmin;
        float t_exit = tmax;
        // printf("t_enter: %f, t_exit: %f\n", t_enter, t_exit);
        if (t_enter < t_exit && t_exit > 0)
            return true;
        else
            return false;
    }
    // 获取八个顶点
    std::vector<Vector3f> getVertices()
    {
        std::vector<Vector3f> vertices;
        vertices.push_back(Vector3f(min.x(), min.y(), min.z()));
        vertices.push_back(Vector3f(min.x(), min.y(), max.z()));
        vertices.push_back(Vector3f(min.x(), max.y(), min.z()));
        vertices.push_back(Vector3f(min.x(), max.y(), max.z()));
        vertices.push_back(Vector3f(max.x(), min.y(), min.z()));
        vertices.push_back(Vector3f(max.x(), min.y(), max.z()));
        vertices.push_back(Vector3f(max.x(), max.y(), min.z()));
        vertices.push_back(Vector3f(max.x(), max.y(), max.z()));
        return vertices;
    }
};

// Base class for all 3d entities.
class Object3D
{
public:
    Object3D() : material(nullptr) {}

    virtual ~Object3D() = default;

    explicit Object3D(Material *material)
    {
        this->material = material;
    }

    // Intersect Ray with this object. If hit, store information in hit structure.
    virtual bool intersect(const Ray &r, Hit &h, float tmin) = 0;
    // 计算包围盒
    virtual AABB getAABB() = 0;
    virtual void set_hit_color(Hit &h, const Vector3f &position)
    {
        if (this->material->getTexture())
        {
            printf("set_texture_color\n");
            h.setColor(Vector3f(1, 1, 1));
        }
        else
        {
            h.setColor(this->material->getColor());
        }
    }
    virtual float getArea()
    {
        printf("getArea Error\n");
        return 0;
    }
    virtual void sample(Hit &hit, float &pdf)
    {
        printf("Sample Error\n");
    }
    // 获取材质
    Material *getMaterial()
    {
        return material;
    }
protected:
    Material *material;
};

#endif
