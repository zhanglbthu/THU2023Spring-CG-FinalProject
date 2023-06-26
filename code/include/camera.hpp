#ifndef CAMERA_H
#define CAMERA_H

#include "ray.hpp"
#include <vecmath.h>
#include <float.h>
#include <cmath>

class Camera
{
public:
    Camera(const Vector3f &center, const Vector3f &direction, const Vector3f &up, int imgW, int imgH)
    {
        this->center = center;
        this->direction = direction.normalized();
        this->horizontal = Vector3f::cross(this->direction, up).normalized();
        this->up = Vector3f::cross(this->horizontal, this->direction);
        this->width = imgW;
        this->height = imgH;
    }

    // Generate rays for each screen-space coordinate
    virtual Ray generateRay(const Vector2f &point) = 0;
    virtual ~Camera() = default;

    int getWidth() const { return width; }
    int getHeight() const { return height; }

protected:
    // Extrinsic parameters
    Vector3f center;
    Vector3f direction;
    Vector3f up;
    Vector3f horizontal;
    // Intrinsic parameters
    int width;
    int height;
};

// TODO: Implement Perspective camera
// You can add new functions or variables whenever needed.
class PerspectiveCamera : public Camera
{

public:
    PerspectiveCamera(const Vector3f &center, const Vector3f &direction,
                      const Vector3f &up, int imgW, int imgH, float angle) : Camera(center, direction, up, imgW, imgH)
    {
        this->angle = angle;
        this->imagePlaneDist = 1 / std::tan(angle / 2);
        this->fxy = imgH / (2 * tan(angle / 2));
    }

    Ray generateRay(const Vector2f &point) override
    {
        Vector3f dir_camera(point.x() - width / 2, height / 2 - point.y(), fxy);
        dir_camera.normalize();
        Vector3f dir_world = dir_camera.x() * horizontal - dir_camera.y() * up + dir_camera.z() * direction;
        dir_world.normalize();
        return Ray(this->center, dir_world);
    }
    float getAngle() const
    {
        return angle;
    }
    float getImagePlaneDist() const
    {
        return imagePlaneDist;
    }

protected:
    float angle;
    float imagePlaneDist;
    float fxy;//从图像空间到世界空间的尺度参数
};

// 带景深的相机类
class DepthCamera : public Camera
{
public:
    DepthCamera(const Vector3f &center, const Vector3f &direction,
                const Vector3f &up, int imgW, int imgH, float angle, float focalLength, float aperture) : Camera(center, direction, up, imgW, imgH)
    {
        this->angle = angle;
        this->imagePlaneDist = 1 / std::tan(angle / 2);
        this->focalLength = focalLength;
        this->aperture = aperture;

        this->fxy = imgH / (2 * tan(angle / 2) * (focalLength + 1));
        printf("focalLength: %f, aperture: %f\n", focalLength, aperture);
    }
    Ray generateRay(const Vector2f &point) override
    {
        Vector3f dir_camera(point.x() - width / 2, height / 2 - point.y(), fxy * (focalLength + 1));
        dir_camera.normalize();
        Vector3f dir_world = dir_camera.x() * horizontal - dir_camera.y() * up + dir_camera.z() * direction;
        dir_world.normalize();

        // 生成光圈采样点
        float theta = 2 * PI * RND2;
        float r = RND2 * aperture / 2;
        Vector3f point2 = r * sin(theta) * up + r * cos(theta) * horizontal;
        
        // 生成最终的光线
        Vector3f dir_final = dir_world * ((focalLength + 1) / (Vector3f::dot(dir_world, direction))) - point2;
        dir_final.normalize();
        return Ray(this->center + point2, dir_final);
    }

protected:
    float angle;
    float imagePlaneDist;
    float aperture;//光圈大小
    float focalLength;//焦距
    float fxy;//从图像空间到世界空间的尺度参数
};

#endif // CAMERA_H
