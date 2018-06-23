#pragma once
#include "cube_color.h"
#include "cube_coords.h"
#include "cube_info.h"
#include <cstdlib>
#include <vector>

constexpr int size_cube = 100;
class CubeData{
    std::vector<CubeInfo> data;
    std::vector<float> bond_data;
public:
    CubeData();
    CubeInfo & get(CubeCoord c);
    float & get_bond(CubeCoord coord, CubeCoord dir);
    std::vector<FaceDrawInfo> get_exposed_faces();
    void update(CubeData & update_data);
    void update_bonds_with_mass_changep(CubeCoord source, CubeCoord dest, float solid_quantity_moved);
};
