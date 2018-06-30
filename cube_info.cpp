#include "cube_info.h"
#include "parameters.h"
#include <iostream>

using namespace  std;

float surface_area(float quantity){
    return pow(quantity,1.0/3.0);//TODO: check effect of turning this to 2/3, like surface area is supposed to be.
}

CubeChangeInfo get_bordering_quantity_vel(QuantityInfo current, QuantityInfo other_cube,Vec3F cube_direction){
    /*
    params: cube_direction: the unit vector pointing from this to the "bordering" cube
            bordering: a cube that is adjacent to the current one
    gloal:  if not a border, then gives some stuff to it depending on velocity and pressure
            if a border, then remove some of the velocity going in the direction of the border.

    concepts:
        pressure: a sort of molecular pressure similar to heat. Pushes out molocules on all sides.
                  The total pressure is evaluated using all substance types, because everything exhibits some pressure
                  Then the pressure's effect on each substance is related to its density, which is caused by the same molecular phenomena
                     as pressure, so the same constant is used to evaluate this as well.
        liquid attraction: liquids have an additional parameter, attraction, which defines a force pulling different cubes together.
                           I tried making internal attraction, but this seems to be better simulated by having a lower pressure constant than the other techniques I tried.
    */
    float liquid_attraction_force = attraction_force_coef * surface_area(current.liquid_mass) * surface_area(other_cube.liquid_mass);
    Vec3F liquid_attraction_vector = -liquid_attraction_force * cube_direction;

    float air_pressure = gass_pressure_coef*current.air_mass;
    float liquid_pressure = liquid_pressure_coef*current.liquid_mass;
    float solid_pressure = solid_pressure_coef*current.solid_mass;
    float total_pressure = air_pressure + liquid_pressure + solid_pressure;

    float air_pressure_speed = total_pressure * gass_pressure_coef;
    float liquid_pressure_speed = total_pressure * liquid_pressure_coef;
    float solid_pressure_speed = total_pressure * solid_pressure_coef;

    Vec3F total_air_motion = current.vec + cube_direction * air_pressure_speed;
    Vec3F total_liquid_motion = current.vec + cube_direction * liquid_pressure_speed;
    Vec3F total_solid_motion = current.vec + cube_direction * solid_pressure_speed;

    float basic_air_amt = max(0.0f,glm::dot(cube_direction,total_air_motion));
    float basic_liquid_amt = max(0.0f,glm::dot(cube_direction,total_liquid_motion));
    float basic_solid_amt = max(0.0f,glm::dot(cube_direction,total_solid_motion));

    float amt_air_given = std::min(basic_air_amt * current.air_mass * seconds_per_calc, current.air_mass/(0.01f+float(SIDES_ON_CUBE)));
    float amt_liquid_given = std::min(basic_liquid_amt * current.liquid_mass * seconds_per_calc,current.liquid_mass/(0.01f+float(SIDES_ON_CUBE)));
    float amt_solid_given = std::min(basic_solid_amt * current.solid_mass * seconds_per_calc,current.solid_mass/(0.01f+float(SIDES_ON_CUBE)));

    Vec3F final_vec = (total_air_motion * amt_air_given +
                       total_liquid_motion * amt_liquid_given +
                       total_solid_motion * amt_solid_given) /
                           (amt_air_given + amt_liquid_given + amt_solid_given + 0.000001f);

    QuantityInfo final_quantity = {amt_air_given,amt_liquid_given,amt_solid_given,final_vec};
    VectorAttraction attract_info{liquid_attraction_vector};
    return CubeChangeInfo{attract_info,final_quantity};
}
RGBVal color(QuantityInfo info){
    return RGBVal{1.0f-std::min(abs(info.air_mass)/2.0f,1.0f),1.0f-std::min(abs(info.solid_mass)/1000.0f,1.0f),1.0f-std::min(abs(info.liquid_mass)/800.0f,1.0f),1.0};
}
bool is_transparent(QuantityInfo info){
    return mass(&info) < 0.03;
}
void add(QuantityInfo * dest, QuantityInfo * src){
    dest->vec = (dest->vec * mass(dest) + src->vec * mass(src)) / (0.000001f + mass(dest) + mass(src));

    dest->air_mass += src->air_mass;
    dest->liquid_mass += src->liquid_mass;
    dest->solid_mass += src->solid_mass;
}
void subtract(QuantityInfo * dest, QuantityInfo * src){
    dest->vec = (dest->vec * mass(dest) - src->vec * mass(src)) / (0.000001f + mass(dest) - mass(src));

    dest->air_mass -= src->air_mass;
    dest->liquid_mass -= src->liquid_mass;
    dest->solid_mass -= src->solid_mass;
}
QuantityInfo random_init(){
    QuantityInfo data;
    data.air_mass = 0*rand() / float(RAND_MAX);
    data.liquid_mass = 50*rand() / float(RAND_MAX);
    data.solid_mass = 0*rand() / float(RAND_MAX);
    data.vec = zero_vec();
    return data;
}
