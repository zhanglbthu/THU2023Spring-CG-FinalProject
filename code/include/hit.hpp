#ifndef HIT_H
#define HIT_H

#include <vecmath.h>
#include "ray.hpp"

class Material;

class Hit
{
public:
    // constructors
    Hit()
    {
        material = nullptr;
        t = 1e38;
        //初始化半径
        radius = 0.0001;
    }

    Hit(float _t, Material *m, const Vector3f &n)
    {
        t = _t;
        material = m;
        normal = n;
        radius = 0.0001;
    }

    Hit(const Hit &h)
    {
        t = h.t;
        material = h.material;
        normal = h.normal;
        radius = 0.0001;
    }

    // destructor
    ~Hit() = default;

    float getT() const
    {
        return t;
    }

    Material *getMaterial() const
    {
        return material;
    }

    const Vector3f &getNormal() const
    {
        return normal;
    }
    // set
    void set(float _t, Material *m, const Vector3f &n, const Vector3f &c)
    {
        t = _t;
        material = m;
        normal = n;
        coords = c;
    }
    
    void setColor(Vector3f c)
    {
        color = c;
    }
    
    Vector3f getColor() const
    {
        return color;
    }
    Vector3f coords = Vector3f::ZERO;
    Vector3f normal;
    Vector3f emission = Vector3f::ZERO;
private:
    float t;
    Material *material;
    Vector3f color = Vector3f::ZERO;
    //光子映射判定半径
    float radius = 0.0f;
};

inline std::ostream &operator<<(std::ostream &os, const Hit &h)
{
    os << "Hit <" << h.getT() << ", " << h.getNormal() << ">";
    return os;
}

#endif // HIT_H
