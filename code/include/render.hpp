#ifndef RENDER_H
#define RENDER_H
#include "scene_parser.hpp"
#include "image.hpp"
#include "camera.hpp"
#include "group.hpp"
#include "light.hpp"
#include "utils.hpp"
#include "material.hpp"
#include <string>

class Render
{
public:
    Render(const std::string &inputFile, const std::string &outputFile)
    {
        parser = new SceneParser(inputFile.c_str());
        group = parser->getGroup();
        group->buildBVH();
        camera = parser->getCamera();
        width = camera->getWidth();
        height = camera->getHeight();
        image = new Image(width, height);
        this->outputFile = outputFile;
    }
    ~Render()
    {
        delete parser;
        delete group;
        delete camera;
        delete image;
    }
    virtual void renderScene() = 0;

protected:
    SceneParser *parser;
    Group *group;
    Camera *camera;
    Image *image;
    int width;
    int height;
    std::string outputFile;
};
class PT : public Render
{
public:
    PT(const std::string &inputFile, const std::string &outputFile) : Render(inputFile, outputFile)
    {
        printf("ParsingPT\n");
    }
    void renderScene() override;
    Vector3f castRay(Ray ray, int depth);
    Vector3f shade(Hit &hit, Vector3f wo);
    void sampleLight(Hit &hit, float &pdf);
    void printProgress(int s, int samples, clock_t startTime);
private:
    int samples = 1000;
};
void PT::renderScene() 
{
    group->buildBVH();
    clock_t start;
    start = clock();
    //打印宽度高度和采样次数
    printf("width:%d, height:%d, samples:%d\n", width, height, samples);
    for (int i = 0; i < width; i++)
    {
        printf("i:%d\n", i);
        for (int j = 0; j < height; j++)
        {
            printf("j:%d\n", j);
            printProgress(i * height + j + height + 1, width * height, start);
            Vector3f color = Vector3f::ZERO;
            for (int s = 0; s < samples; s++)
            {
                printf("s:%d\n", s);
                Ray ray = camera->generateRay(Vector2f(i + RND1, j + RND1));
                color += castRay(ray, 0) / samples;
                printf("color:%f,%f,%f\n", color.x(), color.y(), color.z());
            }
            image->SetPixel(i, j, color);
        }
    }
    //保存图片
    std::string outputFileName = outputFile + ".bmp";
    image->SaveBMP(outputFileName.c_str());
}
Vector3f PT::shade(Hit &hit, Vector3f wo)
{
    if (hit.getMaterial()->hasEmission())
    {
        return hit.getMaterial()->getEmission();
    }
    // 计算直接光照
    Vector3f Ld = Vector3f::ZERO;
    {
        float light_pdf;
        Hit hit_light;
        sampleLight(hit_light, light_pdf);
        //物体指向采样点的方向
        Vector3f objToLight = hit_light.coords - hit.coords;
        Vector3f objToLightDir = objToLight.normalized();
        Hit hit;
        //判断是否有阻挡
        bool isIntersected = group->intersect(Ray(hit.coords, objToLightDir), hit, 1e-3);
        if(hit.getT() - objToLight.length() > -EPLISON)
        {
            Vector3f f_r = hit.getMaterial()->eavl(-objToLightDir, wo, hit.getNormal());
            float r2 = Vector3f::dot(objToLight, objToLight);
            float cosA = std::max(.0f, Vector3f::dot(hit.getNormal(), objToLightDir));
            float cosB = std::max(.0f, Vector3f::dot(hit_light.getNormal(), -objToLightDir));
            Ld = hit_light.getMaterial()->getEmission() * f_r * cosA * cosB / r2 / light_pdf;
        }
    }
    // 计算间接光照
    Vector3f Lr = Vector3f::ZERO;
    {
        if(RND2 < RussianRoulette)
        {
            float pdf =.0f;
            Vector3f nextDir = Vector3f::ZERO;
            hit.getMaterial()->sample(-wo, hit.getNormal(), nextDir, pdf);
            Hit next_hit;
            bool isIntersected = group->intersect(Ray(hit.coords, nextDir), next_hit, 1e-3);
            if(isIntersected && !hit.getMaterial()->hasEmission())
            {
                Vector3f f_r = next_hit.getMaterial()->eavl(-nextDir, wo, next_hit.getNormal());
                float cos = std::max(.0f, Vector3f::dot(hit.getNormal(), nextDir));
                Lr = shade(next_hit, -nextDir) * f_r * cos / pdf / RussianRoulette;
            }
        }
    }
    return Ld + Lr;
}
void PT::sampleLight(Hit &hit, float &pdf)
{
    float emit_area_sum = 0;
    for(int i = 0;i<group->getGroupSize();i++)
    {
        if(group->getObject(i)->getMaterial()->hasEmission())
        {
            emit_area_sum += group->getObject(i)->getArea();
        }
    }
    float p = RND2 * emit_area_sum;
    emit_area_sum = 0;
    for(int i = 0;i<group->getGroupSize();i++)
    {
        if(group->getObject(i)->getMaterial()->hasEmission())
        {
            emit_area_sum += group->getObject(i)->getArea();
            if(p<emit_area_sum)
            {
                group->getObject(i)->sample(hit, pdf);
                break;
            }
        }
    }
}
void PT::printProgress(int iter, int maxIter, clock_t startTime)
{
    // 打印进度
    if (iter % 100 == 0)
    {
        clock_t endTime = clock();
        float time = (float)(endTime - startTime) / CLOCKS_PER_SEC;
        float timePerIter = time / iter;
        float timeLeft = timePerIter * (maxIter - iter);
        printf("Progress: %d/%d, Time used: %.2fs, Time left: %.2fs\r", iter, maxIter, time, timeLeft);
        fflush(stdout);
    }
}
Vector3f PT::castRay(Ray ray, int depth)
{
    Hit hit;
    bool isIntersected = group->intersect(ray, hit, 1e-3);
    if (!isIntersected)
    {
        return Vector3f::ZERO;
    }
    return shade(hit, -ray.getDirection());
}
#endif // RENDER_H