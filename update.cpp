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

float pow3(float x){
    return x * x * x;
}

Vec3F pow3v(Vec3F x){
    return x * x * x;
}

float surface_area(float quantity){
    return powf(quantity,1.0f/3.0f);//TODO: check effect of turning this to 2/3, like surface area is supposed to be.
}
float solid_attr_force(LargeVec solid_splits1, LargeVec solid_splits2){
    return solid_attraction_force_coef * sum_lv(mul_e_lv(sqrt_lv(solid_splits1),sqrt_lv(solid_splits2)));
}
Vec3F get_force_transfer(QuantityInfo current, QuantityInfo other_cube,Vec3F cube_direction){
    float liquid_attraction_force = liquid_attraction_force_coef * surface_area(current.liquid_mass) * surface_area(other_cube.liquid_mass);
    float solid_attraction_force = solid_attr_force(current.solid_splits,other_cube.solid_splits);
    //float solid_attraction_force = solid_attraction_force_coef * surface_area(current.solid_mass) * surface_area(other_cube.solid_mass);
    float total_attr_force = liquid_attraction_force + solid_attraction_force;
    Vec3F attraction_vector = -total_attr_force * cube_direction;
    return attraction_vector;
}

LargeVec get_solid_distribution(LargeVec orig_dist, float solid_transfer_amt, int prefer_transfer_coord){
    assert(prefer_transfer_coord >= 0 && prefer_transfer_coord < LARGE_VEC_SIZE);
    float pref_give_val = min(solid_transfer_amt,orig_dist.data[prefer_transfer_coord]);
    float leftover_val = solid_transfer_amt - pref_give_val;
    LargeVec leftover_vec = orig_dist;
    leftover_vec.data[prefer_transfer_coord] = 0;
    float leftover_perc = leftover_val / (0.00001f + sum_lv(leftover_vec));
    LargeVec give_vec = add_bv(mul_bv(leftover_vec,leftover_perc), create_idx(prefer_transfer_coord,pref_give_val));
    return give_vec;
}
QuantityInfo get_bordering_quantity_vel(QuantityInfo current, QuantityInfo other_cube,Vec3F cube_direction){
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
    return final_quantity;
}
Vec3F reflect_vector_along(Vec3F vector, Vec3F cube_dir){
    //reflects the vector in opposite direction of the cube_dir
    float mag_incident = dot_prod(vector,cube_dir);
    //assert(mag_incident >= 0);
    float dampen_value = 0.0f;
    Vec3F refl_vec = vector - cube_dir * mag_incident * (2.0f - dampen_value);
    return refl_vec;
}
bool should_reflect(float cur_solid_mass, float next_solid_mass, float final_solid_mass){
    float max_mass = max(cur_solid_mass,final_solid_mass);
    float min_mass = min(cur_solid_mass,final_solid_mass);
    return max_mass > min_reflect_mass &&
           max_mass * gap_ratio_reflect > next_solid_mass &&
           max_mass * reflect_max_ratio < min_mass; //doesn't reflect unless masses are both relatively large
}
Vec3F reflect_force(float amt_reflected,Vec3F change_vel){
    return  (1.0f/seconds_per_calc) * amt_reflected * change_vel;
}
CubeCoord mul_offset(CubeCoord coord, int mul_by){
    CubeCoord new_coord = {coord.x * mul_by, coord.y * mul_by, coord.z * mul_by};
    return new_coord;
}
int rand_vec_idx(int timestep, CubeCoord cur_coord, int offsetidx){
    int val = (((timestep * 102647
            + cur_coord.x) * 87811
            + cur_coord.y * 96329)
            + cur_coord.z * 82613) + offsetidx;
    return val % LARGE_VEC_SIZE;
}
int offset_idx(CubeCoord offset){
    return 3 + 3*offset.x + 2*offset.y + offset.z;
}
LargeVec ballance_lv_splits(float mass, LargeVec cur_vals){
    LargeVec res = zero_floor(cur_vals);
    res = add_sca(res,0.00001f);
    float tot_vals = sum_lv(res) + 0.00001f;
    imul_bv(&res,mass/tot_vals);
    return res;
}
int count = 0;
void update_coord_quantity(QuantityInfo * source_data, QuantityInfo * update_data, CubeCoord base_coord, int timestep){
    Vec3F global_gravity_vector = build_vec(0,-gravity_constant * seconds_per_calc,0);

    const QuantityInfo base_orig_quanity = *get(source_data,base_coord);
    QuantityInfo total_quanity = base_orig_quanity;

    Vec3F total_force_val = zero_vec();
    LargeVec res_solid_dist = base_orig_quanity.solid_splits;

    CubeCoord offset;
    visit_all_adjacent_(offset,{
        Vec3F cube_dir = coord_to_vec(offset);
        CubeCoord adj_coord = add_coord(base_coord,offset);
        const QuantityInfo adj_orig_quanity = *get(source_data,adj_coord);
        QuantityInfo add_vec = get_bordering_quantity_vel(base_orig_quanity,adj_orig_quanity,cube_dir);
        Vec3F force_orig_vec = get_force_transfer(base_orig_quanity,adj_orig_quanity,cube_dir);

        if(is_valid_cube(adj_coord)){
            QuantityInfo adj_quant_shift = get_bordering_quantity_vel(adj_orig_quanity,base_orig_quanity,-cube_dir);
            Vec3F force_adj_vec = get_force_transfer(adj_orig_quanity,base_orig_quanity,-cube_dir);

            total_force_val += ( - force_orig_vec + force_adj_vec);

            //handles reflections of solids
            CubeCoord twice_offset = mul_offset(offset,2);
            CubeCoord two_away_coord = add_coord(base_coord,twice_offset);
            if(is_valid_cube(two_away_coord)){
                QuantityInfo one_away_quanity = adj_orig_quanity;
                QuantityInfo two_away_quanity = *get(source_data,two_away_coord);
                if(should_reflect(base_orig_quanity.solid_mass,one_away_quanity.solid_mass,two_away_quanity.solid_mass)){

                    //reflect other mass putting force on self
                    QuantityInfo reflect_amount = get_bordering_quantity_vel(two_away_quanity,one_away_quanity,cube_dir);
                    Vec3F change_vector = reflect_amount.vec - reflect_vector_along(reflect_amount.vec,cube_dir);
                    change_vector *= 0.5f;
                    total_force_val -= reflect_force(reflect_amount.solid_mass,change_vector);

                    //reflect current mass
                    QuantityInfo reflected_vector = add_vec;
                    reflected_vector.vec = reflect_vector_along(add_vec.vec,cube_dir);
                    add_quantity(&total_quanity,&reflected_vector,offset);
                }
            }

            CubeCoord neg_offset = mul_offset(offset,-1);
            CubeCoord neg_coord = add_coord(base_coord,neg_offset);
            QuantityInfo neg_quanity = *get(source_data,neg_coord);
            if(!is_valid_cube(neg_coord) || !should_reflect(neg_quanity.solid_mass,base_orig_quanity.solid_mass,adj_orig_quanity.solid_mass)){
                add_quantity(&total_quanity,&adj_quant_shift,offset);
                LargeVec receive_lv = get_solid_distribution(adj_orig_quanity.solid_splits,adj_quant_shift.solid_mass,rand_vec_idx(timestep,adj_coord,offset_idx(neg_offset)));
                iadd_bv(&res_solid_dist,receive_lv);
            }

            subtract_quantity(&total_quanity,&add_vec,offset);
            LargeVec receive_lv = get_solid_distribution(base_orig_quanity.solid_splits,add_vec.solid_mass,rand_vec_idx(timestep,base_coord,offset_idx(offset)));
            isub_bv(&res_solid_dist,receive_lv);
                                if(count % 10000 == 0){
                                    print_lv(res_solid_dist);
                                    print_lv(receive_lv);
                                    cout << total_quanity.solid_mass << endl;
                                }
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

    Vec3F total_accel_val = total_force_val * (seconds_per_calc / (0.0001f+mass(&base_orig_quanity)));
    total_quanity.vec += total_accel_val + global_gravity_vector;
    total_quanity.solid_splits = ballance_lv_splits(total_quanity.solid_mass,res_solid_dist);
    *get(update_data,base_coord) = total_quanity;
}
