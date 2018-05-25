#pragma once
#include <vector>
#include "cube_color.h"

struct CubeCoord{
    int x;
    int y;
    int z;
};

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
    CubeCoord cube_facing(){
        int inc_axis = tangent_axis();//tangent axis to plane of face
        CubeCoord val = base_coord;
        assert(inc_axis >= 0 && inc_axis < 3);
        int dir = reversed ? 1 : -1;
        switch(inc_axis){
            case 0: val.x += dir; break;
            case 1: val.y += dir; break;
            case 2: val.z += dir; break;
        }
        return val;
    }
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
