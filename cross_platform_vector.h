#pragma once

struct CubeCoord{
    int x;
    int y;
    int z;
};
inline CubeCoord add(CubeCoord a,CubeCoord b){
    CubeCoord coord = {a.x + b.x, a.y + b.y, a.z + b.z};
    return coord;
}

#ifndef OPENCL_BUILD
#include <glm/common.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iostream>
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
#else

#define Vec3F float4
#define CubeCoord int4
void debug_print(Vec3F vec){
}
Vec3F zero_vec(){
    Vec3F vec3 = {0,0,0,0};
    return vec3;
}
float4 coord_to_vec(int4 c){
    return c;
}
#endif


