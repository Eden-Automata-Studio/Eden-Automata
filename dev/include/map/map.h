#ifndef MAP_H
#define MAP_H

#include <iostream>
#include <cstdint>
#include <cmath>

// 模拟二维数组访问
#define INDEX(x, y, width) y * width + x
// 从角度转换为弧度
#define DEG2RAD(degree) ((double_t)degree) * M_PI / 180.0
// 平滑插值
#define FADE(t) 6 * pow(t, 5.0) - 15 * pow(t, 4.0) + 10 * pow(t, 3.0)
// 线性插值
#define INTERPOLATION(a, f, c) a + f * (c - a)
#define SCALE_INTERPOLATE(x, n) (uint64_t)(x * n)


/// @brief 表示W x L的地图，单位是米
class Map{
public:
    uint32_t width;     // 地图的宽度
    uint32_t length;    // 地图的长度
    uint32_t seed;      // 种子
    uint64_t* mapPtr;   // 地图数据指针，值为地图上该点移动难度
    

    /// @brief 通过指定长宽来构造一个随机权重的地图
    /// @param width 
    /// @param length 
    Map(uint32_t width, uint32_t length, uint32_t seed) : width(width), length(length), seed(seed) {
        mapPtr = new uint64_t[width*length];
        constructSelf(seed);
    }

    void constructSelf(uint32_t seed);

    uint64_t* generateBerlinNoiceMap(uint32_t seed, uint64_t maximum, double_t scale_factor);

    uint64_t berlinNoice(uint64_t x, uint64_t y);
};


#endif