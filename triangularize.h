#pragma once
#include "mutex"
#include "vector"

using BYTE = unsigned char;

class RenderBufferData;
class CubeSharedData;

void triangularize_continuously(CubeSharedData * in_data,RenderBufferData * out_data);

class RenderBufferData{
    //Assumes that the same thread does not call update_check and write values.
protected:
    bool has_written = false;
    std::mutex has_written_lock;
    std::vector<BYTE> cube_colors;
    std::vector<float> cube_verticies;
    std::vector<BYTE> write_cube_colors;
    std::vector<float> write_cube_verticies;
public:
    bool update_check();
    std::vector<BYTE> & get_colors(){
        return cube_colors;
    }
    std::vector<float> & get_verticies(){
        return cube_verticies;
    }
    void set_vals(std::vector<BYTE> & colors,std::vector<float> & verticies);
};
