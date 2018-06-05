#pragma once
#include <vector>
using BYTE = unsigned char;
struct RGBVal{
    float r;
    float b;
    float g;
    float a;
    void buffer_colors(std::vector<BYTE> & color_buffer){
        color_buffer.push_back(BYTE(r*255));
        color_buffer.push_back(BYTE(g*255));
        color_buffer.push_back(BYTE(b*255));
        color_buffer.push_back(BYTE(a*255));
    }
};
