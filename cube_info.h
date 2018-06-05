#pragma once
#include "cube_color.h"
#include "cube_coords.h"
#include <cstdlib>
#include <vector>
#include <iostream>
#include <glm/common.hpp>
#include <glm/geometric.hpp>
#include <algorithm>

constexpr int SIDES_ON_CUBE = 6;

class CubeInfo{
    float quantity;
    glm::vec3 velocity;
    bool is_border;
public:
    CubeInfo(bool in_is_border=false);
    void give_bordering_quantity_vel(glm::vec3 cube_direction, CubeInfo & bordering);
    void update_velocity_global();
    RGBVal color();
    bool is_transparent();
};
