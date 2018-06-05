#include "cube_info.h"
#include <iostream>
#include "glm/gtx/string_cast.hpp"

using namespace  std;

CubeInfo::CubeInfo(bool in_is_border){
    is_border = in_is_border;
    quantity = is_border ? 0 : rand() / float(RAND_MAX);
    velocity = glm::vec3(0,0,0);
}
glm::vec3 element_mul(glm::vec3 v1, glm::vec3 v2){
    return glm::vec3(v1.x*v2.x,v1.y*v2.y,v1.z*v2.z);
}
int counter = 0;
void CubeInfo::give_bordering_quantity_vel(glm::vec3 cube_direction, CubeInfo & bordering){
    /*
    params: cube_direction: the unit vector pointing from this to the "bordering" cube
            bordering: a cube that is adjacent to the current one
    gloal:  if not a border, then gives some stuff to it depending on velocity and pressure
            if a border, then remove some of the velocity going in the direction of the border.
    */
    if(this->is_border){
        //cout << "border" << endl;
        //cout << to_string(bordering.velocity) << endl;
        float mag_incident = glm::dot(bordering.velocity,cube_direction);
        if(mag_incident < 0){
            bordering.velocity -= cube_direction * mag_incident;
        }
        //bordering.velocity -= element_mul(bordering.velocity, (cube_direction * 0.8f));
        //cout << to_string(bordering.velocity) << endl;
        return;
    }
    constexpr float vel_quant_adj = 0.001;
    constexpr float quant_vel_adj = 1.0/vel_quant_adj;
    float pressure = this->quantity;

    float velocity_in_dir = glm::dot(cube_direction,velocity);
    float total_vel = velocity_in_dir + pressure;

    //std::cout << pressure << " ";
    //std::cout << velocity_in_dir << "           ";
    //std::cout << quantity << " ";
    if(total_vel <= 0){
        return;
    }

    float amt_given = std::min(total_vel * vel_quant_adj, this->quantity/float(SIDES_ON_CUBE));

    glm::vec3 pressure_motion = cube_direction * pressure;

    glm::vec3 total_motion = velocity + pressure_motion;

    bordering.velocity = ((bordering.velocity * bordering.quantity) + (total_motion * amt_given)) / (bordering.quantity + amt_given);
    bordering.quantity += amt_given;
    this->quantity = this->quantity - amt_given;
    if(!(this->quantity >= 0)){
        cout << this->quantity << endl;
        cout << bordering.quantity << endl;
        cout.flush();
        assert(false);
    }
    //assert( this->quantity >= 0);
    //assert(  bordering.quantity>= 0);
}
void CubeInfo::update_velocity_global(){
    //constant acceleration
    velocity += glm::vec3(0,-0.1,0);
}
RGBVal CubeInfo::color(){
    return RGBVal{1.0,1.0,1.0,std::min(abs(quantity)/2.0f,1.0f)};
}
bool CubeInfo::is_transparent(){
    return quantity < 0.1;
}
