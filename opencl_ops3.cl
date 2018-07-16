
#define powf pow
struct CubeCoord_{
    int x;
    int y;
    int z;
};
#define CubeCoord struct CubeCoord_
#define Vec3F float4

inline CubeCoord add_coord(CubeCoord a,CubeCoord b){
    CubeCoord coord = {a.x + b.x, a.y + b.y, a.z + b.z};
    return coord;
}
inline CubeCoord sub_coord(CubeCoord a,CubeCoord b){
    CubeCoord coord = {a.x - b.x, a.y - b.y, a.z - b.z};
    return coord;
}
inline float square(float x){
    return x * x;
}
inline int sqr(int x){
    return x * x;
}
inline int sqr_len(CubeCoord c){
    return sqr(c.x) + sqr(c.y) + sqr(c.z);
}


Vec3F zero_vec(){
    Vec3F zero = {0,0,0,0};
    return zero;
}
Vec3F build_vec(float x, float y, float z){
    Vec3F res = {x,y,z,0};
    return res;
}
#define dot_prod dot
Vec3F coord_to_vec(CubeCoord c){
    return build_vec(c.x,c.y,c.z);
}
struct QuantityInfo_{
    float air_mass;
    float liquid_mass;
    float solid_mass;
    float __no_use_padding;
    Vec3F vec;
};
#define QuantityInfo struct QuantityInfo_

int int_pow3(int x){
    return x * x * x;
}

int data_size(){
    return int_pow3(size_cube+2);
}
int c_idx(CubeCoord c){
    return ((c.x+1)*(size_cube+1) + (c.y+1))*(size_cube+1) + (c.z+1);
}
global QuantityInfo * get(global QuantityInfo * data,CubeCoord c){
    return data + c_idx(c);
}

bool is_in_axis_bounds(int val){
    return val >= 0 && val < size_cube;
}
bool is_valid_cube(CubeCoord c){
    return
        is_in_axis_bounds(c.x) &&
        is_in_axis_bounds(c.y) &&
        is_in_axis_bounds(c.z);
}

float mass(const QuantityInfo * info){
    return info->air_mass + info->liquid_mass + info->solid_mass;
}

int bond_coord_offset(CubeCoord o){
    return ((o.x+1) * 3 + (o.y+1)) * 3 + (o.z + 1);
}
global float * get_coord_bond_block_start(global float * bond_data, CubeCoord c){
    return bond_data + c_idx(c) * NUM_BONDS_PER_CUBE;
}
global float * get_bond(global float * bond_data,CubeCoord coord, CubeCoord dir){
    int offset = bond_coord_offset(dir);
    return get_coord_bond_block_start(bond_data,coord) + offset;
}

CubeCoord rotate_coord(CubeCoord c){
    CubeCoord res = {c.y,c.z,c.x};
    return res;
}
bool is_valid_bond(CubeCoord offset){
    return sqr_len(offset) <= 3;
}
#define visit_all_adjacent_(coord_var, adj_index_var ,visit_code) \
    {adj_index_var = 0; \
    for(int n = -1; n <= 1; n += 2){ \
        CubeCoord __input = {n,0,0}; \
        for(int d = 0; d < 3; d++){ \
            coord_var = __input;\
            {visit_code} \
            adj_index_var += 1; \
            __input = rotate_coord(__input);\
        }\
    }}
#define visit_all_adjacent_plus_center(coord_var, adj_index_var ,visit_code) \
    {adj_index_var = 0; \
    for(int n = -1; n <= 1; n += 2){ \
        CubeCoord __input = {n,0,0}; \
        for(int d = 0; d < 3; d++){ \
            coord_var = __input;\
            {visit_code} \
            adj_index_var += 1; \
            __input = rotate_coord(__input);\
        }\
    }\
    CubeCoord __input = {0,0,0}; \
    coord_var = __input; \
    {visit_code} \
    adj_index_var += 1; \
    }

#define visit_all_coords_1_box(coord_var ,visit_code) \
    {for(int i = -1; i <= 1; i++){ \
        for(int j = -1; j <= 1; j++){ \
            for(int k = -1; k <= 1; k++){ \
                CubeCoord input_coord = {i,j,k};\
                coord_var = input_coord; \
                {visit_code}; \
            } \
        } \
    }}

struct VectorAttraction_{
    Vec3F force_vec;
};
#define VectorAttraction struct VectorAttraction_

void add_quantity(QuantityInfo * dest, QuantityInfo * src);
void subtract_quantity(QuantityInfo * dest, QuantityInfo * src);

struct CubeChangeInfo_{
    VectorAttraction force_shift;
    QuantityInfo quantity_shift;
};
#define CubeChangeInfo struct CubeChangeInfo_

global float * exch_data_at(global float * exch_data,CubeCoord c){
    return exch_data + c_idx(c) * EXCHANGE_LEN;
}

void add_quantity(QuantityInfo * dest, QuantityInfo * src){
    dest->vec = mass(dest) + mass(src) < 1e-10f ?
                zero_vec() :
            (dest->vec * mass(dest) + src->vec * mass(src)) / (mass(dest) + mass(src));

    dest->air_mass += src->air_mass;
    dest->liquid_mass += src->liquid_mass;
    dest->solid_mass += src->solid_mass;
}
void subtract_quantity(QuantityInfo * dest, QuantityInfo * src){
    dest->vec = mass(dest) - mass(src) < 1e-10f ?
                zero_vec() :
            (dest->vec * mass(dest) - src->vec * mass(src)) / (mass(dest) - mass(src));

    dest->air_mass -= src->air_mass;
    dest->liquid_mass -= src->liquid_mass;
    dest->solid_mass -= src->solid_mass;
}

Vec3F reflect_vector_along(Vec3F vector, Vec3F cube_dir){
    //reflects the vector in opposite direction of the cube_dir
    float mag_incident = dot_prod(vector,cube_dir);
    float dampen_value = 0.1f;
    Vec3F refl_vec = vector - cube_dir * mag_incident * (2.0f - dampen_value);
    return refl_vec;
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

    QuantityInfo final_quantity = {amt_air_given,amt_liquid_given,amt_solid_given,0,final_vec};

    VectorAttraction attract_info = {liquid_attraction_vector};
    CubeChangeInfo change_info = {attract_info,final_quantity};
    return change_info;
}
float mass_conv(float m){
    return powf(m,1.0f/3.0f);
}
kernel void update_coord_quantity(global QuantityInfo * source_data, global float * source_bonds, global QuantityInfo * update_data, global float * all_exchange_data){
    CubeCoord base_coord = {
        get_global_id(0),
        get_global_id(1),
        get_global_id(2)
    };
    Vec3F global_gravity_vector = build_vec(0,-gravity_constant * seconds_per_calc,0);

    const QuantityInfo base_orig_quanity = *get(source_data,base_coord);
    QuantityInfo total_quanity = base_orig_quanity;

    Vec3F bond_accel =build_vec(0,0,0);


    CubeCoord offset;
    visit_all_coords_1_box(offset,{
        //bond energy ~ coef * distance^2
        QuantityInfo adj_quantity = *get(source_data,add_coord(base_coord,offset));
        float distance = sqr_len(offset);
        if(distance > 0){
            float bond_val = *get_bond(source_bonds,base_coord,offset);
            float base_mass = mass_conv(base_orig_quanity.solid_mass);
            float adj_mass = mass_conv(adj_quantity.solid_mass);
            float attract_force = base_mass * adj_mass * bond_val / (0.000001f+sqrt(distance)*(base_mass + adj_mass));
            float repell_force = (repell_coef * base_orig_quanity.solid_mass * adj_quantity.solid_mass) /  (0.000001f+sqrt(distance)*(base_orig_quanity.solid_mass  + adj_quantity.solid_mass));
            float accel = (attract_force - repell_force) / (mass(&base_orig_quanity) + 0.00001f);
            Vec3F accel_bond_val = accel * normalize(coord_to_vec(offset));
            bond_accel += accel_bond_val * seconds_per_calc;
            /*
            if(distance == 1){
                Vec3F vel_dif = adj_quantity.vec - base_orig_quanity.vec;
                //float force_mag = base_mass * adj_mass * bond_val;
                Vec3F shear_accel_vel = 0.1f*vel_dif;//(vel_dif * force_mag) / (0.0001f + orig_quantity.mass());
                //bond_accel += shear_accel_vel;
            }*/
        }
    });

    Vec3F total_accel_val = zero_vec();
    global float * base_exchange_data = exch_data_at(all_exchange_data,base_coord);
    float amount_mass_untranfered = base_orig_quanity.solid_mass;

    int offset_index;
    visit_all_adjacent_(offset,offset_index,{
        Vec3F cube_dir = coord_to_vec(offset);
        CubeCoord adj_coord = add_coord(base_coord,offset);
        QuantityInfo adj_orig_quanity = *get(source_data,adj_coord);
        CubeChangeInfo change_info = get_bordering_quantity_vel(base_orig_quanity,adj_orig_quanity,cube_dir);

        QuantityInfo add_vec = change_info.quantity_shift;

        if(is_valid_cube(adj_coord)){
            CubeChangeInfo adj_change_info = get_bordering_quantity_vel(adj_orig_quanity,base_orig_quanity,-cube_dir);

            total_accel_val += (seconds_per_calc / (0.0001f+mass(&base_orig_quanity))) *
                    ( - change_info.force_shift.force_vec + adj_change_info.force_shift.force_vec);

            add_quantity(&total_quanity,&adj_change_info.quantity_shift);
            subtract_quantity(&total_quanity,&add_vec);

            float total_exchange = adj_change_info.quantity_shift.solid_mass - add_vec.solid_mass;
            base_exchange_data[offset_index] = max(0.0f,total_exchange);
            amount_mass_untranfered -= max(0.0f,-total_exchange);//add_vec.solid_mass;
        }
        else{
            //is border cube
            QuantityInfo reflected_vector = add_vec;
            reflected_vector.vec = reflect_vector_along(add_vec.vec,cube_dir);

            add_quantity(&total_quanity,&reflected_vector);
            subtract_quantity(&total_quanity,&add_vec);
            base_exchange_data[offset_index] = 0;
        }
    });

    base_exchange_data[STATIC_EXCH_IDX] = amount_mass_untranfered;

    total_quanity.vec += total_accel_val + global_gravity_vector + bond_accel;
    *get(update_data,base_coord) = total_quanity;
}
kernel void update_bonds(global QuantityInfo * source_data, global QuantityInfo * updated_data, global float * source_bonds, global float * all_exchange_data, global float * update_bonds){
    CubeCoord base_coord = {
        get_global_id(0),
        get_global_id(1),
        get_global_id(2)
    };
    global float * base_exchange_data = exch_data_at(all_exchange_data,base_coord);
    CubeCoord bond_offset;
    visit_all_coords_1_box(bond_offset,{
        CubeCoord bond_eval_coord = add_coord(base_coord,bond_offset);
        if(!is_valid_cube(bond_eval_coord)){
            return;
        }
        float new_bond_energy = 0;
        global float * bond_ev_exch_data = exch_data_at(all_exchange_data,bond_eval_coord);
        int old_base_idx;
        CubeCoord old_base_offset;
        visit_all_adjacent_plus_center(old_base_offset,old_base_idx,{
            CubeCoord old_base = add_coord(base_coord, old_base_offset);
            float amnt_leaving_old_base = base_exchange_data[old_base_idx];
            float amt_old_base = get(source_data,old_base)->solid_mass;
            //float prop_leaving_old_base = amnt_leaving_old_base / max(0.00001f,this->get(old_base).data.solid_mass);
            int old_adj_idx;
            CubeCoord old_adj_offset;
            visit_all_adjacent_plus_center(old_adj_offset,old_adj_idx,{
                CubeCoord old_adj = add_coord(bond_eval_coord, old_adj_offset);
                float amnt_leaving_old_adj = bond_ev_exch_data[old_adj_idx];
                float amt_old_adj = get(source_data,old_adj)->solid_mass;
                //float prop_leaving_old_adj = amnt_leaving_old_adj / max(0.00001f,this->get(old_adj).data.solid_mass);
                CubeCoord dir = sub_coord(old_adj, old_base);
                if(is_valid_bond(dir)){
                    float old_bond_strength = *get_bond(source_bonds,old_base,dir);
                    float old_bond_energy = (old_bond_strength * amt_old_base * amt_old_adj) / (0.000001f + amt_old_base + amt_old_adj);
                    float energy_left = old_bond_energy*amnt_leaving_old_adj*amnt_leaving_old_base/(0.000001f + amt_old_base * amt_old_adj);
                    new_bond_energy += energy_left;
                }
            });
        });
        float mass1 = get(updated_data,base_coord)->solid_mass;
        float mass2 = get(updated_data,bond_eval_coord)->solid_mass;
        float new_bond_coef = ((mass1 + mass2) * new_bond_energy) / (0.000001f +  mass1 * mass2);
        *get_bond(update_bonds,base_coord,bond_offset) = new_bond_coef;
    });
}
