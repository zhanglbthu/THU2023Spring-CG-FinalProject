#ifndef CURVE_HPP
#define CURVE_HPP

#include "object3d.hpp"
#include <vecmath.h>
#include <vector>
#include <utility>
#include <algorithm>

// TODO (PA2): Implement Bernstein class to compute spline basis function.计算样条基函数
//       You may refer to the python-script for implementation.

// The CurvePoint object stores information about a point on a curve
// after it has been tesselated: the vertex (V) and the tangent (T)
// It is the responsiblility of functions that create these objects to fill in all the data.
struct CurvePoint
{
    Vector3f V; // Vertex
    Vector3f T; // Tangent  (unit)
    float t;
};

Vector3f cuurrent_point = Vector3f::ZERO;
bool compare(const CurvePoint &a, const CurvePoint &b)
{
    return (a.V - cuurrent_point).squaredLength() < (b.V - cuurrent_point).squaredLength();
}

class Curve : public Object3D
{
protected:
    std::vector<Vector3f> controls;
    std::vector<CurvePoint> data;

public:
    explicit Curve(std::vector<Vector3f> points) : controls(std::move(points)){}

    bool intersect(const Ray &r, Hit &h, float tmin) override
    {
        return false;
    }
    std::vector<Vector3f> &getControls()
    {
        return controls;
    }
    void getT(const float &x, const float &y, float &t)
    {
        // 获取与（x，y）最近的点的t值
        // float min_dis = 100000000.0f;
        // for (int i = 0; i < data.size(); i++)
        // {
        //     float dis = (data[i].V - Vector3f(x, y, 0.0f)).squaredLength();
        //     if (dis < min_dis)
        //     {
        //         min_dis = dis;
        //         t = data[i].t;
        //     }
        // }
        cuurrent_point = Vector3f(x, y, 0.0f);
        t = (*std::min_element(data.begin(), data.end(), compare)).t;
    }
    // 检查交点是否在曲面上
    bool check_point(const Vector3f &V)
    {
        // 遍历data找到最小距离
        // Vector3f point_current(-sqrt(V.x() * V.x() + V.z() * V.z()), V.y(), 0.0f);
        // float min_dis = 100000000.0f;
        // for (int i = 0; i < data.size(); i++)
        // {
        //     float dis = (data[i].V.x() - point_current.x()) * (data[i].V.x() - point_current.x()) + (data[i].V.y() - point_current.y()) * (data[i].V.y() - point_current.y());
        //     if (dis < min_dis)
        //     {
        //         min_dis = dis;
        //     }
        // }
        // if (sqrt(min_dis) > 1e-2)
        // {
        //     // printf("min_dis: %f\n", sqrt(min_dis));
        //     return false;
        // }
        // return true;
        cuurrent_point = Vector3f(-sqrt(V.x() * V.x() + V.z() * V.z()), V.y(), 0.0f);
        auto it = std::min_element(data.begin(), data.end(), compare);
        if (((*it).V - cuurrent_point).length() > 1e-2)
            return false;
        return true;
    }
    virtual void getPoint(const float &t, Vector3f &V, Vector3f &T) = 0;
    virtual void discretize(int resolution, std::vector<CurvePoint> &data) = 0;
    // 检查t是否合法
    virtual bool checkT(const float &t) = 0;
    // 修正t的函数
    virtual void fixT(float &t) = 0;

    // 返回data
    std::vector<CurvePoint> getData()
    {
        return data;
    }
};
/// @brief 辅助类，计算Bezier曲线和B样条的基函数
class Bernstein
{
public:
    static float evaluateBasis(int i, int n, float t)
    {
        if (i < 0 || i > n)
        {
            return 0;
        }
        if (n == 0)
        {
            return 1;
        }
        return (1 - t) * evaluateBasis(i, n - 1, t) + t * evaluateBasis(i - 1, n - 1, t);
    }
    static float evaluateBasisDerivative(int i, int n, float t)
    {
        return n * (evaluateBasis(i - 1, n - 1, t) - evaluateBasis(i, n - 1, t));
    }
    static float evaluateBasis(int i, int k, float t, std::vector<float> knots)
    {
        if (k == 0)
        {
            return (t >= knots[i] && t < knots[i + 1]) ? 1.0f : 0.0f;
        }
        else
        {
            return ((t - knots[i]) / (knots[i + k] - knots[i])) * evaluateBasis(i, k - 1, t, knots) + ((knots[i + k + 1] - t) / (knots[i + k + 1] - knots[i + 1])) * evaluateBasis(i + 1, k - 1, t, knots);
        }
    }
    static float evaluateBasisDerivative(int i, int k, float t, std::vector<float> knots)
    {
        float left = evaluateBasis(i, k - 1, t, knots);
        float right = evaluateBasis(i + 1, k - 1, t, knots);
        return k * ((left / (knots[i + k] - knots[i])) - (right / (knots[i + k + 1] - knots[i + 1])));
    }
};

class BezierCurve : public Curve
{
public:
    explicit BezierCurve(const std::vector<Vector3f> &points) : Curve(points)
    {
        if (points.size() < 4 || points.size() % 3 != 1)
        {
            printf("Number of control points of BezierCurve must be 3n+1!\n");
            exit(0);
        }
        this->discretize(1000, data);
    }
    bool checkT(const float &t) override
    {
        return t >= 0.0f && t <= 1.0f;
    }
    void fixT(float &t) override
    {
        if (t < 0.0f)
        {
            t = 0.0f;
        }
        if (t > 1.0f)
        {
            t = 1.0f;
        }
    }
    void getPoint(const float &t, Vector3f &V, Vector3f &T) override
    {
        int n = controls.size() - 1;
        V = Vector3f::ZERO;
        T = Vector3f::ZERO;
        for (int i = 0; i <= n; i++)
        {
            float basis = Bernstein::evaluateBasis(i, n, t);
            float basisDerivative = Bernstein::evaluateBasisDerivative(i, n, t);
            V += basis * controls[i];
            T += basisDerivative * controls[i];
        }
        // printf("myV: %f %f %f\n", V.x(), V.y(), V.z());
        // printf("myT: %f %f %f\n", T.x(), T.y(), T.z());

        // std::vector<Vector3f> A = controls;
        // V = Vector3f::ZERO;
        // T = Vector3f::ZERO;
        // for(int i = 0; i < n; i++)
        //     for(int j = 0; j < n - i; j++)
        //     {
        //         if(i == n - 1)
        //             T = A[1] - A[0];
        //         A[j] = (1 - t) * A[j] + t * A[j + 1];
        //     }
        // V = A[0];
        // printf("V: %f %f %f\n", V.x(), V.y(), V.z());
        // printf("T: %f %f %f\n", T.x(), T.y(), T.z());
    }
    void discretize(int resolution, std::vector<CurvePoint> &data) override
    {
        data.clear();
        int n = controls.size() - 1;
        for (int i = 0; i < resolution; i++)
        {
            float t = (float)i / (float)(resolution - 1);
            CurvePoint point;
            point.V = Vector3f::ZERO;
            point.T = Vector3f::ZERO;
            point.t = t;
            for (int j = 0; j <= n; j++)
            {
                float basis = Bernstein::evaluateBasis(j, n, t);
                float basisDerivative = Bernstein::evaluateBasisDerivative(j, n, t);
                point.V += basis * controls[j];
                point.T += basisDerivative * controls[j];
            }
            point.T.normalize();
            data.push_back(point);
        }
    }
    AABB getAABB() override
    {
        return AABB(Vector3f(-INFINITY, -INFINITY, -INFINITY), Vector3f(INFINITY, INFINITY, INFINITY));
    }

protected:
};

class BsplineCurve : public Curve
{
public:
    BsplineCurve(const std::vector<Vector3f> &points) : Curve(points)
    {
        if (points.size() < 4)
        {
            printf("Number of control points of BspineCurve must be more than 4!\n");
            exit(0);
        }
        this->discretize(100, data);
        // 打印data信息
        //  printf("data:\n");
        //  for (int i = 0; i < data.size(); i++)
        //  {
        //      printf("V:%f %f %f\n", data[i].V.x(), data[i].V.y(), data[i].V.z());
        //  }
    }
    bool checkT(const float &t) override
    {
        return (t >= k / (float)(n + k + 1)) && (t <= (n + 1) / (float)(n + k + 1));
    }
    void fixT(float &t) override
    {
        if (t < k / (float)(n + k + 1))
        {
            t = k / (float)(n + k + 1) + FLT_EPSILON;
        }
        if (t > (n + 1) / (float)(n + k + 1))
        {
            t = (n + 1) / (float)(n + k + 1) - FLT_EPSILON;
        }
    }
    void getPoint(const float &t, Vector3f &V, Vector3f &T) override
    {
        int m = n + k + 1;
        V = Vector3f::ZERO;
        T = Vector3f::ZERO;
        std::vector<float> knotVector(m + 1, 0.0f);
        for (int i = 0; i <= m; i++)
        {
            knotVector[i] = i / (float)(n + k + 1);
        }
        for (int i = 0; i <= n; i++)
        {
            float basisValue = Bernstein::evaluateBasis(i, k, t, knotVector);
            float basisDerivative = Bernstein::evaluateBasisDerivative(i, k, t, knotVector);
            V += basisValue * controls[i];
            T += basisDerivative * controls[i];
        }
    }
    void discretize(int resolution, std::vector<CurvePoint> &data) override
    {
        data.clear();
        // TODO (PA2): fill in data vector
        int m = n + k + 1;
        std::vector<float> knotVector(m + 1, 0.0f);
        for (int i = 0; i <= m; i++)
        {
            knotVector[i] = i / (float)(n + k + 1);
        }
        for (int i = 0; i < resolution; i++)
        {
            float t = knotVector[k] + (i / (float)(resolution - 1)) * (knotVector[n + 1] - knotVector[k]);
            CurvePoint point;
            point.V = Vector3f::ZERO;
            point.T = Vector3f::ZERO;
            point.t = t;
            for (int j = 0; j <= n; j++)
            {
                float basisValue = Bernstein::evaluateBasis(j, k, t, knotVector);
                float basisDerivative = Bernstein::evaluateBasisDerivative(j, k, t, knotVector);
                point.V += basisValue * controls[j];
                point.T += basisDerivative * controls[j];
            }
            point.T.normalize();
            data.push_back(point);
        }
    }
    AABB getAABB() override
    {
        return AABB(Vector3f(-INFINITY, -INFINITY, -INFINITY), Vector3f(INFINITY, INFINITY, INFINITY));
    }

protected:
    int n = controls.size() - 1;
    int k = 3;
};
#endif // CURVE_HPP