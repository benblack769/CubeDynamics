#pragma once
#include "quantity_info.h"

inline float square(float x){
    return x * x;
}
struct VectorAttraction{
    Vec3F force_vec;
};
inline float mass(QuantityInfo * info){
    return info->air_mass + info->liquid_mass + info->solid_mass;
}

void add(QuantityInfo * dest, QuantityInfo * src);
void subtract(QuantityInfo * dest, QuantityInfo * src);

struct CubeChangeInfo{
    VectorAttraction force_shift;
    QuantityInfo quantity_shift;
};

constexpr int SIDES_ON_CUBE = 6;

void update_coords(QuantityInfo * source_data, QuantityInfo * update_data, int base_x,int base_y,int base_z);
