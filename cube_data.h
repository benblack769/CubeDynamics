#pragma once
#include "cube_color.h"
#include "cube_coords.h"
#include "cube_info.h"
#include <cstdlib>
#include <vector>

constexpr int size_cube = 22;
class CubeData{
    std::vector<CubeInfo> data;
    std::vector<float> bond_data;
public:
    CubeData();
    CubeInfo & get(CubeCoord c);
    float & get_bond(CubeCoord coord, CubeCoord dir);
    std::vector<FaceDrawInfo> get_exposed_faces();
    void update(CubeData & update_data);
};
