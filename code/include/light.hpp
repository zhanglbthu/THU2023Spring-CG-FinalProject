#ifndef LIGHT_H
#define LIGHT_H

#include <Vector3f.h>
#include "object3d.hpp"
#include "utils.hpp"

class Light
{
public:
    Light() = default;

    virtual ~Light() = default;
    // 获取光源颜色
    virtual Vector3f getColor() const = 0;
    virtual void getIllumination(const Vector3f &p, Vector3f &dir, Vector3f &col) const = 0;
    virtual Ray randomlyEmitRay() const = 0;
    // 获取光强
    Vector3f getEmission() const
    {
        return emission;
    }

protected:
    // 光强
    Vector3f emission;
};

class DirectionalLight : public Light
{
public:
    DirectionalLight() = delete;

    DirectionalLight(const Vector3f &d, const Vector3f &c)
    {
        direction = d.normalized();
        color = c;
    }

    ~DirectionalLight() override = default;

    ///@param p unsed in this function
    ///@param distanceToLight not well defined because it's not a point light
    void getIllumination(const Vector3f &p, Vector3f &dir, Vector3f &col) const override
    {
        // the direction to the light is the opposite of the
        // direction of the directional light source
        dir = -direction;
        col = color;
    }
    Ray randomlyEmitRay() const override
    {
        // 光线的起点为无穷远处
        Vector3f origin = Vector3f(INFINITY, INFINITY, INFINITY);
        // 光线的方向为光源的方向
        Vector3f direction = this->direction.normalized();
        return Ray(origin, direction);
    }
    // 获取光源颜色
    Vector3f getColor() const override
    {
        return color;
    }

private:
    Vector3f direction;
    Vector3f color;
};

class PointLight : public Light
{
public:
    PointLight() = delete;

    PointLight(const Vector3f &p, const Vector3f &c)
    {
        position = p;
        color = c;
    }

    ~PointLight() override = default;

    void getIllumination(const Vector3f &p, Vector3f &dir, Vector3f &col) const override
    {
        // the direction to the light is the opposite of the
        // direction of the directional light source
        dir = (position - p);
        dir = dir / dir.length();
        col = color;
    }
    // 获取光源颜色
    Vector3f getColor() const override
    {
        return color;
    }
    // 随机发出光线
    Ray randomlyEmitRay() const override
    {
        // 点光源随机发出光线
        //  光线的起点为光源的位置
        Vector3f origin = position;
        // 光线的方向为随机方向
        Vector3f direction = random_cosine_direction().normalized();
        return Ray(origin, direction);
    }
    // 计算随机方向
    Vector3f random_cosine_direction() const
    {
        // 生成随机数 u1 和 u2
        float u1 = RND2;
        float u2 = RND2;

        // 计算随机的极角 theta
        float theta = 2.0f * PI * u1;

        // 计算随机的半径 r
        float r = std::sqrt(u2);

        // 计算随机方向的 x、y 和 z 分量
        float x = std::cos(theta) * 2.0f * std::sqrt(r * (1.0f - r));
        float y = std::sin(theta) * 2.0f * std::sqrt(r * (1.0f - r));
        float z = 1.0f - 2.0f * r;

        return Vector3f(x, y, z);
    }

private:
    Vector3f position;
    Vector3f color;
};
class DiskLight : public Light
{
public:
    DiskLight() = delete;

    DiskLight(const Vector3f &p, const Vector3f &d, const Vector3f &c, float r, Vector3f emission)
    {
        position = p;
        direction = d.normalized();
        color = c;
        radius = r;
        this->emission = emission;

        //TODO:计算x轴和y轴
        if (direction.x() < 1e-4 && direction.x() > -1e-4)
            if (direction.z() < 1e-4 && direction.z() > -1e-4)
                if (1 - direction.y() < 1e-4 || 1 + direction.y() < 1e-4)
                {
                    x_axis = Vector3f::RIGHT;
                    y_axis = Vector3f::FORWARD;
                    return;
                }
        if (abs(direction.x()) < 1e-3 && abs(direction.z()) < 1e-3)
        { // 和y轴平行
            x_axis = Vector3f(1, 0, 0);
            y_axis = Vector3f(0, 0, 1);
        }
        else
        {
            x_axis = Vector3f::cross(direction, Vector3f::UP).normalized();
            y_axis = Vector3f::cross(direction, x_axis).normalized();
        }
    }

    ~DiskLight() override = default;

    void getIllumination(const Vector3f &p, Vector3f &dir, Vector3f &col) const override
    {
        // the direction to the light is the opposite of the
        // direction of the directional light source
        dir = (position - p);
        dir = dir / dir.length();
        col = color;
    }
    // 获取光源颜色
    Vector3f getColor() const override
    {
        return color;
    }
    // 随机发出光线
    Ray randomlyEmitRay() const override
    {
        //TODO:随机发出光线
        float alpha = RND2 * 2 * PI;
        Vector3f p = position + radius * cos(alpha) * x_axis + radius * sin(alpha) * y_axis;
        Vector3f new_dir = direction * RND2 + RND1 * x_axis + RND1 * y_axis;
        new_dir.normalize();
        if(new_dir.squaredLength() < 1e-4)
            new_dir = direction;
        return Ray(p, new_dir);
    }

private:
    Vector3f position;
    Vector3f direction;
    Vector3f color;
    float radius;
    Vector3f x_axis;
    Vector3f y_axis;
};

#endif // LIGHT_H
