#include "ray_utils.hpp"
// 正交归一化
Vector3f RayUtils::ons(const Vector3f &v1, Vector3f &v2, Vector3f &v3)
{
    if (std::abs(v1.x()) > std::abs(v1.y()))
    {
        float invLen = 1.0f / std::sqrt(v1.x() * v1.x() + v1.z() * v1.z());
        v2 = Vector3f(-v1.z() * invLen, 0.0f, v1.x() * invLen);
    }
    else
    {
        float invLen = 1.0f / std::sqrt(v1.y() * v1.y() + v1.z() * v1.z());
        v2 = Vector3f(0.0f, v1.z() * invLen, -v1.y() * invLen);
    }
    v3 = Vector3f::cross(v1, v2);
}
// 采样半球
Vector3f RayUtils::hemisphere(double u1, double u2)
{
    double r = std::sqrt(1 - u1 * u1);
    double phi = 2 * PI * u2;
    return Vector3f(std::cos(phi) * r, std::sin(phi) * r, u1);
}
// 漫反射
Vector3f RayUtils::diffuse(const Vector3f &I, const Vector3f &N)
{
    float r = sqrt(1.0 - RND2 * RND2);
    float phi = 2 * PI * RND2;
    Vector3f n = Vector3f::dot(I, N) < 0 ? N : -N;
    Vector3f u = Vector3f::cross((std::abs(n.x()) > 0.1 ? Vector3f(0, 1, 0) : Vector3f(1, 0, 0)), n).normalized();
    Vector3f v = Vector3f::cross(n, u);
    return (u * cos(phi) * r + v * sin(phi) * r + n * sqrt(1 - r * r)).normalized();
}
// 反射
Vector3f RayUtils::reflect(const Vector3f &I, const Vector3f &N)
{
    return I - 2 * Vector3f::dot(I, N) * N;
}
// 折射
Vector3f RayUtils::refract(const Vector3f &direction, Vector3f normal, float n)
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
    float cosTheta1 = -Vector3f::dot(normal, direction);
    Vector3f reflectDirection = (direction + 2.0 * cosTheta1 * normal).normalized();
    float cosTheta2 = 1.0 - n * n * (1.0 - cosTheta1 * cosTheta1);
    if (cosTheta2 < 0)
    {
        return reflectDirection;
    }

    float Rprob = R0 + (1.0 - R0) * pow(1.0 - cosTheta1, 5.0); // Schlick's approximation
    Vector3f refractDirection = (direction * n + normal * (n * cosTheta1 - sqrt(cosTheta2))).normalized();

    if (RND2 < Rprob)
    {
        return reflectDirection;
    }
    else
    {
        return refractDirection;
    }
}