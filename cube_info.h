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

struct MassVec{
    float mass;
    glm::vec3 vec;
};
class CubeInfo{
public:
    float quantity;
    glm::vec3 velocity;
    bool is_border;
public:
    CubeInfo(bool in_is_border=false);
    MassVec get_bordering_quantity_vel(glm::vec3 cube_direction);
    void update_velocity_global();
    RGBVal color();
    bool is_transparent();
    void subtract_mass(MassVec removal){
        velocity = (velocity * quantity - removal.vec * removal.mass) / (quantity - removal.mass);
        quantity -= removal.mass;
        assert(quantity >= 0);
    }
    void add_massvec(MassVec addition){
        velocity = (velocity * quantity + addition.vec * addition.mass) / (quantity + addition.mass);
        quantity += addition.mass;
        assert(quantity >= 0);
    }
};
