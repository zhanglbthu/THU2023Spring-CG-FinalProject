#include <ray_tracing.hpp>

void printVector3f(const Vector3f &v)
{
    std::cout << "(" << v.x() << ", " << v.y() << ", " << v.z() << ")" << std::endl;
}
Ray_Tracing::Ray_Tracing()
{
    // 构造函数的实现
}

void Ray_Tracing::renderScene(const std::string &inputFile, const std::string &outputFile, int maxDepth)
{
    // 解析场景文件
    SceneParser *parser = new SceneParser(inputFile.c_str());
    Camera *camera = parser->getCamera();
    int w = camera->getWidth();
    int h = camera->getHeight();
    Image *image = new Image(w, h);
    float weight = 1;
    for(int i = 0; i < w;i++)
    {
        for(int j = 0; j < h;j++)
        {
            colors.push_back(Vector3f::ZERO);
        }
    }
    // 实现光线追踪的代码

        for (int i = 0; i < w; i++)
        {
            for (int j = 0; j < h; j++)
            {
                //打印进度progress
                if (i % 10 == 0 && j == 0)
                {
                    std::cout << "progress: " << i << "/" << w << std::endl;
                }
                Vector3f color = Vector3f::ZERO;
                for(int iter = 0; iter < numIter; iter++)
                {
                    // 发射光线
                    Ray ray = camera->generateRay(Vector2f(i + RND1, j + RND1));
                    color += traceRay(ray, *parser, maxDepth, weight);
                    image->SetPixel(i, j, color/(iter+1));
                }
            }
        }

    
    // 保存渲染结果为BMP文件
    image->SaveBMP(outputFile.c_str());

    delete image;
    delete camera;
}

Vector3f Ray_Tracing::traceRay(const Ray &ray, const SceneParser &parser, int depth, float weight)
{
    if (depth == 0 || weight < 0.0001)
    {
        return Vector3f::ZERO;
    }
    Hit hit;
    bool isIntersected = parser.getGroup()->intersect(ray, hit, 0);
    if (isIntersected)
    {
        float type = RND2;
        Material *material = hit.getMaterial();
        Vector3f finalColor = Vector3f::ZERO;

        Vector3f direction = ray.getDirection().normalized();
        Vector3f newOrigin = ray.pointAtParameter(hit.getT());
        Vector3f normal = hit.getNormal().normalized();
        // 计算直接光照
        for (int li = 0; li < parser.getNumLights(); li++)
        {
            Light *light = parser.getLight(li);
            Vector3f L, lightColor;
            light->getIllumination(ray.pointAtParameter(hit.getT()), L, lightColor);
            finalColor += hit.getMaterial()->Shade(ray, hit, L, lightColor);
        }

        float reflect_weight = material->getType().y();
        float refract_weight = material->getType().z();

        if (reflect_weight > 0)
        {
            // 计算镜面反射
            Vector3f reflectDir = ray.reflectDir(direction, normal);
            Ray reflectRay(newOrigin, reflectDir);
            finalColor += weight * traceRay(reflectRay, parser, depth--, weight * 0.2);
        }
        if (refract_weight > 0)
        {
            // 计算折射
            float n = material->getRefractiveIndex();
            Vector3f refractDir = ray.refractDir(direction, normal, n);
            Ray refractRay(newOrigin, refractDir);
            finalColor += weight * traceRay(refractRay, parser, depth--, 0.2 * weight);
        }
        return finalColor;
    }
    else
    {
        return Vector3f::ZERO;
    }
}