#ifndef BVH_H
#define BVH_H

#include <ray.hpp>
#include <vecmath.h>
#include <vector>
#include <object3d.hpp>

// BVN节点结构体
class BVHNode
{
public:
    BVHNode *left;
    BVHNode *right;
    AABB box;
    //存储物体
    int start;
    int end;
    std::vector<Object3D*> objects;
    BVHNode() {}
    BVHNode(BVHNode *left, BVHNode *right, AABB box, int start, int end) : left(left), right(right), box(box), start(start), end(end) {}
    //判断射线是否与包围盒相交
    bool intersect(const Ray &ray, float tmin, float tmax) const
    {
        return box.intersect(ray, tmin, tmax);
    }
};
// BVN类
class BVH
{
public:
    BVH() {}
    BVH(std::vector<Object3D*> objects, int start, int end)
    {
        this->objects = objects;
        this->start = start;
        this->end = end;
        root = buildBVH(start, end);
        printf("BVH: start: %d, end: %d\n", start, end);
    }
    //判断射线是否与包围盒相交
    bool intersect(const Ray &ray, Hit &hit, float tmin) const
    {
        return intersectBVH(ray, hit, tmin, root);
    }
    //获取根节点包围盒
    AABB getAABB() const
    {
        return root->box;
    }
private:
    BVHNode *root;
    std::vector<Object3D*> objects;
    int start;
    int end;
    //构建BVH
    BVHNode *buildBVH(int start, int end)
    {
        AABB box;
        for (int i = start; i < end; i++)
        {
            AABB temp = objects[i]->getAABB();
            // printf("AABB: min: (%f, %f, %f), max: (%f, %f, %f)\n", temp.min.x(), temp.min.y(), temp.min.z(), temp.max.x(), temp.max.y(), temp.max.z());
            if (i == start)
            {
                box = temp;
            }
            else
            {
                box.min = Vector3f(std::min(box.min.x(), temp.min.x()), std::min(box.min.y(), temp.min.y()), std::min(box.min.z(), temp.min.z()));
                box.max = Vector3f(std::max(box.max.x(), temp.max.x()), std::max(box.max.y(), temp.max.y()), std::max(box.max.z(), temp.max.z()));
            }
        }
        // box.print();
        if (end - start == 1)
        {
            return new BVHNode(nullptr, nullptr, box, start, end);
        }
        else
        {
            int axis = box.max.x() - box.min.x() > box.max.y() - box.min.y() ? 0 : 1;
            axis = box.max.z() - box.min.z() > box.max[axis] - box.min[axis] ? 2 : axis;
            int mid = (start + end) / 2;
            std::nth_element(objects.begin() + start, objects.begin() + mid, objects.begin() + end, [axis](Object3D *a, Object3D *b) 
            {
                if(a->getAABB().min[axis] < b->getAABB().min[axis])
                {
                    return true;
                }
                else
                {
                    return false;
                }
            });
            return new BVHNode(buildBVH(start, mid), buildBVH(mid, end), box, start, end);
        }
    }
    //判断射线是否与包围盒相交
    bool intersectBVH(const Ray &ray, Hit &hit, float tmin, BVHNode *node) const
    {
        if (node == nullptr)
        {
            return false;
        }
        if (!node->intersect(ray, -INFINITY, INFINITY))
        {
            //printf("not intersect\n");
            return false;
        }
        bool result = false;
        if (node->left == nullptr && node->right == nullptr)
        {
            // printf("leaf node\n");
            for (int i = node->start; i
                < node->end; i++)
            {
                if (objects[i]->intersect(ray, hit, tmin))
                {
                    result = true;
                }
            }
        }
        else
        {
            if (intersectBVH(ray, hit, tmin, node->left))
            {
                result = true;
            }
            if (intersectBVH(ray, hit, tmin, node->right))
            {
                result = true;
            }
        }
        return result;
    }
};
#endif