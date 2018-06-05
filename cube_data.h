#pragma once
#include "cube_color.h"
#include "cube_coords.h"
#include "cube_info.h"
#include <cstdlib>
#include <vector>

constexpr int size_cube = 30;
class CubeData{
    std::vector<CubeInfo> data;
public:
    CubeData():
        data(size_cube*size_cube*size_cube){}
    CubeInfo & get(int x, int y, int z);
    CubeInfo & get(CubeCoord c);
    std::vector<FaceDrawInfo> get_exposed_faces();
    void update();
};
