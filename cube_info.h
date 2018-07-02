#pragma once
#include "cube_color.h"
#include "cube_coords.h"
#include <cstdlib>
#include <vector>
#include <iostream>
#include <glm/common.hpp>
#include <glm/geometric.hpp>
#include <algorithm>
#include "glm/gtx/string_cast.hpp"

constexpr int SIDES_ON_CUBE = 6;

using Vec3F = glm::vec3;
struct VectorAttraction{
    Vec3F force_vec;
};
struct QuantityInfo{
    float air_mass;
    float liquid_mass;
    float solid_mass;
    Vec3F vec;
    float mass(){
        return air_mass + liquid_mass + solid_mass;
    }
    void add(QuantityInfo addval){
        this->vec = abs((this->mass() + addval.mass())) < 10e-15f ?
                        Vec3F(0,0,0) :
                        (this->vec * this->mass() + addval.vec * addval.mass()) / (this->mass() + addval.mass());

        this->air_mass += addval.air_mass;
        this->liquid_mass += addval.liquid_mass;
        this->solid_mass += addval.solid_mass;
    }
    void subtract(QuantityInfo subval){
        QuantityInfo add_neg_val = subval;
        add_neg_val.air_mass = - add_neg_val.air_mass;
        add_neg_val.liquid_mass = - add_neg_val.liquid_mass;
        add_neg_val.solid_mass = - add_neg_val.solid_mass;
        this->add(add_neg_val);
    }
    void debug_print(){
        using namespace  std;
        cout << this->air_mass << endl;
        cout << this->liquid_mass << endl;
        cout << this->solid_mass << endl;
        cout << to_string(this->vec) << endl;
    }
};
inline float square(float x){
    return x * x;
}
struct CubeChangeInfo{
    VectorAttraction force_shift;
    QuantityInfo quantity_shift;
};
class CubeInfo{
public:
    QuantityInfo data;
    bool is_border;
public:
    CubeInfo(bool in_is_border=false);
    CubeChangeInfo get_bordering_quantity_vel(const CubeInfo & other_cube,Vec3F cube_direction,float solid_interal_bond)const;
    RGBVal color();
    bool is_transparent();
    void debug_print(){
        if(is_border){
            std::cout << "bordernode";
        }
        else{
            data.debug_print();
        }
    }
};
