#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <vecmath.h>
#include "object3d.hpp"

// transforms a 3D point using a matrix, returning a 3D point
static Vector3f transformPoint(const Matrix4f &mat, const Vector3f &point)
{
    return (mat * Vector4f(point, 1)).xyz();
}

// transform a 3D directino using a matrix, returning a direction
static Vector3f transformDirection(const Matrix4f &mat, const Vector3f &dir)
{
    return (mat * Vector4f(dir, 0)).xyz();
}

// TODO: implement this class so that the intersect function first transforms the ray
class Transform : public Object3D
{
public:
    Transform() {}

    Transform(const Matrix4f &m, Object3D *obj) : o(obj)
    {
        init_transform = m;
        transform = m.inverse();
        this->material = obj->getMaterial();
    }

    ~Transform()
    {
    }

    virtual bool intersect(const Ray &r, Hit &h, float tmin)
    {
        Vector3f trSource = transformPoint(transform, r.getOrigin());
        Vector3f trDirection = transformDirection(transform, r.getDirection());
        Ray tr(trSource, trDirection);
        bool inter = o->intersect(tr, h, tmin);
        if (inter)
        {
            h.set(h.getT(), h.getMaterial(), transformDirection(transform.transposed(), h.getNormal()).normalized(), tr.pointAtParameter(h.getT()));
            //打印交点颜色
            // printf("transform: (%f, %f, %f)\n", h.getColor().x(), h.getColor().y(), h.getColor().z());
        }
        return inter;
    }
    // 根据物体包围盒计算变换后的包围盒
    AABB getAABB() override
    {
        // 原始包围盒
        AABB aabb = o->getAABB();
        // 获取物体包围盒的8个顶点
        std::vector<Vector3f> vertices = aabb.getVertices();

        Vector3f min(INFINITY, INFINITY, INFINITY);
        Vector3f max(-INFINITY, -INFINITY, -INFINITY);
        // TODO:根据矩阵确定新的包围盒
        for (const Vector3f &vertex : vertices)
        {
            Vector4f transformedVertex = init_transform * Vector4f(vertex.x(), vertex.y(), vertex.z(), 1.0f);
            Vector3f transformedPoint(transformedVertex.x(), transformedVertex.y(), transformedVertex.z());

            // 更新最小点和最大点
            min = min.min(transformedPoint);
            max = max.max(transformedPoint);
        }
        // printf("transformAABB: min: (%f, %f, %f), max: (%f, %f, %f)\n", min.x(), min.y(), min.z(), max.x(), max.y(), max.z());
        return AABB(min, max);
    }

protected:
    Object3D *o; // un-transformed object
    Matrix4f transform;
    Matrix4f init_transform;
    Vector3f scale;
    float uniformScale;
    Vector3f translate;
    Vector3f rotate;
};

#endif // TRANSFORM_H
