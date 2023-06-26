#ifndef RAY_H
#define RAY_H

#include <cassert>
#include <iostream>
#include <utils.hpp>
#include <cmath>
#include <Vector3f.h>

// Ray class mostly copied from Peter Shirley and Keith Morley
class Ray
{
public:
    Ray() = delete;
    Ray(const Vector3f &orig, const Vector3f &dir)
    {
        origin = orig;
        direction = dir.normalized();
    }

    Ray(const Ray &r)
    {
        origin = r.origin;
        direction = r.direction;
    }

    const Vector3f &getOrigin() const
    {
        return origin;
    }

    const Vector3f &getDirection() const
    {
        return direction;
    }

    Vector3f pointAtParameter(float t) const
    {
        return origin + direction * t;
    }
    // 计算反射ray
    Ray reflectRay(const Vector3f &incident, const Vector3f &normal, float t) const
    {
        Vector3f reflectDir = -2.0f * Vector3f::dot(incident, normal) * normal;
        Vector3f reflectPoint = pointAtParameter(t);
        return Ray(reflectPoint, reflectDir);
    }
    // 计算反射方向
    Vector3f reflectDir(const Vector3f &incident, const Vector3f &normal) const
    {
        float cosTheta = Vector3f::dot(incident, normal);
        Vector3f reflectDir = incident - 2.0f * cosTheta * normal;
        return reflectDir.normalized();
    }
    // 计算折射ray
    Ray refractRay(const Vector3f &incident, const Vector3f &normal, float t, float refractiveIndex) const
    {
        float cosTheta = Vector3f::dot(incident, normal);
        float sinTheta = sqrt(1 - cosTheta * cosTheta);
        float sinThetaT = sinTheta / refractiveIndex;
        float cosThetaT = sqrt(1 - sinThetaT * sinThetaT);
        Vector3f refractDir = (incident / refractiveIndex + (cosTheta / refractiveIndex - cosThetaT) * normal).normalized();
        Vector3f refractPoint = pointAtParameter(t);
        return Ray(refractPoint, refractDir);
    }
    // 计算折射方向
    Vector3f refractDir(const Vector3f &direction, Vector3f normal, float n) const
    {
        float R0 = ((1.0 - n) * (1.0 - n)) / ((1.0 + n) * (1.0 + n));
        float cosTheat = Vector3f::dot(normal, direction);
        // 判断光线是从介质内部出射还是从外部入射
        if (cosTheat > 0)
        {
            // 从介质内部出射
            normal = -normal;
            n = 1.0 / n;
        }
        n = 1.0 / n;
        // TODO:求光线折射的方向
        float cosTheta1 = -Vector3f::dot(normal, direction);
        Vector3f reflectDirection = (direction + 2.0 * cosTheta1 * normal).normalized();
        float cosTheta2 = 1.0 - n * n * (1.0 - cosTheta1 * cosTheta1);
        if (cosTheta2 < 0)
        {
            return reflectDirection;
        }

        float Rprob = R0 + (1.0 - R0) * pow(1.0 - cosTheta1, 5.0);
        Vector3f refractDirection = (direction * n + normal * (n * cosTheta1 - sqrt(cosTheta2))).normalized();

        if (RND2 < Rprob)
        {
            return reflectDirection;
        }
        else
        {
            // 打印发生折射
            return refractDirection;
        }
    }

    void ons(const Vector3f &v1, Vector3f &v2, Vector3f &v3) const 
    {
        if (std::abs(v1.x()) > std::abs(v1.y()))
        {
            // project to the y = 0 plane and construct a normalized orthogonal vector in this plane
            float invLen = 1.f / sqrtf(v1.x() * v1.x() + v1.z() * v1.z());
            v2 = Vector3f(-v1.z() * invLen, 0.0f, v1.x() * invLen);
        }
        else
        {
            // project to the x = 0 plane and construct a normalized orthogonal vector in this plane
            float invLen = 1.0f / sqrtf(v1.y() * v1.y() + v1.z() * v1.z());
            v2 = Vector3f(0.0f, v1.z() * invLen, -v1.y() * invLen);
        }
        v3 = Vector3f::cross(v1, v2);
    }
    Vector3f hemisphere(double u1, double u2) const 
    {
        const double r = sqrt(u1 * u1);
        const double phi = 2 * PI * u2;
        return Vector3f(cos(phi) * r, sin(phi) * r, sqrt(1 - u1 * u1));
    }

    // 已知平面法向量，求漫反射光线方向
    Vector3f diffuseDir(const Vector3f &direction, const Vector3f &normal) const
    {
        // float r1 = 2 * PI * RND2, r2 = RND2, r2s = sqrt(r2);
        // Vector3f nl = Vector3f::dot(normal, direction) < 0 ? normal : normal * -1;
        // //Vec w=nl, u=((fabs(w.x)>.1?Vec(0,1):Vec(1))%w).norm(), v=w%u;
        // Vector3f w = nl;
        // Vector3f u = Vector3f::cross(fabs(w.x()) > .1 ? Vector3f(0, 1, 0) : Vector3f(1, 0, 0), w).normalized();
        // Vector3f v = Vector3f::cross(w, u);
        // Vector3f dir = (u * cos(r1) * r2s + v * sin(r1) * r2s + w * sqrt(1 - r2)).normalized();
        // return dir;

        // Vector3f rotX, rotY;
        // ons(normal, rotX, rotY);
		// Vector3f sampledDir = hemisphere(RND2, RND2);
        // Vector3f rotatedDir;
        // rotatedDir.x() = Vector3f::dot(sampledDir, Vector3f(rotX.x(), rotY.x(), normal.x()));
        // rotatedDir.y() = Vector3f::dot(sampledDir, Vector3f(rotX.y(), rotY.y(), normal.y()));
        // rotatedDir.z() = Vector3f::dot(sampledDir, Vector3f(rotX.z(), rotY.z(), normal.z()));
        // return rotatedDir.normalized();

        Vector3f z_ = Vector3f::cross(direction, normal);
        Vector3f x_ = Vector3f::cross(z_, normal);
        z_.normalize();
        x_.normalize();
        Vector3f next_direction;
        if (Vector3f::dot(direction, normal) < 0)
            next_direction = RND1 * z_ + RND1 * x_ + RND2 * normal;
        else
            next_direction = RND1 * z_ + RND1 * x_ - RND2 * normal;
        next_direction.normalize();
        return next_direction;
    }
    // randomUnitVector实现
    Vector3f randomUnitVector() const
    {
        float x, y, z;
        do
        {
            x = 2.0 * RND - 1.0;
            y = 2.0 * RND - 1.0;
            z = 2.0 * RND - 1.0;
        } while (x * x + y * y + z * z > 1.0);
        return Vector3f(x, y, z).normalized();
    }
    // 修改ray的方向
    void setDirection(const Vector3f &dir)
    {
        direction = dir.normalized();
    }
    // 修改ray的起点
    void setOrigin(const Vector3f &ori)
    {
        origin = ori;
    }

private:
    Vector3f origin;
    Vector3f direction;
};

inline std::ostream &operator<<(std::ostream &os, const Ray &r)
{
    os << "Ray <" << r.getOrigin() << ", " << r.getDirection() << ">";
    return os;
}

#endif // RAY_H
