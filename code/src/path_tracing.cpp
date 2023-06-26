#include <path_tracing.hpp>

Path_Tracing::Path_Tracing()
{
    // 构造函数的实现
}
// 判断Vector3f的每个值是否均小于1e-3
bool isZero(Vector3f v)
{
    return v.x() < 1e-3 && v.y() < 1e-3 && v.z() < 1e-3;
}
void Path_Tracing::renderScene(const std::string &inputFile, const std::string &outputFile)
{
    // 解析场景文件
    parser = new SceneParser(inputFile.c_str());
    group = parser->getGroup();
    group->buildBVH();
    Camera *camera = parser->getCamera();
    int w = camera->getWidth();
    int h = camera->getHeight();
    Image *image = new Image(w, h);
    // 记录时间
    clock_t start;
    start = clock();
    // 多线程
// 添加多线程命令
// #pragma omp parallel for schedule(dynamic, 1)
    for (int i = 0; i < w; i++)
    {
        // 打印进度和剩余时间
        fprintf(stderr, "\rRendering (%d spp) %5.2f%%", numIter, 100. * i / (w - 1));
        // printProgress(i, w, start);
        for (int j = 0; j < h; j++)
        {
            // 打印进度
            Vector3f color = Vector3f::ZERO;
            for (int iter = 0; iter < numIter; iter++)
            {
                // 发射光线
                Ray ray = camera->generateRay(Vector2f(i + RND1, j + RND1));
                color += traceRay(ray, 0);
            }
            image->SetPixel(i, j, (color / numIter));
        }
    }
    // 保存渲染结果为BMP文件
    // 文件输出名
    std::string outputFileName = outputFile + ".bmp";
    image->SaveBMP(outputFileName.c_str());

    delete image;
    delete camera;
}

Vector3f Path_Tracing::traceRay(Ray &ray, int depth)
{
    // Russian Roulette
    double rrFactor = 1.0;
    if (depth > 5)
    {
        const double rrStopProbability = 0.1;
        if (RND2 <= rrStopProbability)
        {
            return Vector3f::ZERO;
        }
        rrFactor = 1.0 / (1.0 - rrStopProbability);
    }
    Hit hit;
    // 判断是否相交
    bool isIntersect = group->intersect(ray, hit, 1e-3);
    if(!isIntersect)
    {
        return Vector3f::ZERO;
    }
    Vector3f hp_pos = ray.pointAtParameter(hit.getT());
    Vector3f hp_normal = hit.getNormal();
    Vector3f hp_color = hit.getColor();
    Vector3f emission = hit.getMaterial()->getEmission();

    ray.setOrigin(hp_pos);

    Vector3f type = hit.getMaterial()->getType();

    float rnd = RND2;
    if(rnd < type.x())
    {
        // diffuse
        ray.setDirection(RayUtils::diffuse(ray.getDirection(), hp_normal));
        return emission + hp_color * traceRay(ray, depth + 1) * rrFactor; 
    }
    else if(rnd < type.x() + type.y())
    {
        // specular
        ray.setDirection(RayUtils::reflect(ray.getDirection(), hp_normal));
        return emission + hp_color * traceRay(ray, depth + 1) * rrFactor; 
    }
    else
    {
        // refract
        float n = hit.getMaterial()->getRefractionIndex();
        ray.setDirection(RayUtils::refract(ray.getDirection(), hp_normal, n));
        return emission + hp_color * traceRay(ray, depth + 1) * rrFactor; 
    }
}
void Path_Tracing::printProgress(int iter, int maxIter, clock_t startTime)
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