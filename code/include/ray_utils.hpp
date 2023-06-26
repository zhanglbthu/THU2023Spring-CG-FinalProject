#ifndef RAY_UTILS_H
#define RAY_UTILS_H

#include<vecmath.h>
#include<utils.hpp>
class RayUtils
{
public:
    static Vector3f ons(const Vector3f& v1, Vector3f& v2, Vector3f& v3);
    static Vector3f hemisphere(double u1, double u2);
    static Vector3f diffuse(const Vector3f &I, const Vector3f &N);
    static Vector3f reflect(const Vector3f &I, const Vector3f &N);
    static Vector3f refract(const Vector3f &I, Vector3f N, float n);
};
#endif // RAY_UTILS_H