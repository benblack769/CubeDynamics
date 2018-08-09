#pragma once
#include <glm/common.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iostream>
#include <numeric>
#include <random>
#include "parameters.h"

struct CubeCoord{
    int x;
    int y;
    int z;
};

inline CubeCoord add_coord(CubeCoord a,CubeCoord b){
    CubeCoord coord = {a.x + b.x, a.y + b.y, a.z + b.z};
    return coord;
}
inline CubeCoord sub_coord(CubeCoord a,CubeCoord b){
    CubeCoord coord = {a.x - b.x, a.y - b.y, a.z - b.z};
    return coord;
}
inline void debug_print(CubeCoord vec){
    std::cout << "(" << vec.x << ',' << vec.y << ',' << vec.z << ")" << std::endl;
}
inline float square(float x){
    return x * x;
}
inline int sqr(int x){
    return x * x;
}
inline int sqr_len(CubeCoord c){
    return sqr(c.x) + sqr(c.y) + sqr(c.z);
}

using Vec3F = glm::vec4;
inline void debug_print(Vec3F vec){
    std::cout << to_string(vec) << std::endl;
}
inline Vec3F zero_vec(){
    return Vec3F(0,0,0,0);
}
inline Vec3F build_vec(float x, float y, float z){
    return Vec3F(x,y,z,0);
}
inline Vec3F coord_to_vec(CubeCoord c){
    return Vec3F(c.x,c.y,c.z,0);
}
inline float dot_prod(Vec3F v1, Vec3F v2){
    return glm::dot(v1,v2);
}
inline Vec3F normalize(Vec3F v){
    return glm::normalize(v);
}
struct LargeVec{
    float data[LARGE_VEC_SIZE] = {0};
};

inline float sum_lv(LargeVec largevec){
    float sum_val = 0.0f;
    for(int i = 0; i < LARGE_VEC_SIZE; i++){
        sum_val += largevec.data[i];
    }
    return sum_val;
}
inline LargeVec rand_largevec(){
    std::default_random_engine eng;
    std::uniform_real_distribution<float> dist;
    LargeVec res;
    for(int i = 0; i < LARGE_VEC_SIZE; i++){
        res.data[i] = dist(eng);
    }
    return res;
}
inline LargeVec zero_floor(LargeVec vec){
    for(int i = 0; i < LARGE_VEC_SIZE; i++){
        vec.data[i] = std::max(0.0f, vec.data[i]);
    }
}
inline LargeVec zero_lv(){
    return LargeVec();
}
inline LargeVec create_idx(int idx, float val){
    LargeVec res = zero_lv();
    res.data[idx] = val;
    return res;
}
inline void imul_bv(LargeVec * vec,float by){
    for(int i = 0; i < LARGE_VEC_SIZE; i++){
        vec->data[i] *= by;
    }
}
inline void iadd_bv(LargeVec * dest,LargeVec src){
    for(int i = 0; i < LARGE_VEC_SIZE; i++){
        dest->data[i] += src.data[i];
    }
}
inline void isub_bv(LargeVec * dest,LargeVec src){
    for(int i = 0; i < LARGE_VEC_SIZE; i++){
        dest->data[i] -= src.data[i];
    }
}
inline LargeVec add_bv(LargeVec one,LargeVec other){
    for(int i = 0; i < LARGE_VEC_SIZE; i++){
        one.data[i] += other.data[i];
    }
    return one;
}
inline LargeVec add_sca(LargeVec one,float scal){
    for(int i = 0; i < LARGE_VEC_SIZE; i++){
        one.data[i] += scal;
    }
    return one;
}
inline LargeVec sqrt_lv(LargeVec lv){
    for(int i = 0; i < LARGE_VEC_SIZE; i++){
        lv.data[i] = sqrt(lv.data[i]);
    }
    return lv;
}
inline LargeVec mul_bv(LargeVec vec,float by){
    for(int i = 0; i < LARGE_VEC_SIZE; i++){
        vec.data[i] *= by;
    }
    return vec;
}
inline LargeVec mul_e_lv(LargeVec v1,LargeVec v2){
    for(int i = 0; i < LARGE_VEC_SIZE; i++){
        v1.data[i] *= v2.data[i];
    }
    return v1;
}
inline void idiv_lv(LargeVec * vec,float by){
    float byinv = 1.0f/(by+0.0000001f);
    imul_bv(vec,byinv);
}
inline void print_lv(LargeVec lv){
    for(int i = 0; i < LARGE_VEC_SIZE; i++){
        std::cout << lv.data[i] << '\t';
    }
    std::cout << std::endl;
}
