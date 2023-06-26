#ifndef REVSURFACE_HPP
#define REVSURFACE_HPP

#include "object3d.hpp"
#include "curve.hpp"
#include <tuple>
#include <utils.hpp>

class RevSurface : public Object3D
{

    Curve *pCurve;
    AABB aabb;
    float x_max = 0.0f;
    float y_min = 0.0f;
    float y_max = 0.0f;
    std::vector<CurvePoint> data = pCurve->getData();

public:
    RevSurface(Curve *pCurve, Material *material) : pCurve(pCurve), Object3D(material)
    {
        for(const auto &cp : pCurve->getControls())
        {
            if(cp.z() != 0.0)
            {
                printf("Profile of revSurface must be flat on xy plane.\n");
                exit(0);
            }
            x_max = std::max(x_max, abs(cp.x()));
            y_min = std::min(y_min, cp.y());
            y_max = std::max(y_max, cp.y());
        }
    }
    ~RevSurface() override
    {
        delete pCurve;
    }
    float get_f(const Vector3f &dir, const Vector3f &origin, float x, float y)
    {
        float y1 = y - origin.y();
        float a1 = pow((y1 * dir.x() + dir.y() * origin.x()), 2);
        float a2 = pow((y1 * dir.z() + dir.y() * origin.z()), 2);
        float a3 = dir.y() * dir.y() * x * x;
        return a1 + a2 - a3;
    }
    float get_f_grad(const Vector3f &dir, const Vector3f &origin, float x, float y, float x_grad, float y_grad)
    {
        float y1 = y - origin.y();
        float a1 = 2 * (y1 * dir.x() + dir.y() * origin.x()) * dir.x() * y_grad;
        float a2 = 2 * (y1 * dir.z() + dir.y() * origin.z()) * dir.z() * y_grad;
        float a3 = 2 * dir.y() * dir.y() * x * x_grad;
        return a1 + a2 - a3;
    }
    bool intersect(const Ray &r, Hit &h, float tmin) override
    {
        float x_init, y_init;
        bool initIntersect = getXY(r, x_init, y_init);
        if (!initIntersect)
        {
            return false;
        }
        float t = 0.0f;
        Vector3f point = Vector3f::ZERO;
        Vector3f grad = Vector3f::ZERO;
        pCurve->getT(x_init, y_init, t);
        int depth = 0;
        while (depth < 20)
        {
            pCurve->fixT(t);
            if(!pCurve->checkT(t))
            {
                return false;
            }
            pCurve->getPoint(t, point, grad);
            float f = get_f(r.getDirection(), r.getOrigin(), point.x(), point.y());
            float f_grad = get_f_grad(r.getDirection(), r.getOrigin(), point.x(), point.y(), grad.x(), grad.y());
            //打印depth，t,f，f_grad和步长
            //printf("depth: %d, t: %f, f: %f, f_grad: %f, step: %f\n", depth, t, f, f_grad, f / f_grad);
            if (std::abs(f) < 1e-6)
            {
                float tr = 0;
                if (abs(r.getDirection().y()) > 1e-3)
                {
                    tr = (point.y() - r.getOrigin().y()) / r.getDirection().y();
                }
                else
                {
                    float a = r.getDirection().z() * r.getDirection().z() + r.getDirection().x() + r.getDirection().x();
                    float b = 2 * (r.getDirection().z() * r.getOrigin().z() + r.getOrigin().x() * r.getOrigin().x());
                    float c = pow(r.getOrigin().x(), 2) + pow(r.getOrigin().z(), 2) - pow(point.x(), 2);
                    float alpha = pow(b, 2) - 4 * a * c;
                    if(alpha < 0)
                    {
                        return false;
                    }
                    tr = (sqrt(alpha) - b) / (2 * a);
                }
                Vector2f normal_2d = Vector2f(grad.y(), -grad.x()).normalized();
                Vector3f normal_3d = Vector3f::ZERO;
                Vector3f next_origin = r.pointAtParameter(tr);
                if (tr > tmin && tr < h.getT())
                {
                    if (abs(point.x()) < 1e-4)
                    {
                        normal_3d = point.y() >= 0 ? Vector3f(0, 1, 0) : Vector3f(0, -1, 0);
                    }
                    else
                    {
                        float cosTheat = (r.getDirection().x() * tr + r.getOrigin().x()) / point.x();
                        float sinTheat = (r.getDirection().z() * tr + r.getOrigin().z()) / point.x();
                        normal_3d = Vector3f(cosTheat * normal_2d.x(), normal_2d.y(), sinTheat * normal_2d.x()).normalized();
                    }
                    h.set(tr, this->material, normal_3d, next_origin);
                    set_hit_color(h, next_origin);
                    return true;
                }
            }
            float step = f / f_grad;
            t = t - step;
            depth++;
        }
        return false;
    }
    void set_hit_color(Hit &h, const Vector3f &position) override
    {
        if (this->material->getTexture())
        {
            float u = 0.5 + atan2(-position.z(), position.x()) / (2 * PI);
            float v = (y_max - position.y()) / (y_max - y_min);
            // printf("%f %f\n", u, v);
            Vector3f color = this->material->getTextureColor(Vector2f(u, v));
            h.setColor(color);
        }
        else
        {
            h.setColor(material->getColor());
        }
    }
    // 计算射线与圆柱体包围盒的交点对应的二维曲线x,y
    bool getXY(const Ray &r, float &x_init, float &y_init)
    {
        float tmin = 0.0f;
        float tmax = 10000.0f;
        Vector3f min = Vector3f(-x_max, y_min, -x_max);
        Vector3f max = Vector3f(x_max, y_max, x_max);
        //如果射线起始点在包围盒内
        if (abs(r.getOrigin().x()) <= x_max && abs(r.getOrigin().z()) <= x_max && r.getOrigin().y() >= y_min && r.getOrigin().y() <= y_max)
        {
            x_init = r.getOrigin().x() * r.getOrigin().x() + r.getOrigin().z() * r.getOrigin().z();
            x_init = -sqrt(x_init);
            y_init = r.getOrigin().y();
            return true;
        }
        for (int i = 0; i < 3; i++)
        {
            if(abs(r.getDirection()[i]) < 1e-3)
            {
                if(r.getOrigin()[i] < min[i] || r.getOrigin()[i] > max[i])
                {
                    return false;
                }
                else
                {
                    continue;
                }
            }
            float invD = 1.0f / r.getDirection()[i];
            float t0 = (min[i] - r.getOrigin()[i]) * invD;
            float t1 = (max[i] - r.getOrigin()[i]) * invD;
            if (invD < 0.0f)
            {
                std::swap(t0, t1);
            }
            tmin = t0 > tmin ? t0 : tmin;
            tmax = t1 < tmax ? t1 : tmax;
        }
        float t_enter = tmin;
        float t_exit = tmax;
        if (t_enter < t_exit && t_exit > 0)
        {
            float t = t_enter > 0 ? t_enter : t_exit;
            Vector3f point = r.pointAtParameter(t);

            x_init = -sqrt(point.x() * point.x() + point.z() * point.z());
            y_init = point.y();
            return true;
        }
        else
        {
            return false;
        }
    }
    AABB getAABB() override
    {
        return AABB(Vector3f(-x_max, y_min, -x_max), Vector3f(x_max, y_max, x_max));
    }
};

#endif // REVSURFACE_HPP
