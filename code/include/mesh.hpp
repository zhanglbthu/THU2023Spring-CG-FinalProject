#ifndef MESH_H
#define MESH_H

#include <vector>
#include "object3d.hpp"
#include "triangle.hpp"
#include "Vector2f.h"
#include "Vector3f.h"
#include "bvh.hpp"

class Mesh : public Object3D
{

public:
    
    Mesh(const char *filename, Material *m);
    
    struct TriangleIndex
    {
        TriangleIndex()
        {
            x[0] = 0;
            x[1] = 0;
            x[2] = 0;
        }
        int &operator[](const int i) { return x[i]; }
        // By Computer Graphics convention, counterclockwise winding is front face
        int x[3]{};
    };

    std::vector<Vector3f> v;//顶点
    std::vector<Vector2f> vt;//点纹理
    std::vector<Vector3f> vn;//点法线

    std::vector<Vector3f> n;//法线

    vector<TriangleIndex> vIndex, vtIndex, vnIndex;//顶点索引，纹理索引，法线索引

    std::vector<Object3D*> triangles;//三角形
    
    bool intersect(const Ray &r, Hit &h, float tmin) override
    {
        return bvh->intersect(r, h, tmin);
    }
    // 计算包围盒
    AABB getAABB() override
    {
        Vector3f min(INFINITY, INFINITY, INFINITY);
        Vector3f max(-INFINITY, -INFINITY, -INFINITY);
        for (int i = 0; i < v.size(); i++)
        {
            min.x() = std::min(min.x(), v[i].x());
            min.y() = std::min(min.y(), v[i].y());
            min.z() = std::min(min.z(), v[i].z());
            max.x() = std::max(max.x(), v[i].x());
            max.y() = std::max(max.y(), v[i].y());
            max.z() = std::max(max.z(), v[i].z());
        }
        // printf("meshAABB: min: (%f, %f, %f), max: (%f, %f, %f)\n", min.x(), min.y(), min.z(), max.x(), max.y(), max.z());
        return AABB(min, max);
    }
    
private:
    // Normal can be used for light estimation
    void computeNormal();
    //设置三角形的纹理和法线方法
    void set_texture_and_normal();
    //bvh
    BVH *bvh;
    //是否支持纹理和法线
    bool has_texture_and_normal = false;
};

#endif
