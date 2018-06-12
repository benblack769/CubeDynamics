#include "cube_info.h"
#include <iostream>

using namespace  std;

CubeInfo::CubeInfo(bool in_is_border){
    is_border = in_is_border;
    data.air_mass = is_border ? 0 : rand() / float(RAND_MAX);
    data.liquid_mass = is_border ? 0 : 10*rand() / float(RAND_MAX);
    data.vec = glm::vec3(0,0,0);
}
//int counter = 0;
QuantityInfo CubeInfo::get_bordering_quantity_vel(glm::vec3 cube_direction){
    /*
    params: cube_direction: the unit vector pointing from this to the "bordering" cube
            bordering: a cube that is adjacent to the current one
    gloal:  if not a border, then gives some stuff to it depending on velocity and pressure
            if a border, then remove some of the velocity going in the direction of the border.
    */
    assert(!this->is_border);
    constexpr float vel_quant_adj = 0.0002;
    float max_liquid_density = 1.0f/30.0f;
    float liquid_volume = max_liquid_density * data.liquid_mass;
    float liquid_pressure = 0.0f;//liquid_volume < 1.0f ? 0 : (data.liquid_mass - max_liquid_volume)*30;
    //float vol_taken_liquid = min(0.95f,(max_liquid_volume - data.liquid_mass)/max_liquid_volume);
    float air_pressure = data.air_mass*30;
    //float total_pressure = air_pressure + liquid_pressure;

    glm::vec3 total_air_motion = data.vec + cube_direction * air_pressure;
    glm::vec3 total_liquid_motion = data.vec + cube_direction * liquid_pressure;

    float basic_air_amt = max(0.0f,glm::dot(cube_direction,total_air_motion));
    float basic_liquid_amt = max(0.0f,glm::dot(cube_direction,total_liquid_motion));

    float amt_air_given = std::min(basic_air_amt * this->data.air_mass * vel_quant_adj, max(0.0f,-0.0000001f+this->data.air_mass/float(SIDES_ON_CUBE)));
    float amt_liquid_given = std::min(basic_liquid_amt * this->data.liquid_mass * vel_quant_adj,max(0.0f,-0.0000001f+this->data.liquid_mass/float(SIDES_ON_CUBE)));

    QuantityInfo air_transfer_quantity = {amt_air_given,0, total_air_motion};
    QuantityInfo liquid_transfer_quantity = {0,amt_liquid_given, total_liquid_motion};

    QuantityInfo final_quantity = air_transfer_quantity;
    final_quantity.add(liquid_transfer_quantity);
    //cout << "start" << endl;
    //air_transfer_quantity.debug_print();
    //final_quantity.debug_print();
    //debug_print();
    return final_quantity;
}
void CubeInfo::update_velocity_global(){
    //constant acceleration
    this->data.vec += glm::vec3(0,-0.1,0);
}
RGBVal CubeInfo::color(){
    return RGBVal{1.0f-std::min(abs(data.air_mass)/2.0f,1.0f),1.0,1.0f-std::min(abs(data.liquid_mass)/20.0f,1.0f),1.0};//,std::min(abs(data.air_mass)/2.0f,1.0f)};
}
bool CubeInfo::is_transparent(){
    return data.mass() < 0.03;
}
