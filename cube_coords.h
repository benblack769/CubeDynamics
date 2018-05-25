#pragma once
#include <vector>

struct CubeCoord{
    int x;
    int y;
    int z;
};
constexpr int faces_per_cube = 12;
constexpr int verticies_per_face = 3;
constexpr int verticies_per_cube = faces_per_cube*verticies_per_face;


void add_cube_vertex(CubeCoord cube,std::vector<float> & vertex_buffer);
