#ifndef PATH_TRACING_H
#define PATH_TRACING_H

#include "scene_parser.hpp"
#include "image.hpp"
#include "camera.hpp"
#include "group.hpp"
#include "light.hpp"
#include "material.hpp"
#include "ray_utils.hpp"
#include <string>
class Path_Tracing
{
public:
    Path_Tracing();
    void renderScene(const std::string &inputFile, const std::string &outputFile);
    int numIter = 1000; // 迭代次数
    // 其他必要的方法和成员声明

private:
    Vector3f traceRay(Ray &ray, int depth);
    //打印进度，已用时间和剩余时间
    void printProgress(int iter, int maxIter, clock_t startTime);
    SceneParser *parser;
    Group *group;
    // 私有成员和方法声明
};

#endif // PATH_TRACING_H
