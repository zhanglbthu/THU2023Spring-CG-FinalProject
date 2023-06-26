#ifndef UTILS_HPP
#define UTILS_HPP
#include <random>
#include <cstdlib>
#include <cmath>
#include <omp.h>

//define相机发射光线时二维坐标的随机扰动，用于随机渐进式光子映射，范围为[-0.5, 0.5]
static std::mt19937 mersenneTwister;
static std::uniform_real_distribution<float> uniform;
const float init_radius = 0.5f;
//光子迭代次数
const int photon_iter = 100;
//光子总数
const int photon_num = 10000000;
//定义最大迭代深度
#define MAX_DEPTH 5
const float EPLISON = 1e-5;
const float RussianRoulette = 0.8;
const int NewtonDepth = 20;
//sppm aplha
#define ALPHA 0.7
#define RND (uniform(mersenneTwister) - 0.5) // 生成[-0.5, 0.5]的随机数
#define RND1 (2.0 * uniform(mersenneTwister) - 1.0) //[-1, 1]均匀分布
#define RND2 (uniform(mersenneTwister)) // 生成[0, 1]的随机数
#define PI 3.14159265358979323846264338327950288

inline float clamp(const float &lo, const float &hi, const float &v)
{ return std::max(lo, std::min(hi, v)); }
#endif // UTILS_HPP