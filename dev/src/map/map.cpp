#include "map/map.h"
#include <random>
#include <omp.h>
#include <iostream>
#include <chrono>
#include <cstdlib>
#include <cmath>
#include <png.h>
#include "map/PerlinNoise.hpp"

void Map::constructSelf(uint32_t seed){
    auto a = generateBerlinNoiceMap(seed, 255, 0.01);
}

void write_png(const char* filename, uint64_t* data, int width, int height) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        perror("fopen");
        return;
    }

    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png) {
        fclose(fp);
        return;
    }

    png_infop info = png_create_info_struct(png);
    if (!info) {
        png_destroy_write_struct(&png, nullptr);
        fclose(fp);
        return;
    }

    if (setjmp(png_jmpbuf(png))) {
        png_destroy_write_struct(&png, &info);
        fclose(fp);
        return;
    }

    png_init_io(png, fp);

    png_set_IHDR(
        png,
        info,
        width, height,
        8, 
        PNG_COLOR_TYPE_GRAY,
        PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_DEFAULT,
        PNG_FILTER_TYPE_DEFAULT
    );
    png_write_info(png, info);

    for (int y = 0; y < height; y++) {
        png_bytep row = (png_bytep) malloc(width * sizeof(png_byte));
        for (int x = 0; x < width; x++) {
            row[x] = static_cast<png_byte>(data[y * width + x] & 0xFF);
        }
        png_write_row(png, row);
        free(row);
    }

    png_write_end(png, nullptr);
    
    png_destroy_write_struct(&png, &info);
    fclose(fp);
}

uint64_t* Map::generateBerlinNoiceMap(uint32_t seed, uint64_t maximum, double_t scale_factor){
    std::pair<double_t,double_t>* vecArr = new std::pair<double_t,double_t>[width*length];
    uint64_t* resultArr = new uint64_t[width*length];
#ifdef DEBUG_PRINT
    auto start = std::chrono::high_resolution_clock::now();
#endif
    #pragma omp parallel
    {    
        const siv::PerlinNoise::seed_type seed_ = seed;

	    const siv::PerlinNoise perlin{ seed_ };
        #pragma omp for collapse(2)
        for (uint64_t x = 0; x < this->width; x++)
        {
            for (uint64_t y = 0; y < this->length; y++)
            {
                const double noise = perlin.octave2D_01(((double_t)(x) * scale_factor), ((double_t)(y) * scale_factor), 14);
                uint64_t scaledNoise = SCALE_INTERPOLATE(noise, maximum);
                resultArr[INDEX(x,y,this->width)] = scaledNoise;
            }

        }
        
    }
    
#ifdef DEBUG_PRINT
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = end - start;
    std::cout << "With omp: " << duration.count()/1000000<< " ms" << std::endl;
    write_png("map1.png",resultArr, this->width, this->length);
#endif
    delete[] vecArr;
    return resultArr;
}