#pragma once
#include <glm/common.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iostream>

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
