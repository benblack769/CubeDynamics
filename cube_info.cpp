#include "cube_info.h"
#include <iostream>
#include "glm/gtx/string_cast.hpp"

using namespace  std;

CubeInfo::CubeInfo(bool in_is_border){
    is_border = in_is_border;
    quantity = is_border ? 0 : rand() / float(RAND_MAX);
    velocity = glm::vec3(0,0,0);
}
//int counter = 0;
MassVec CubeInfo::get_bordering_quantity_vel(glm::vec3 cube_direction){
    /*
    params: cube_direction: the unit vector pointing from this to the "bordering" cube
            bordering: a cube that is adjacent to the current one
    gloal:  if not a border, then gives some stuff to it depending on velocity and pressure
            if a border, then remove some of the velocity going in the direction of the border.
    */
    assert(!this->is_border);
    constexpr float vel_quant_adj = 0.0002;
    float pressure = this->quantity*30;

    glm::vec3 pressure_motion = cube_direction * pressure;
    glm::vec3 total_motion = velocity + pressure_motion;

    float total_vel = glm::dot(cube_direction,total_motion);

    if(total_vel <= 0){
        return MassVec{0,glm::vec3(0,0,0)};
    }
    if(total_vel > 1000){
        cout << "g1000\n";
    }

    float amt_given = std::min(total_vel * this->quantity * vel_quant_adj, this->quantity/float(SIDES_ON_CUBE));

    return MassVec{amt_given, total_motion};
}
void CubeInfo::update_velocity_global(){
    //constant acceleration
    velocity += glm::vec3(0,-0.1,0);
}
RGBVal CubeInfo::color(){
    return RGBVal{std::min(abs(quantity)/10.0f,1.0f),1.0,1.0,std::min(abs(quantity)/2.0f,1.0f)};
}
bool CubeInfo::is_transparent(){
    return quantity < 0.03;
}
void CubeInfo::debug_print(){
    cout << this->quantity << endl;
    cout << to_string(this->velocity) << endl;
}
