#include "update.h"
#include "parameters.h"

using namespace  std;


CubeCoord rotate_coord(CubeCoord c){
    CubeCoord res = {c.y,c.z,c.x};
    return res;
}
#define visit_all_adjacent_(coord_var ,visit_code) \
    {for(int n = -1; n <= 1; n += 2){ \
        CubeCoord __input = {n,0,0}; \
        for(int d = 0; d < 3; d++){ \
            coord_var = __input;\
            {visit_code} \
            __input = rotate_coord(__input);\
        }\
    }}

struct VectorAttraction{
    Vec3F force_vec;
};


struct CubeChangeInfo{
    VectorAttraction force_shift;
    QuantityInfo quantity_shift;
};
Vec3F even_orientation(float mass){
    float cubrt = pow(mass,1.0f/3.0f);
    return build_vec(cubrt,cubrt,cubrt);
}
Vec3F orient_add(Vec3F o1, float m1, Vec3F o2, float m2,CubeCoord dir){
    Vec3F o3;
    float mt = m1 + m2;
    if(mt < 1e-10){
        return zero_vec();
    }
    float * v1 = (float *)(&o1);
    float * v2 = (float *)(&o2);
    float * vr = (float *)(&o3);

#define surf_calc(i0,i1,i2) \
        {float surf1 = v1[i1] * v1[i2]; \
        float surf2 = v2[i1] * v2[i2]; \
        float surft = (surf1 * m1 + surf2 * m2) / mt; \
        float i2t = (v1[i2] * m1 + v2[i2] * m2) / mt; \
        vr[i1] = surft / (i2t + 0.00001f); \
        vr[i2] = surft / (vr[i1] + 0.00001f); \
        vr[i0] = mt / (vr[i1] * vr[i2] + 0.00001f);}

    if(dir.x){
        surf_calc(0,1,2)
        /*
        float surf1 = v1[1] * v1[2];
        float surf2 = v2[1] * v2[2];
        float surft = (surf1 + surf2);
        vr[1] = surft / (v1[2] + v2[2] + 0.00001f);
        vr[2] = surft / (v1[1] + v2[1] + 0.00001f);
        vr[0] = mt / (vr[1] * vr[2] + 0.00001f);*/
    }
    else if(dir.y){
        surf_calc(1,2,0)
    }
    else{
        assert(dir.z);
        surf_calc(2,0,1)
    }
#undef surf_calc
    return o3;
}
//int main(){
//    cout << to_string(orient_calc(build_vec(20,3,3),3*3*20,build_vec(15,20,20),15*20*20,CubeCoord{0,0,1})) << endl;
//}
void add_quantity(QuantityInfo * dest, QuantityInfo * src, CubeCoord dir){
    dest->vec = mass(dest) + mass(src) < 1e-10f ?
                zero_vec() :
            (dest->vec * mass(dest) + src->vec * mass(src)) / (mass(dest) + mass(src));

    dest->solid_orientation = orient_add(
                src->solid_orientation,
                src->solid_mass,
                dest->solid_orientation,
                dest->solid_mass,
                dir
                );

    dest->air_mass += src->air_mass;
    dest->liquid_mass += src->liquid_mass;
    dest->solid_mass += src->solid_mass;
    assert(dest->air_mass >= 0);
}
void subtract_quantity(QuantityInfo * dest, QuantityInfo * src, CubeCoord dir){
    dest->vec = mass(dest) - mass(src) < 1e-10f ?
                zero_vec() :
            (dest->vec * mass(dest) - src->vec * mass(src)) / (mass(dest) - mass(src));

    float prop_removed = src->solid_mass / (0.0001f + dest->solid_mass);
    dest->solid_orientation -= dest->solid_orientation * prop_removed * abs(coord_to_vec(dir));

    dest->air_mass -= src->air_mass;
    dest->liquid_mass -= src->liquid_mass;
    dest->solid_mass -= src->solid_mass;
    assert(dest->air_mass >= 0);
}

Vec3F reflect_vector_along(Vec3F vector, Vec3F cube_dir){
    //reflects the vector in opposite direction of the cube_dir
    float mag_incident = dot_prod(vector,cube_dir);
    //assert(mag_incident >= 0);
    float dampen_value = 0.1f;
    Vec3F refl_vec = vector - cube_dir * mag_incident * (2.0f - dampen_value);
    return refl_vec;
}

float pow3(float x){
    return x * x * x;
}

Vec3F pow3v(Vec3F x){
    return x * x * x;
}

float surface_area(float quantity){
    return powf(quantity,1.0f/3.0f);//TODO: check effect of turning this to 2/3, like surface area is supposed to be.
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
    float liquid_attraction_force = liquid_attraction_force_coef * surface_area(current.liquid_mass) * surface_area(other_cube.liquid_mass);
    float solid_attraction_force = solid_attraction_force_coef * surface_area(current.solid_mass) * surface_area(other_cube.solid_mass);
    float total_attr_force = liquid_attraction_force + solid_attraction_force;
    Vec3F liquid_attraction_vector = -total_attr_force * cube_direction;

    float air_pressure = gass_pressure_coef*current.air_mass;
    float liquid_pressure = liquid_pressure_coef*current.liquid_mass;
    float solid_pressure = solid_pressure_coef*current.solid_mass;
    float total_pressure = air_pressure + liquid_pressure + solid_pressure;

    float air_pressure_speed = total_pressure * gass_pressure_coef;
    float liquid_pressure_speed = total_pressure * liquid_pressure_coef;
    float solid_pressure_speed = total_pressure * solid_pressure_coef;


    Vec3F total_air_motion = current.vec + cube_direction * air_pressure_speed;
    Vec3F total_liquid_motion = current.vec + cube_direction * liquid_pressure_speed;
    Vec3F total_solid_motion_f = current.vec + cube_direction * solid_pressure_speed;

    Vec3F total_solid_motion = current.vec + cube_direction * pow3v(current.solid_orientation) * directional_solid_pressure;

    float basic_air_amt = max(0.0f,dot_prod(cube_direction,total_air_motion));
    float basic_liquid_amt = max(0.0f,dot_prod(cube_direction,total_liquid_motion));
    float basic_solid_amt = max(0.0f,dot_prod(cube_direction,total_solid_motion));

    float amt_air_given = min(basic_air_amt * current.air_mass * seconds_per_calc, current.air_mass/(0.01f+SIDES_ON_CUBE));
    float amt_liquid_given = min(basic_liquid_amt * current.liquid_mass * seconds_per_calc,current.liquid_mass/(0.01f+SIDES_ON_CUBE));
    float amt_solid_given = min(basic_solid_amt * current.solid_mass * seconds_per_calc,current.solid_mass/(0.01f+SIDES_ON_CUBE));

    /*
    QuantityInfo air_transfer_quantity = {amt_air_given,0,0, total_air_motion};
    QuantityInfo liquid_transfer_quantity = {0,amt_liquid_given,0, total_liquid_motion};
    QuantityInfo solid_transfer_quantity = {0,0,amt_solid_given, total_solid_motion};

    QuantityInfo final_quantity = air_transfer_quantity;
    add(&final_quantity,&liquid_transfer_quantity);
    add(&final_quantity,&solid_transfer_quantity);
    */
    Vec3F final_vec = (total_air_motion * amt_air_given +
                       total_liquid_motion * amt_liquid_given +
                       total_solid_motion * amt_solid_given) /
                           (amt_air_given + amt_liquid_given + amt_solid_given + 0.000001f);

    float prop_solid_given = amt_solid_given / (0.0001f + current.solid_mass);
    Vec3F solid_orient = current.solid_orientation * (1.0f-abs(cube_direction)*(1.0f-prop_solid_given));
    //cout << to_string( (1.0f-abs(cube_direction)*(1.0f-prop_solid_given))) << endl;
    QuantityInfo final_quantity = {amt_air_given,amt_liquid_given,amt_solid_given,0,final_vec,solid_orient};

    VectorAttraction attract_info = {liquid_attraction_vector};
    CubeChangeInfo change_info = {attract_info,final_quantity};
    return change_info;
}
float mass_conv(float m){
    return powf(m,1.0f/3.0f);
}
int count = 0;
void update_coord_quantity(QuantityInfo * source_data, QuantityInfo * update_data, CubeCoord base_coord){
    Vec3F global_gravity_vector = build_vec(0,-gravity_constant * seconds_per_calc,0);

    const QuantityInfo base_orig_quanity = *get(source_data,base_coord);
    QuantityInfo total_quanity = base_orig_quanity;

    CubeCoord offset;

    Vec3F total_accel_val = zero_vec();

    visit_all_adjacent_(offset,{
        Vec3F cube_dir = coord_to_vec(offset);
        CubeCoord adj_coord = add_coord(base_coord,offset);
        QuantityInfo adj_orig_quanity = *get(source_data,adj_coord);
        CubeChangeInfo change_info = get_bordering_quantity_vel(base_orig_quanity,adj_orig_quanity,cube_dir);

        QuantityInfo add_vec = change_info.quantity_shift;

        if(is_valid_cube(adj_coord)){
            CubeChangeInfo adj_change_info = get_bordering_quantity_vel(adj_orig_quanity,base_orig_quanity,-cube_dir);

            total_accel_val += (seconds_per_calc / (0.0001f+mass(&base_orig_quanity))) *
                    ( - change_info.force_shift.force_vec + adj_change_info.force_shift.force_vec);

            add_quantity(&total_quanity,&adj_change_info.quantity_shift,offset);
            subtract_quantity(&total_quanity,&add_vec,offset);
                                count++;
        }
        else{
            //assert(change_info.force_shift.force_vec == Vec3F(0,0,0));
            //is border cube
            QuantityInfo reflected_vector = add_vec;
            reflected_vector.vec = reflect_vector_along(add_vec.vec,cube_dir);

            add_quantity(&total_quanity,&reflected_vector,offset);
            subtract_quantity(&total_quanity,&add_vec,offset);
        }
    });

    total_quanity.vec += total_accel_val + global_gravity_vector;
    *get(update_data,base_coord) = total_quanity;
}
