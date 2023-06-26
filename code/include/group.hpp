#ifndef GROUP_H
#define GROUP_H

#include "object3d.hpp"
#include "bvh.hpp"
#include "ray.hpp"
#include "hit.hpp"
#include <iostream>
#include <vector>

// TODO: Implement Group - add data structure to store a list of Object*
class Group : public Object3D
{

public:
    Group()
    {
    }
    //根据物体集合建树
    explicit Group(int num_objects) : objects(num_objects)
    {
    }

    ~Group() override
    {
    }
    //建树
    void buildBVH()
    {
        bvh = new BVH(objects, 0, objects.size());
    }
    bool intersect(const Ray &r, Hit &h, float tmin) override
    {
        return bvh->intersect(r, h, tmin);
    }

    void addObject(int index, Object3D *obj)
    {
        if (index >= 0 && index < objects.size())
        {
            objects[index] = obj;
        }
    }
    int getGroupSize()
    {
        return objects.size();
    }
    //计算包围盒
    AABB getAABB() override
    {
        return bvh->getAABB();
    }
    //获取第i个物体
    Object3D *getObject(int i)
    {
        return objects[i];
    }
    //获取物体中emission不为0的物体
    std::vector<Object3D *> getLightObjects()
    {
        std::vector<Object3D *> light_objects;
        for (int i = 0; i < objects.size(); i++)
        {
            if (objects[i]->getMaterial()->hasEmission())
            {
                light_objects.push_back(objects[i]);
            }
        }
        return light_objects;
    }
private:
    std::vector<Object3D *> objects;
    //bvh树
    BVH *bvh;
};

#endif
