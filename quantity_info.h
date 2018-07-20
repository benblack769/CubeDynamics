#pragma once
#include "cross_platform_vector.h"
#include "parameters.h"

struct QuantityInfo{
    float air_mass;
    float liquid_mass;
    float solid_mass;
    float __no_use_padding;
    Vec3F vec;
};

inline int int_pow3(int x){
    return x * x * x;
}

inline int data_size(){
    return int_pow3(size_cube+2);
}
inline int c_idx(CubeCoord c){
    return ((c.x+1)*(size_cube+1) + (c.y+1))*(size_cube+1) + (c.z+1);
}
inline QuantityInfo * get(QuantityInfo * data,CubeCoord c){
    return data + c_idx(c);
}

inline bool is_in_axis_bounds(int val){
    return val >= 0 && val < size_cube;
}
inline bool is_valid_cube(CubeCoord c){
    return
        is_in_axis_bounds(c.x) &&
        is_in_axis_bounds(c.y) &&
        is_in_axis_bounds(c.z);
}

inline float mass(const QuantityInfo * info){
    return info->air_mass + info->liquid_mass + info->solid_mass;
}
