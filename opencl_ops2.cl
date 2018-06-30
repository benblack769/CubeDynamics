#define seconds_per_calc 0.001
#define attraction_force_coef = 10000.0
#define liquid_pressure_coef = 0.8
#define solid_pressure_coef = 0.5
#define gass_pressure_coef = 10.0
#define gravity_constant = 1000

struct CubeCoord_{
    int x;
    int y;
    int z;
};
#define CubeCoord struct CubeCoord_
#define Vec3F float4

CubeCoord add(CubeCoord a,CubeCoord b){
    CubeCoord coord = {a.x + b.x, a.y + b.y, a.z + b.z};
    return coord;
}

Vec3F zero_vec(){
    Vec3F zero = {0,0,0,0};
    return zero;
}
Vec3F build_vec(float x, float y, float z){
    Vec3F res = {x,y,z,0};
    return res;
}
Vec3F coord_to_vec(CubeCoord c){
    return build_vec(c.x,c.y,c.z);
}
struct QuantityInfo_{
    float air_mass;
    float liquid_mass;
    float solid_mass;
    Vec3F vec;
};
#define QuantityInfo struct QuantityInfo_
#define size_cube 30

QuantityInfo * get(QuantityInfo * data,CubeCoord c);

bool is_in_axis_bounds(int val){
    return val >= 0 && val < size_cube;
}
bool is_valid_cube(CubeCoord c){
    return
        is_in_axis_bounds(c.x) &&
        is_in_axis_bounds(c.y) &&
        is_in_axis_bounds(c.z);
}

float mass(QuantityInfo * info){
    return info->air_mass + info->liquid_mass + info->solid_mass;
}
#define visit_all_adjacent_(iter_var,visit_code) \
    {for(int n = -1; n <= 1; n += 2){ \
        iter_var = CubeCoord{n,0,0}; \
        {visit_code}; \
        iter_var = CubeCoord{0,n,0}; \
        {visit_code}; \
        iter_var = CubeCoord{0,0,n}; \
        {visit_code}; \
    }}
float square(float x){
    return x * x;
}
struct VectorAttraction{
    Vec3F force_vec;
};

struct CubeChangeInfo{
    VectorAttraction force_shift;
    QuantityInfo quantity_shift;
};

#define SIDES_ON_CUBE 6

void add(QuantityInfo * dest, QuantityInfo * src){
    dest->vec = mass(dest) + mass(src) < 1e-10f ?
                zero_vec() :
            (dest->vec * mass(dest) + src->vec * mass(src)) / (mass(dest) + mass(src));

    dest->air_mass += src->air_mass;
    dest->liquid_mass += src->liquid_mass;
    dest->solid_mass += src->solid_mass;
    assert(dest->air_mass >= 0);
}
void subtract(QuantityInfo * dest, QuantityInfo * src){
    dest->vec = mass(dest) - mass(src) < 1e-10f ?
                zero_vec() :
            (dest->vec * mass(dest) - src->vec * mass(src)) / (mass(dest) - mass(src));

    dest->air_mass -= src->air_mass;
    dest->liquid_mass -= src->liquid_mass;
    dest->solid_mass -= src->solid_mass;
    assert(dest->air_mass >= 0);
}

QuantityInfo * get(QuantityInfo * data,CubeCoord c){
    //assert(is_valid_index(c));
    return data + (((c.x+1)*(size_cube+1) + (c.y+1))*(size_cube+1) + (c.z+1));
}
float dot_prod(Vec3F v1,Vec3F v2){
    Vec3F prod = v1 * v2;
    float * arr = (float*)(&prod);
    float sum = 0;
    for(int i = 0; i < 4; i++){
        sum += arr[i];
    }
    return sum;
}
Vec3F reflect_vector_along(Vec3F vector, Vec3F cube_dir){
    //reflects the vector in opposite direction of the cube_dir
    float mag_incident = dot_prod(vector,cube_dir);
    //assert(mag_incident >= 0);
    float dampen_value = 0.1f;
    Vec3F refl_vec = vector - cube_dir * mag_incident * (2.0f - dampen_value);
    return refl_vec;
}

float surface_area(float quantity){
    return pow(quantity,1.0/3.0);//TODO: check effect of turning this to 2/3, like surface area is supposed to be.
}

struct CubeChangeInfo get_bordering_quantity_vel(QuantityInfo current, QuantityInfo other_cube,Vec3F cube_direction){
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

    float basic_air_amt = std::max(0.0f,dot_prod(cube_direction,total_air_motion));
    float basic_liquid_amt = std::max(0.0f,dot_prod(cube_direction,total_liquid_motion));
    float basic_solid_amt = std::max(0.0f,dot_prod(cube_direction,total_solid_motion));

    float amt_air_given = std::min(basic_air_amt * current.air_mass * seconds_per_calc, current.air_mass/(0.01f+float(SIDES_ON_CUBE)));
    float amt_liquid_given = std::min(basic_liquid_amt * current.liquid_mass * seconds_per_calc,current.liquid_mass/(0.01f+float(SIDES_ON_CUBE)));
    float amt_solid_given = std::min(basic_solid_amt * current.solid_mass * seconds_per_calc,current.solid_mass/(0.01f+float(SIDES_ON_CUBE)));

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

    QuantityInfo final_quantity = {amt_air_given,amt_liquid_given,amt_solid_given,final_vec};

    struct VectorAttraction attract_info = {liquid_attraction_vector};
    struct CubeChangeInfo res_info = {attract_info,final_quantity};
    return res_info;
}
kernel void update_coords(global QuantityInfo * source_data, global QuantityInfo * update_data){
    int base_x = get_global_id(0);
    int base_y = get_global_id(1);
    int base_z = get_global_id(2);
    CubeCoord base_coord = {base_x,base_y,base_z};
    Vec3F global_gravity_vector = build_vec(0,-gravity_constant * seconds_per_calc,0);

    QuantityInfo base_orig_quanity = *get(source_data,base_coord);
    QuantityInfo total_quanity = base_orig_quanity;

    Vec3F total_accel_val = zero_vec();
    CubeCoord offset;
    visit_all_adjacent_(offset,{
        Vec3F cube_dir = coord_to_vec(offset);
        CubeCoord adj_coord = add(base_coord,offset);
        QuantityInfo adj_orig_quanity = *get(source_data,adj_coord);
        struct CubeChangeInfo change_info = get_bordering_quantity_vel(base_orig_quanity,adj_orig_quanity,cube_dir);

        QuantityInfo add_vec = change_info.quantity_shift;

        if(is_valid_cube(adj_coord)){
            struct CubeChangeInfo adj_change_info = get_bordering_quantity_vel(adj_orig_quanity,base_orig_quanity,-cube_dir);

            total_accel_val += (seconds_per_calc / (0.0001f+mass(&base_orig_quanity))) *
                    ( - change_info.force_shift.force_vec + adj_change_info.force_shift.force_vec);

            add(&total_quanity,&adj_change_info.quantity_shift);
            subtract(&total_quanity,&add_vec);
        }
        else{
            //assert(change_info.force_shift.force_vec == Vec3F(0,0,0));
            //is border cube
            QuantityInfo reflected_vector = add_vec;
            reflected_vector.vec = reflect_vector_along(add_vec.vec,cube_dir);

            add(&total_quanity,&reflected_vector);
            subtract(&total_quanity,&add_vec);
        }
    });
    total_quanity.vec += total_accel_val + global_gravity_vector;
    *get(update_data,base_coord) = total_quanity;
}