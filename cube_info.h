#pragma once
#include "cube_color.h"
#include "cube_coords.h"
#include <cstdlib>
#include <vector>
#include <iostream>
#include <algorithm>

constexpr int SIDES_ON_CUBE = 6;

inline float square(float x){
    return x * x;
}
struct VectorAttraction{
    Vec3F force_vec;
};
struct QuantityInfo{
    float air_mass;
    float liquid_mass;
    float solid_mass;
    Vec3F vec;
};
inline float mass(QuantityInfo * info){
    return info->air_mass + info->liquid_mass + info->solid_mass;
}

void add(QuantityInfo * dest, QuantityInfo * src);
void subtract(QuantityInfo * dest, QuantityInfo * src);
QuantityInfo random_init();

struct CubeChangeInfo{
    VectorAttraction force_shift;
    QuantityInfo quantity_shift;
};

CubeChangeInfo get_bordering_quantity_vel(QuantityInfo current, QuantityInfo other,Vec3F cube_direction);
RGBVal color(QuantityInfo info);
bool is_transparent(QuantityInfo info);
