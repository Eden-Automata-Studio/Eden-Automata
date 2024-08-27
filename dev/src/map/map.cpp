#include "map/map.h"
#include <random>
#include <omp.h>
#include <iostream>
#include <chrono>
#include <cstdlib>
#include <cmath>
#include <png.h>

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
    //#pragma omp parallel
    {    
        srand(seed + omp_get_thread_num());
        //#pragma omp for
        for (uint64_t i = 0; i < this->length * this->width; i++)
        {
            uint64_t rng = rand() % 360;
            double_t magnitude = 1.0;
            double_t rotateAngle = DEG2RAD(rng);
            std::pair<double_t,double_t> vector(magnitude * cos(rotateAngle), magnitude * sin(rotateAngle));
            vecArr[i] = vector;
        }
        //#pragma omp for collapse(2)
        for (uint64_t x = 0; x < this->width; x++)
        {
            for (uint64_t y = 0; y < this->length; y++)
            {
                double_t scaled_x = x * scale_factor;
                double_t scaled_y = y * scale_factor;

                std::pair<uint64_t,uint64_t> leftButtom  ((uint64_t)scaled_x     , (uint64_t)scaled_y    );
                std::pair<uint64_t,uint64_t> leftTop     ((uint64_t)scaled_x     , (uint64_t)scaled_y + 1);
                std::pair<uint64_t,uint64_t> rightButtom ((uint64_t)scaled_x + 1 , (uint64_t)scaled_y    );
                std::pair<uint64_t,uint64_t> rightTop    ((uint64_t)scaled_x + 1 , (uint64_t)scaled_y + 1);

                uint64_t leftButtomIndex = INDEX(leftButtom.first, leftButtom.second, this->width);
                uint64_t leftTopIndex = INDEX(leftTop.first, leftTop.second, this->width);
                uint64_t rightButtomIndex = INDEX(rightButtom.first, rightButtom.second, this->width);
                uint64_t rightTopIndex = INDEX(rightTop.first, rightTop.second, this->width);

                std::pair<double_t,double_t> leftButtomVector = vecArr[leftButtomIndex];
                std::pair<double_t,double_t> leftTopVector    = vecArr[leftTopIndex];
                std::pair<double_t,double_t> rightButtomVector= vecArr[rightButtomIndex];
                std::pair<double_t,double_t> rightTopVector   = vecArr[rightTopIndex];

                std::pair<double_t,double_t> toLeftButtomVector  (((double_t)leftButtom.first)  - scaled_x ,((double_t)leftButtom.second)  - scaled_y);
                std::pair<double_t,double_t> toLeftTopVector     (((double_t)leftTop.first)     - scaled_x ,((double_t)leftTop.second )    - scaled_y);
                std::pair<double_t,double_t> toRightButtomVector (((double_t)rightButtom.first) - scaled_x ,((double_t)rightButtom.second) - scaled_y);
                std::pair<double_t,double_t> toRightTopVector    (((double_t)rightTop.first)    - scaled_x ,((double_t)rightTop.second)    - scaled_y);

                double_t dotProductLeftButtom  = leftButtomVector.first  * toLeftButtomVector.first  + leftButtomVector.second  * toLeftButtomVector.second ;
                double_t dotProductLeftTop     = leftTopVector.first     * toLeftTopVector.first     + leftTopVector.second     * toLeftTopVector.second    ;
                double_t dotProductRightButtom = rightButtomVector.first * toRightButtomVector.first + rightButtomVector.second * toRightButtomVector.second;
                double_t dotProductRightTop    = rightTopVector.first    * toRightTopVector.first    + rightTopVector.second    * toRightTopVector.second   ;

                std::pair<double_t,double_t> localLoc (scaled_x - leftButtom.first, scaled_y - leftButtom.second);
                std::pair<double_t,double_t> localFadeLoc (FADE(localLoc.first), FADE(localLoc.second));
                std::pair<double_t,double_t> interpolationX (INTERPOLATION(dotProductRightButtom,localFadeLoc.second, dotProductLeftTop),
                                                             INTERPOLATION( dotProductLeftButtom, localFadeLoc.second, dotProductRightTop ));
                double_t interpolationY = INTERPOLATION(interpolationX.first , localFadeLoc.first, interpolationX.second);

                uint64_t scaledInterpolation = SCALE_INTERPOLATE(interpolationY, maximum);
                resultArr[INDEX(x,y,this->width)] = scaledInterpolation;
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