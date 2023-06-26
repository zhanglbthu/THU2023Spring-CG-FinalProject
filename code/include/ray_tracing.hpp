#ifndef RAY_TRACING_H
#define RAY_TRACING_H

#include "scene_parser.hpp"
#include "image.hpp"
#include "camera.hpp"
#include "group.hpp"
#include "light.hpp"
#include "utils.hpp"
#include "material.hpp" // 需要包含材质类的头文件
#include <string>
class Ray_Tracing
{
public:
    Ray_Tracing();
    void renderScene(const std::string &inputFile, const std::string &outputFile, int maxDepth);
    int numIter = 100; // 迭代次数
    std::vector<Vector3f> colors; // 保存每次迭代的颜色
    // 其他必要的方法和成员声明

private:
    Vector3f traceRay(const Ray &ray, const SceneParser& parser, int depth, float weight);
    // 私有成员和方法声明
};

#endif // RAY_TRACING_H
