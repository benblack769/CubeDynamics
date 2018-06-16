#include "cube_info.h"
#include <iostream>

using namespace  std;

CubeInfo::CubeInfo(bool in_is_border){
    is_border = in_is_border;
    if(!is_border){
        data.air_mass = 0.0f;//rand() / float(RAND_MAX);
        data.liquid_mass = 20*rand() / float(RAND_MAX);
    }
    else{
        data.air_mass = 0;
        data.liquid_mass = 0;
    }
    data.vec = glm::vec3(0,0,0);
}
float surface_area(float quantity){
    return pow(quantity,1.0/3.0);
}
float square(float x){
    return x * x;
}
int counter = 0;
CubeChangeInfo CubeInfo::get_bordering_quantity_vel(const CubeInfo & other_cube,glm::vec3 cube_direction){
    /*
    params: cube_direction: the unit vector pointing from this to the "bordering" cube
            bordering: a cube that is adjacent to the current one
    gloal:  if not a border, then gives some stuff to it depending on velocity and pressure
            if a border, then remove some of the velocity going in the direction of the border.
    */
    assert(!this->is_border);
    constexpr float vel_quant_adj = 0.001;
    constexpr float attraction_force_coef = 10.0;
    constexpr float liquid_pressure_coef = 0.5;
    constexpr float gass_pressure_coef = 30.0;

    float liquid_attraction_force = attraction_force_coef * surface_area(this->data.liquid_mass) * surface_area(other_cube.data.liquid_mass);
    float attraction_speed = liquid_attraction_force / max((this->data.liquid_mass),1e-8f);

    float internal_attraction_force = 0;//attraction_force_coef * (surface_area(this->data.liquid_mass));
    float liquid_raw_pressure = liquid_pressure_coef*data.liquid_mass;
    float liquid_net_pressure = max(0.0f,liquid_raw_pressure-internal_attraction_force);
    float liquid_force = liquid_net_pressure;// + attraction_speed;

    float air_pressure = gass_pressure_coef*data.air_mass;

    glm::vec3 total_air_motion = data.vec + cube_direction * air_pressure;
    glm::vec3 total_liquid_motion = data.vec + cube_direction * liquid_force;

    float basic_air_amt = max(0.0f,glm::dot(cube_direction,total_air_motion));
    float basic_liquid_amt = max(0.0f,glm::dot(cube_direction,total_liquid_motion));

    float amt_air_given = std::min(basic_air_amt * this->data.air_mass * vel_quant_adj, max(0.0f,-0.0000001f+this->data.air_mass/float(SIDES_ON_CUBE)));
    float amt_liquid_given = std::min(basic_liquid_amt * this->data.liquid_mass * vel_quant_adj,max(0.0f,-0.0000001f+this->data.liquid_mass/float(SIDES_ON_CUBE)));

    QuantityInfo air_transfer_quantity = {amt_air_given,0, total_air_motion};
    QuantityInfo liquid_transfer_quantity = {0,amt_liquid_given, total_liquid_motion};

    QuantityInfo final_quantity = air_transfer_quantity;
    final_quantity.add(liquid_transfer_quantity);
    if(false && this->data.liquid_mass > 5 && counter % 100000 == 0 ){
        cout << "start" << endl;
        cout << attraction_speed << endl;
        cout << liquid_raw_pressure << endl;
        cout << liquid_attraction_force << endl;
        cout << to_string(basic_liquid_amt) << endl;
        cout << to_string(amt_liquid_given) << endl;
        cout << to_string(basic_liquid_amt * this->data.liquid_mass * vel_quant_adj) << endl;
        cout << to_string(max(0.0f,-0.0000001f+this->data.liquid_mass/float(SIDES_ON_CUBE))) << endl;
        cout << to_string(amt_liquid_given) << endl;
        cout << to_string(total_liquid_motion) << endl;
        cout << liquid_transfer_quantity.liquid_mass << endl;
        cout << to_string(data.vec ) << endl;
        cout << to_string(liquid_force) << endl;
        liquid_transfer_quantity.debug_print();
        final_quantity.debug_print();
    }
    VectorAttraction attract_info{liquid_attraction_force};
    counter++;
    return CubeChangeInfo{attract_info,final_quantity};
}
void CubeInfo::update_velocity_global(){
    //constant acceleration
    this->data.vec += glm::vec3(0,-1,0);
}
RGBVal CubeInfo::color(){
    return RGBVal{1.0f-std::min(abs(data.air_mass)/2.0f,1.0f),1.0,1.0f-std::min(abs(data.liquid_mass)/20.0f,1.0f),1.0};//,std::min(abs(data.air_mass)/2.0f,1.0f)};
}
bool CubeInfo::is_transparent(){
    return data.mass() < 0.03;
}
