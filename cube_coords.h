#pragma once
#include "cube_color.h"
#include "cross_platform_vector.h"
#include <vector>
#include <cassert>
#include <iostream>

inline std::ostream & operator << (std::ostream & os, CubeCoord v){
    return os << v.x << "  " << v.y << "  " << v.z;
}

constexpr int NUM_AXIS = 3;
struct FaceInfo{
    CubeCoord base_coord;
    bool reversed;
    int _axis;
    int num_verticies_to_buffer(){
        return 2*3;
    }
    int axis_1(){
        return _axis;
    }
    int axis_2(){
        return (_axis+1)%NUM_AXIS;
    }
    int tangent_axis(){
        return (_axis+2)%NUM_AXIS;
    }
    CubeCoord cube_facing();
    void buffer_verticies(std::vector<float> & vertex_buffer);
};

struct FaceDrawInfo{
    RGBVal color;
    FaceInfo face;
    void add_to_buffer(std::vector<BYTE> & color_buffer, std::vector<float> & vertex_buffer){
        for(int i = 0; i < face.num_verticies_to_buffer(); i++){
            color.buffer_colors(color_buffer);
        }
        face.buffer_verticies(vertex_buffer);
    }
};
