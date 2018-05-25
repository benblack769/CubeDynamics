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
    int axis;
    int num_verticies_to_buffer(){
        return 2*3;
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
