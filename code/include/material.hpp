#ifndef MATERIAL_H
#define MATERIAL_H

#include <cassert>
#include <vecmath.h>

#include "ray.hpp"
#include "hit.hpp"
#include "texture.hpp"
#include <iostream>
#include <algorithm>

// TODO: Implement Shade function that computes Phong introduced in class.
class Material
{
public:
    Material(const Vector3f &color = Vector3f::ZERO, const Vector3f &emission = Vector3f::ZERO, const Vector3f &type = Vector3f(1, 1, 1), float refractiveIndex = 1.0f,
             const char *texturePath = "") : color(color), emission(emission), refractiveIndex(refractiveIndex), texture(texturePath)
    {
        this->type = type / (type.x() + type.y() + type.z()); // 归一化
        if (strlen(texturePath) > 0)
        {
            hasTexture = true;
        }
        else
        {
            hasTexture = false;
        }
    }

    virtual ~Material() = default;

    virtual Vector3f getDiffuseColor() const
    {
        return diffuseColor;
    }

    Vector3f Shade(const Ray &ray, const Hit &hit,
                   const Vector3f &dirToLight, const Vector3f &lightColor)
    {
        Vector3f shaded = Vector3f::ZERO;
        //
        Vector3f normal = hit.getNormal().normalized(); // 法向量

        float cosTheta = std::max(0.0f, Vector3f::dot(normal, dirToLight.normalized())); // 余弦值

        // Vector3f Id = diffuseColor * cosTheta;
        Vector3f Id = color * cosTheta;

        Vector3f rayDirection = ray.getDirection().normalized();                                            // 光线方向
        Vector3f reflectedRay = (2 * Vector3f::dot(normal, dirToLight) * normal - dirToLight).normalized(); // 反射光线方向
        Vector3f viewDirection = -rayDirection;                                                             // 视线方向
        float cosAlpha = std::max(0.0f, Vector3f::dot(reflectedRay, viewDirection));
        Vector3f Is = specularColor * pow(cosAlpha, shininess);
        shaded = lightColor * (Id + Is);
        return shaded;
    }
    // getRefractionIndex
    float getRefractionIndex() const
    {
        return refractiveIndex;
    }
    // 获取类型
    Vector3f getType() const
    {
        return type;
    }
    // 获取发光系数
    Vector3f getEmission() const
    {
        return emission;
    }
    // 获取折射率
    float getRefractiveIndex() const
    {
        return refractiveIndex;
    }
    // 获取材质颜色
    Vector3f getColor() const
    {
        return color;
    }
    // 返回材质是否有纹理
    bool getTexture() const
    {
        return this->hasTexture;
    }
    // 获取纹理颜色
    Vector3f getTextureColor(const Vector2f &uv) const
    {
        return texture.getColor(uv);
    }
    bool hasEmission() const
    {
        if (emission.length() > EPLISON)
            return true;
        else
            return false;
    }
    Vector3f eavl(const Vector3f &wi, const Vector3f &wo, const Vector3f &N)
    {
        // TODO:完善计算brdf的函数
        float cosTheta = Vector3f::dot(N, wo);
        if (cosTheta < 0)
            return Vector3f::ZERO;
        else
            return color / PI;
    }
    // 计算世界坐标系下的向量
    Vector3f toWorld(const Vector3f &a, const Vector3f &N)
    {
        Vector3f B, C;
        if (std::fabs(N.x()) > std::fabs(N.y()))
        {
            float invLen = 1.0f / std::sqrt(N.x() * N.x() + N.z() * N.z());
            C = Vector3f(N.z() * invLen, 0.0f, -N.x() * invLen);
        }
        else
        {
            float invLen = 1.0f / std::sqrt(N.y() * N.y() + N.z() * N.z());
            C = Vector3f(0.0f, N.z() * invLen, -N.y() * invLen);
        }
        B = Vector3f::cross(C, N);
        return a.x() * B + a.y() * C + a.z() * N;
    }
    // 计算漫反射光线
    Vector3f diffuse(const Vector3f &I, const Vector3f &N)
    {
        float x_1 = RND2, x_2 = RND2;
        float z = std::fabs(1.0 - 2.0 * x_1);
        float r = std::sqrt(1.0 - z * z);
        float phi = 2.0 * PI * x_2;
        Vector3f localRay = Vector3f(r * std::cos(phi), r * std::sin(phi), z);
        return toWorld(localRay, N);
    }
    // 计算反射光线
    Vector3f reflect(const Vector3f &I, const Vector3f &N) const
    {
        return I - 2 * Vector3f::dot(I, N) * N;
    }
    // 计算折射光线
    Vector3f refract(const Vector3f &I, const Vector3f &N, const float &ior) const
    {
        float cosi = clamp(-1, 1, Vector3f::dot(I, N));
        float etai = 1, etat = ior;
        Vector3f n = N;
        if (cosi < 0)
        {
            cosi = -cosi;
        }
        else
        {
            std::swap(etai, etat);
            n = -N;
        }
        float eta = etai / etat;
        float k = 1 - eta * eta * (1 - cosi * cosi);
        // 计算反射光线
        return k < 0 ? reflect(I, N) : eta * I + (eta * cosi - sqrtf(k)) * n;
    }
    Vector3f sampleDir(const Vector3f &wi, const Vector3f &N)
    {
        float type = RND2;
        if (type < this->type.x())
        {
            // 漫反射
            return diffuse(wi, N);
        }
        else if (type < this->type.x() + this->type.y())
        {
            // 镜面反射
            return reflect(wi, N);
        }
        else
        {
            // 折射
            return refract(wi, N, refractiveIndex);
        }
    }
    void sample(const Vector3f &wi, const Vector3f &N, Vector3f &nextDir, float &pdf)
    {
        float type = RND2;
        if (type < this->type.x())
        {
            // 漫反射
            nextDir = diffuse(wi, N);
            pdf = 1.0f / (2 * PI);
        }
        else if (type < this->type.x() + this->type.y())
        {
            // 镜面反射
            nextDir = reflect(wi, N);
            pdf = 1.0f;
        }
        else
        {
            // 折射
            nextDir = refract(wi, N, refractiveIndex);
            pdf = 1.0f;
        }
    }

protected:
    Vector3f color;
    Vector3f diffuseColor;
    Vector3f specularColor;
    float shininess;
    // 折射率
    float refractiveIndex;
    // 反射光强
    float reflectIntensity;
    // 折射光强
    float refractIntensity;
    // Texture
    Texture texture;
    bool hasTexture;
    Vector3f type;     // （漫反射，镜面反射, 折射）
    Vector3f emission; // 发光系数
};

#endif // MATERIAL_H
