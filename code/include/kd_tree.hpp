#ifndef KD_TREE_HPP
#define KD_TREE_HPP
#include "vecmath.h"
#include "utils.hpp"
#include <vector>
#include <iostream>
#include <algorithm>
// Photon struct
class Photon
{
public:
    Vector3f position;
    Vector3f direction;
    Vector3f power;
    Photon(Vector3f _position, Vector3f _direction, Vector3f _power)
    {
        position = _position;
        direction = _direction;
        power = _power;
    }
};
class VisiblePoint
{
public:
    // 构造函数
    VisiblePoint(int index)
    {
        position = Vector3f(INFINITY, INFINITY, INFINITY);
        direction = Vector3f::ZERO;
        normal = Vector3f::ZERO;


        this->index = index;
        photon_num = 0;
        radius = init_radius;
        flux = Vector3f::ZERO;
        fluxLight = Vector3f::ZERO;
        power = Vector3f::ZERO;

        bdrf = -1;
        new_photon_num = 0;
        new_flux = Vector3f::ZERO;
    }
    Vector3f position;//位置
    Vector3f direction;//方向
    Vector3f normal;//法向

    int index;//序号
    int photon_num;//光子数
    float radius;//半径
    Vector3f flux;//光通量
    Vector3f fluxLight;
    Vector3f power;
    int bdrf;//bdrf

    //每一轮新增量
    int new_photon_num;
    Vector3f new_flux;
};
// KDTree node struct
class KDTreeNode
{
public:
    // 构造函数
    KDTreeNode(VisiblePoint *vp = nullptr, KDTreeNode *left = nullptr, KDTreeNode *right = nullptr, KDTreeNode *parent = nullptr)
        : vp(vp), left(left), right(right), parent(parent)
    {
    }
    VisiblePoint *vp;
    // 左右子树
    KDTreeNode *left;
    KDTreeNode *right;
    // 父节点
    KDTreeNode *parent;
};
class KDTree
{
public:
    // KDTree的大小
    int size;
    // 构造函数
    KDTree() = default;
    // 递归建树
    KDTreeNode *build(std::vector<VisiblePoint> &visiblePoints, int left, int right, int depth)
    {
        if (left > right)
        {
            return nullptr;
        }
        // 选择方差最大的维度
        int dim = depth % 3;
        // 选择中位数
        int mid = (left + right) / 2;
        std::nth_element(visiblePoints.begin() + left, visiblePoints.begin() + mid, visiblePoints.begin() + right + 1,
                         [&](const VisiblePoint &a, const VisiblePoint &b)
                         {
                             return a.position[dim] < b.position[dim];
                         });
        KDTreeNode *node = new KDTreeNode();
        node->vp = &visiblePoints[mid];
        node->left = build(visiblePoints, left, mid - 1, depth + 1);
        node->right = build(visiblePoints, mid + 1, right, depth + 1);
        if (node->left)
        {
            node->left->parent = node;
        }
        if (node->right)
        {
            node->right->parent = node;
        }
        size++;
        return node; // 返回根节点
        // 打印kd树规模
    }
    //给定光子坐标，在可见点集合中进行查找，如果满足光子和可见点的距离小于可见点的半径，则更新可见点的信息
    void update(Photon &photon, KDTreeNode *node, int depth)
    {
        if (node == nullptr)
        {
            return;
        }
        // 选择方差最大的维度
        int dim = depth % 3;
        // 节点的半径
        float radius = node->vp->radius;
        // 选择中位数
        float distance = (node->vp->position - photon.position).length();
        if (distance <= radius)
        {
            float factor = (node->vp->photon_num * ALPHA + ALPHA) / (node->vp->photon_num * ALPHA + 1.0);
            node->vp->photon_num++;
            node->vp->radius = node->vp->radius * factor;
            node->vp->flux = (node->vp->flux + node->vp->power * photon.power) * factor;
        }
        if (photon.position[dim] < node->vp->position[dim])
        {
            update(photon, node->left, depth + 1);
        }
        else
        {
            update(photon, node->right, depth + 1);
        }
    }
    // 打印kd树规模
    void printSize()
    {
        std::cout << "KDTree size: " << size << std::endl;
    }
    //从根节点开始删除树，最后令空节点指向nullptr，不要删除root
    void deleteTree(KDTreeNode *node)
    {
        if (node == nullptr)
        {
            return;
        }
        deleteTree(node->left);
        deleteTree(node->right);
        delete node;
        node = nullptr;
    }

};
#endif