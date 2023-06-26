#ifndef SPPM_H
#define SPPM_H

#include "scene_parser.hpp"
#include "image.hpp"
#include "camera.hpp"
#include "group.hpp"
#include "light.hpp"
#include "material.hpp"
#include "ray.hpp"
#include "utils.hpp"
#include "kd_tree.hpp"
#include "constant.hpp"
#include "sphere.hpp"
#include <ctime>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

namespace ppm
{
    Vector3f absolute(const Vector3f &v)
    {
        return Vector3f(fabs(v.x()), fabs(v.y()), fabs(v.z()));
    }
    float max(const Vector3f &v)
    {
        return std::max(std::max(v.x(), v.y()), v.z());
    }
    float min(const Vector3f &v)
    {
        return std::min(std::min(v.x(), v.y()), v.z());
    }
    // 可见点
    struct HitPoint
    {
        Vector3f baseColor, position, normal, flux;
        double radius;
        unsigned int photonCount;
        int x, y;
        bool light = false;
    };
    struct List
    {
        HitPoint *id;
        List *next;
    };
    struct pixelColor
    {
        Vector3f color;
        int x, y;
    };

    List *ListAdd(HitPoint *i, List *h)
    {
        List *list = new List;
        list->id = i;
        list->next = h;
        return list;
    }
    List **hash_grid;
    List *hitpoints = nullptr;
    pixelColor *pixels;
    // 存储图像颜色的二维数组
    Vector3f **imageColor;
    // 初始化
    void init(int w, int h)
    {
        imageColor = new Vector3f *[w];
        for (int i = 0; i < w; i++)
        {
            imageColor[i] = new Vector3f[h];
        }
    }
    AABB hitpointsBB;

    // typedef unordered_map<string, double> pl;
    unsigned int hash_num;
    float hash_size;

    inline unsigned int hashNumber(const int ix, const int iy, const int iz)
    {
        return (unsigned int)((ix * 73856093) ^ (iy * 19349663) ^ (iz * 83492791)) % hash_num;
    }
    void build_hash_grid(const int w, const int h)
    {
        // add all HitPoints to BB
        hitpointsBB.reset();
        List *lst = hitpoints;
        while (lst != NULL)
        {
            HitPoint *hp = lst->id;
            lst = lst->next;
            hitpointsBB.fit(hp->position);
        }

        // create initial radius
        Vector3f ssize = hitpointsBB.max - hitpointsBB.min;
        float irad = ((ssize.x() + ssize.y() + ssize.z()) / 3.0) / ((w + h) / 2.0) * 2.0;

        // add all HitPoints + radius to BB
        hitpointsBB.reset();
        lst = hitpoints;
        while (lst != NULL)
        {
            HitPoint *hp = lst->id;
            lst = lst->next;
            hp->radius = irad * irad;
            hp->photonCount = 0;
            hp->flux = Vector3f::ZERO;
            hash_num++;
            hitpointsBB.fit(hp->position - irad);
            hitpointsBB.fit(hp->position + irad);
        }

        // make each grid cell two times larger than the initial radius
        hash_size = 1.0 / (irad * 2.0);

        // build the hash table
        hash_grid = new List *[hash_num];
        for (unsigned int i = 0; i < hash_num; i++)
            hash_grid[i] = NULL;
        lst = hitpoints;
        while (lst != NULL)
        {
            HitPoint *hp = lst->id;
            lst = lst->next;

            Vector3f BMin = ((hp->position - irad) - hitpointsBB.min) * hash_size;
            Vector3f BMax = ((hp->position + irad) - hitpointsBB.min) * hash_size;

            int xAdd = 0, yAdd = 0, zAdd = 0;
            if (BMin.x() == BMax.x())
                xAdd = 1;
            if (BMin.y() == BMax.y())
                yAdd = 1;
            if (BMin.z() == BMax.z())
                zAdd = 1;

            for (int iz = abs(int(BMin.z())) - zAdd; iz <= abs(int(BMax.z())) + zAdd; iz++)
            {
                for (int iy = abs(int(BMin.y())) - yAdd; iy <= abs(int(BMax.y())) + yAdd; iy++)
                {
                    for (int ix = abs(int(BMin.x())) - xAdd; ix <= abs(int(BMax.x())) + xAdd; ix++)
                    {
                        int hv = hashNumber(ix, iy, iz);
                        hash_grid[hv] = ListAdd(hp, hash_grid[hv]);
                    }
                }
            }
        }
    }
    int rounds = 1000;
    int photons = 10000000;
    int samples = 10;
    Ray sphere(Object3D *light, double u1, double u2)
    {
        Sphere *s = dynamic_cast<Sphere *>(light);
        double theta = 2 * PI * u1;
        double phi = acos(1 - 2 * u2);
        double x = sin(phi) * cos(theta);
        double y = sin(phi) * sin(theta);
        double z = cos(phi);
        Vector3f d = Vector3f(x, y, z);
        return Ray(s->getCenter() + d * s->getRadius() * (1.1), Vector3f(x, y, z));
    }
    void ons(const Vector3f &v1, Vector3f &v2, Vector3f &v3)
    {
        if (std::abs(v1.x()) > std::abs(v1.y()))
        {
            // project to the y = 0 plane and construct a normalized orthogonal vector in this plane
            float invLen = 1.f / sqrtf(v1.x() * v1.x() + v1.z() * v1.z());
            v2 = Vector3f(-v1.z() * invLen, 0.0f, v1.x() * invLen);
        }
        else
        {
            // project to the x = 0 plane and construct a normalized orthogonal vector in this plane
            float invLen = 1.0f / sqrtf(v1.y() * v1.y() + v1.z() * v1.z());
            v2 = Vector3f(0.0f, v1.z() * invLen, -v1.y() * invLen);
        }
        v3 = Vector3f::cross(v1, v2);
    }
    Vector3f hemisphere(double u1, double u2)
    {
        const double r = sqrt(u1 * u1);
        const double phi = 2 * PI * u2;
        return Vector3f(cos(phi) * r, sin(phi) * r, sqrt(1 - u1 * u1));
    }
    bool isZero(const Vector3f &v)
    {
        return v.x() < 1e-3 && v.y() < 1e-3 && v.z() < 1e-3;
    }
    bool compareByIndex(const VisiblePoint &a, const VisiblePoint &b)
    {
        return a.index < b.index;
    }
    // 随机渐进式光子映射类
    class SPPM
    {
    public:
        SPPM(int numPhoton, int numIter, const std::string &inputFile, const std::string &outputFile);
        // 使用随机渐进式光子映射渲染场景的方法
        void renderScene();
        void trace(Ray &ray, const int &depth, bool hitPointPass, Vector3f &flux, Vector3f &baseCl, int pixel_x, int pixel_y);
        Vector3f amcr(const double x, const double y)
        {
            double w = width;
            double h = height;
            float fovx = PI / 4;
            float fovy = (h / w) * fovx;
            return Vector3f(((2 * x - w) / w) * tan(fovx),
                            -((2 * y - h) / h) * tan(fovy),
                            -1.0);
        }

    private:
        // 光子映射的变量
        int numPhoton; // 光子数
        int numIter;   // 迭代次数
        // 存储二维图像中通过每个像素的光线的接触点
        // 可见点集合
        std::vector<VisiblePoint> visiblePoints;
        // 可见点kd树
        KDTree *kdTree;
        // kd树的根节点
        KDTreeNode *root;

        // 场景变量
        // 场景的尺寸
        int width;
        int height;
        // 大小为width*height的像素数组
        std::vector<Vector3f> pixels;
        SceneParser *parser; // 场景解析器

        int total_photon_num = 0;

        Camera *camera;         // 场景相机
        Group *group;           // 场景中的组
        std::string inputFile;  // 输入文件名
        std::string outputFile; // 输出文件名
    };

    SPPM::SPPM(int numPhoton, int numIter, const std::string &inputFile, const std::string &outputFile)
        : numPhoton(numPhoton), numIter(numIter), inputFile(inputFile), outputFile(outputFile), kdTree(nullptr), root(nullptr)
    {
        if (inputFile.empty())
        {
            std::cout << "inputFile is empty!" << std::endl;
            exit(1);
        }
        else
        {
            printf("SPPM initializing...\n");
            parser = new SceneParser(inputFile.c_str());
            group = parser->getGroup();
            group->buildBVH();
            camera = parser->getCamera();
            width = camera->getWidth();
            height = camera->getHeight();
        }
        for (int i = 0; i < width; i++)
        {
            for (int j = 0; j < height; j++)
            {
                visiblePoints.push_back(VisiblePoint(i * height + j));
            }
        }
    }
    void SPPM::trace(Ray &ray, const int &depth, bool hitPointPass, Vector3f &flux, Vector3f &baseCl, int pixel_x, int pixel_y)
    {
        // Russian Roulette
        if (depth >= 5)
        {
            const double rrStopProbability = 0.1;
            if (RND2 < rrStopProbability)
            {
                return;
            }
        }
        Hit intersection;
        bool isIntersect = group->intersect(ray, intersection, 0.0001);
        if (!isIntersect)
            return;
        // Travel the ray to the hit point where the closest object lies and compute the surface normal there.
        Vector3f hpPosition = ray.pointAtParameter(intersection.getT());
        Vector3f normal = intersection.getNormal().normalized();
        ray.setOrigin(hpPosition);

        // 定义随机选择反射类型的变量
        float typeSelect = RND2;
        // 获取交点类型
        Vector3f type = intersection.getMaterial()->getType();

        // 判断反射类型
        if (typeSelect < type.x())
        {
            // 漫反射
            if (hitPointPass)
            {
                // eye ray
                HitPoint *hitpoint = new HitPoint;
                // 判断交点是否发光
                if (intersection.getMaterial()->hasEmission())
                {
                    hitpoint->light = true;
                }
                hitpoint->baseColor = intersection.getColor() * baseCl;
                hitpoint->position = hpPosition;
                hitpoint->normal = normal;
                hitpoint->x = pixel_x;
                hitpoint->y = pixel_y;
                hitpoints = ListAdd(hitpoint, hitpoints);
                flux = intersection.getColor();
            }
            else
            {
                // photon ray
                // 返回绝对值
                Vector3f hashLocation = absolute((hpPosition - hitpointsBB.min) * hash_size);
                List *hitpoints = hash_grid[hashNumber(hashLocation.x(), hashLocation.y(), hashLocation.z())];
                while (hitpoints != nullptr)
                {
                    HitPoint *hitpoint = hitpoints->id;
                    hitpoints = hitpoints->next;

                    if (!hitpoint->light)
                    {
                        Vector3f diff = hitpoint->position - hpPosition; // 交点到光子的距离
                        // check normals to be closer than 90 degree (avoids some edge brightning)
                        if ((Vector3f::dot(hitpoint->normal, normal) > 1e-3) && (Vector3f::dot(diff, diff) <= hitpoint->radius))
                        {
                            float alphaCorrectedPhotonCount = hitpoint->photonCount * ALPHA;
                            double radiusCorrection = (alphaCorrectedPhotonCount + ALPHA) / (alphaCorrectedPhotonCount + 1.0);
                            hitpoint->radius = hitpoint->radius * radiusCorrection;
                            hitpoint->photonCount++;
                            float dist = 1 - (intersection.getT() / (max(hitpointsBB.max)) - max(hitpointsBB.min));
                            //printf("%f\n", dist);
                            Vector3f newFlux = hitpoint->baseColor * flux * (1. / PI);
                            hitpoint->flux = (hitpoint->flux + newFlux * Vector3f(dist, dist, dist)) * radiusCorrection;
                            // 打印flux分量
                            // printf("%f %f %f\n", hitpoint->flux.x(), hitpoint->flux.y(), hitpoint->flux.z());

                            if (intersection.getMaterial()->hasEmission())
                            {
                                hitpoint->flux = hitpoint->flux + intersection.getColor() * intersection.getMaterial()->getEmission() * (PI * 4.0);
                            }
                        }
                    }
                }
                // Russian Roulette
                float p = max(intersection.getColor());
                if (RND2 < p)
                {
                    ray.setDirection(ray.diffuseDir(ray.getDirection(), normal));
                    Vector3f newFlux = intersection.getColor() * flux * 1.0 / p;
                    trace(ray, depth + 1, hitPointPass, newFlux, baseCl, pixel_x, pixel_y);
                }
            }
        }
        else if (typeSelect < type.x() + type.y())
        {
            // 镜面反射
            ray.setDirection(ray.reflectDir(ray.getDirection(), normal));
            Vector3f newFlux = intersection.getColor() * flux;
            Vector3f newbaseCl = intersection.getColor() * baseCl;
            trace(ray, depth + 1, hitPointPass, newFlux, newbaseCl, pixel_x, pixel_y);
        }
        else
        {
            // 折射
            float n = intersection.getMaterial()->getRefractiveIndex();
            ray.setDirection(ray.refractDir(ray.getDirection(), normal, n));
            Vector3f newFlux = intersection.getColor() * flux;
            Vector3f newbaseCl = intersection.getColor() * baseCl;
            trace(ray, depth + 1, hitPointPass, newFlux, newbaseCl, pixel_x, pixel_y);
        }
    }
    void SPPM::renderScene()
    {
        // 解析场景文件
        printf("SPPM begin...\n");
        Camera *camera = parser->getCamera();
        int w = camera->getWidth();
        int h = camera->getHeight();
        Image image(w, h);
        init(w, h);
        // 获取group中emission不为0的物体
        printf("SPPM getting light objects...\n");
        std::vector<Object3D *> lights = group->getLightObjects();
        printf("SPPM rendering...\n");
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                // TODO:超采样

                // 相机发出光线
                for (int s = 0; s < samples; s++)
                {
                    Ray ray = camera->generateRay(Vector2f(x + RND1, y + RND1));
                    Vector3f temp1 = Vector3f(1, 1, 1);
                    Vector3f temp2 = Vector3f(1, 1, 1);
                    trace(ray, 0, true, temp1, temp2, x, y);
                }
                Ray ray = camera->generateRay(Vector2f(x + RND1, y + RND1));
                Vector3f temp1 = Vector3f(1, 1, 1);
                Vector3f temp2 = Vector3f(1, 1, 1);
                trace(ray, 0, true, temp1, temp2, x, y);
            }
        }
        printf("SPPM building hash grid...\n");
        // 打印hitpoints的数量
        //  build the hash table over the measurement points
        build_hash_grid(width, height);
        std::cout << "HitPoints size: " << hash_num << std::endl;
        // 打印每个hitpoint的photonCount
        List *lst = hitpoints;
        while (lst != nullptr)
        {
            HitPoint *hitpoint = lst->id;
            lst = lst->next;
        }
        int updateF = rounds / 10;
        int photons_per_pass = photons / rounds;

        clock_t start = clock();
        for (int i = 1; i <= rounds; i++)
        {
            // 打印进度progress，用时和剩余时间
            double progress = (double)i / rounds;
            clock_t end = clock();
            double time = (double)(end - start) / CLOCKS_PER_SEC;
            double time_left = (double)(rounds - i) / i * time;
            std::cout << "progress: " << progress * 100 << "%, time: " << time << "s, time left: " << time_left << "s" << std::endl;
            for (int j = 0; j < lights.size(); j++)
            {
                for (int photon = 0; photon < 1000; photon++)
                {
                    Ray ray = sphere(lights[j], RND2, RND2);
                    Vector3f rotX, rotY;
                    ons(ray.getDirection(), rotX, rotY);
                    Vector3f sampledDir = hemisphere(RND2, RND2);
                    Vector3f rotatedDir;
                    rotatedDir.x() = Vector3f::dot((rotX.x(), rotY.x(), ray.getDirection().x()), sampledDir);
                    rotatedDir.y() = Vector3f::dot((rotX.y(), rotY.y(), ray.getDirection().y()), sampledDir);
                    rotatedDir.z() = Vector3f::dot((rotX.z(), rotY.z(), ray.getDirection().z()), sampledDir);
                    ray.setDirection(rotatedDir);
                    Vector3f temp = Vector3f(1, 1, 1);
                    Vector3f flux = lights[j]->getMaterial()->getEmission() * lights[j]->getMaterial()->getColor() * PI * 4.0;
                    trace(ray, 0, false, flux, temp, -1, -1);
                }
            }
            // density estimation
            List *lst = hitpoints;
            while (lst != nullptr)
            {
                HitPoint *hitpoint = lst->id;
                lst = lst->next;
                Vector3f color_light = Vector3f::ZERO;
                Vector3f color_normal = Vector3f::ZERO;
                if (hitpoint->light)
                {
                    // color_light = hitpoint->baseColor * 255;
                    imageColor[hitpoint->x][hitpoint->y] += color_light;
                    //image.SetPixel(hitpoint->x, hitpoint->y, hitpoint->baseColor * 255);
                }
                else
                {
                    // 打印光子数量
                    // printf("%d\n", hitpoint->photonCount);
                    color_normal = hitpoint->flux * (1.0 / (PI * hitpoint->radius * i * 1000));
                    // printf("%f %f %f\n", color_normal.x(), color_normal.y(), color_normal.z());
                    imageColor[hitpoint->x][hitpoint->y] += color_normal;
                    //image.SetPixel(hitpoint->x, hitpoint->y, hitpoint->flux * (1.0 / (PI * hitpoint->radius * i * 1000)));
                }
            }
            if (i % updateF == 0)
            {
                // 输出打印图片
                std::cout << "image" << i << " is saving..." << std::endl;
                std::string filename = outputFile + std::to_string(i) + ".bmp";
                // 利用imageColor给image赋值
                for (int i = 0; i < width; i++)
                {
                    for (int j = 0; j < height; j++)
                    {
                        image.SetPixel(i, j, imageColor[i][j] * (1.0 / (samples * updateF)));
                        imageColor[i][j] = Vector3f::ZERO;
                    }
                }
                image.SaveImage(filename.c_str());
            }
        }
        // List *lst1 = hitpoints;
        // while (lst1 != nullptr)
        // {
        //     HitPoint *hitpoint = lst1->id;
        //     lst1 = lst1->next;
        //     //打印flux分量
        //     printf("%f %f %f\n", hitpoint->flux.x(), hitpoint->flux.y(), hitpoint->flux.z());
        // }
    }
}
#endif // SPPM_H