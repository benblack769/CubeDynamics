#include "cube_data.h"
#include <iostream>
#include "parameters.h"
#include "glm/gtx/string_cast.hpp"
#include "glm/gtx/norm.hpp"
using namespace std;

constexpr int NUM_BONDS_PER_CUBE = 27;

int sqr(int x){
    return x * x;
}
int sqr_len(CubeCoord c){
    return sqr(c.x) + sqr(c.y) + sqr(c.z);
}
int sqr_dist(CubeCoord c1, CubeCoord c2){
    return sqr_len(c1 - c2);
}
bool is_valid_bond(CubeCoord offset){
    return sqr_len(offset) <= 3;
}
Vec3F coord_to_vec(CubeCoord c){
    return Vec3F(c.x,c.y,c.z);
}
template<class visit_fn_ty>
void visit_all_coords(visit_fn_ty visit_fn){
    for(int i = 0; i < size_cube; i++){
        for(int j = 0; j < size_cube; j++){
            for(int k = 0; k < size_cube; k++){
                visit_fn(CubeCoord{i,j,k});
            }
        }
    }
}
template<class visit_fn_ty>
void visit_all_coords_between(CubeCoord lower,CubeCoord upper,visit_fn_ty visit_fn){
    assert(
                lower.x < upper.x &&
                lower.y < upper.y &&
                lower.z < upper.z
        );
    for(int i = lower.x; i < upper.x; i++){
        for(int j = lower.y; j < upper.y; j++){
            for(int k = lower.z; k < upper.z; k++){
                visit_fn(CubeCoord{i,j,k});
            }
        }
    }
}
template<class visit_fn_ty>
void visit_all_coords_1_box(visit_fn_ty visit_fn){
    for(int i = -1; i <= 1; i++){
        for(int j = -1; j <= 1; j++){
            for(int k = -1; k <= 1; k++){
                visit_fn(CubeCoord{i,j,k});
            }
        }
    }
}

template<class visit_fn_ty>
void visit_all_adjacent(visit_fn_ty visit_fn){
    int adj_index = 0;
    for(int n = -1; n <= 1; n += 2){
        visit_fn(adj_index+0,CubeCoord(n,0,0));
        visit_fn(adj_index+1,CubeCoord(0,n,0));
        visit_fn(adj_index+2,CubeCoord(0,0,n));
        adj_index += 3;
    }
}

template<class visit_fn_ty>
void visit_all_adjacent_plus_center(visit_fn_ty visit_fn){
    visit_all_adjacent(visit_fn);
    visit_fn(SIDES_ON_CUBE,CubeCoord(0,0,0));
}
template<class visit_fn_ty>
void visit_all_faces(CubeCoord cube,visit_fn_ty visit_fn){
    for(int axis = 0; axis < 3; axis++){
        visit_fn(FaceInfo{cube,false,axis});
        visit_fn(FaceInfo{cube,true,axis});
    }
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
int int_pow3(int x){
    return x * x * x;
}
CubeData::CubeData():
    data(int_pow3(size_cube)),
    bond_data(NUM_BONDS_PER_CUBE*int_pow3(size_cube),0){
    for(auto & a : data){
        a.data.solid_mass = 0.001;
    }
    float solid_mass_start_val = 100;
    visit_all_coords_between(CubeCoord{2,2,2},CubeCoord{6,6,6},[&](CubeCoord coord){
        this->get(coord).data.solid_mass = solid_mass_start_val;
    });
    visit_all_coords_between(CubeCoord{4,12,4},CubeCoord{8,18,8},[&](CubeCoord coord){
        this->get(coord).data.solid_mass = solid_mass_start_val;
    });
    visit_all_coords_between(CubeCoord{15,1,15},CubeCoord{19,5,19},[&](CubeCoord coord){
        this->get(coord).data.solid_mass = solid_mass_start_val;
    });
    visit_all_coords([&](CubeCoord coord){
        visit_all_coords_1_box([&](CubeCoord offset){
            CubeCoord new_coord = coord + offset;
            float bond_strength = bond_strength_coef *
                    this->get(coord).data.solid_mass *
                    this->get(new_coord).data.solid_mass /
                    (square(solid_mass_start_val) * (1+sqr_len(offset)));
            this->get_bond(coord,offset) = bond_strength;
        });
    });
    //get(10,10,10).data.air_mass = 100;
    //get(5,5,5).data.liquid_mass = 100;
}

CubeInfo & CubeData::get(CubeCoord c){
    static CubeInfo border(true);
    return is_valid_cube(c) ?
                data.at(c.x*size_cube*size_cube + c.y*size_cube + c.z):
                border;
}
int bond_coord_offset(CubeCoord o){
    assert(is_valid_bond(o));
    return ((o.x+1) * 3 + (o.y+1)) * 3 + (o.z + 1);
}
float * get_coord_bond_block_start(vector<float> & bond_data, CubeCoord c){
    static float border_bond[NUM_BONDS_PER_CUBE] = {0};
    return is_valid_cube(c) ? &bond_data.at(((c.x)*(size_cube)*(size_cube) + (c.y)*(size_cube) + (c.z)) * NUM_BONDS_PER_CUBE)
                            : border_bond;
}
float & CubeData::get_bond(CubeCoord coord, CubeCoord dir){
    int offset = bond_coord_offset(dir);
    return get_coord_bond_block_start(this->bond_data,coord)[offset];
}
std::vector<FaceDrawInfo> CubeData::get_exposed_faces(){
    std::vector<FaceDrawInfo> info;
    visit_all_coords([&](CubeCoord coord){
        if(!this->get(coord).is_transparent()){
            visit_all_faces(coord,[&](FaceInfo face){
                if(!is_valid_cube(face.cube_facing()) || this->get(face.cube_facing()).is_transparent()){
                    info.push_back(FaceDrawInfo{this->get(coord).color(),face});
                }
            });
        }
    });
    return info;
}
Vec3F reflect_vector_along(Vec3F vector, Vec3F cube_dir){
    //reflects the vector in opposite direction of the cube_dir
    float mag_incident = glm::dot(vector,cube_dir);
    //assert(mag_incident >= 0);
    float dampen_value = 0.1f;
    Vec3F refl_vec = vector - cube_dir * mag_incident * (2.0f - dampen_value);
    return refl_vec;
}
float mass_force_coef(float one_mass){
    float accel_1 = 1.0f / (0.0001f+one_mass);
    //Vec3F accel_2 = force_vector / (0.0001f+other_mass);
    float speed_1 = accel_1 * seconds_per_calc;
    //Vec3F speed_2 = accel_2 * seconds_per_calc;
    return speed_1;
}
float energy_to_accel(float energy_val, float mass){
    return sqrt(2.0f * energy_val / max(0.0001f,mass));
}
constexpr int EXCHANGE_LEN = 7;
constexpr int STATIC_EXCH_IDX = EXCHANGE_LEN-1;
float * exch_data_at(vector<float> & exch_data,CubeCoord c){
    static float border_weights[7] = {0};
    return is_valid_cube(c) ?
                &exch_data.at((c.x*size_cube*size_cube + c.y*size_cube + c.z) * EXCHANGE_LEN):
                border_weights;
}
void normalize_exchange(float * exch){
    float sum = 0;
    for(int i = 0; i < EXCHANGE_LEN; i++){
        sum += exch[i];
    }
    float inv_val = 1.0f/max(0.0000001f,sum);
    for(int i = 0; i < EXCHANGE_LEN; i++){
        exch[i] *= inv_val;
    }
}
int counter = 0;
void CubeData::update(CubeData & update_data){
    Vec3F global_gravity_vector = Vec3F(0,-gravity_constant * seconds_per_calc,0);

    vector<float> all_exchange_data(EXCHANGE_LEN*int_pow3(size_cube),0);

    visit_all_coords([&](CubeCoord base_coord){

        Vec3F bond_accel(0,0,0);

        visit_all_coords_1_box([&](CubeCoord offset){
            //bond energy ~ coef * distance^2
            float distance = sqr_len(offset);
            if(distance != 0){
                float bond_val = this->get_bond(base_coord,offset);
                float bond_accel_mag = distance * energy_to_accel(bond_val,this->get(base_coord).data.mass());
                glm::vec3 accel_bond_val = (bond_accel_mag / sqrt(distance)) * coord_to_vec(offset);
                bond_accel += accel_bond_val;
            }
        });

        QuantityInfo total_quanity = this->get(base_coord).data;
        Vec3F total_accel_val = Vec3F(0,0,0);

        float * base_exchange_data = exch_data_at(all_exchange_data,base_coord);
        float total_exchange_data = 0;

        visit_all_adjacent([&](int offset_index, CubeCoord offset){
            Vec3F cube_dir = coord_to_vec(offset);
            CubeCoord adj_coord = base_coord + offset;
            CubeChangeInfo change_info = this->get(base_coord).get_bordering_quantity_vel(this->get(adj_coord),cube_dir);

            QuantityInfo add_vec = change_info.quantity_shift;

            //QuantityInfo shift_across_border = {1,0,0,Vec3F(0,0,0)};

            if(is_valid_cube(adj_coord)){
                CubeChangeInfo adj_change_info = this->get(adj_coord).get_bordering_quantity_vel(this->get(base_coord),-cube_dir);

                total_accel_val += mass_force_coef(this->get(base_coord).data.mass()) *
                        (- change_info.force_shift.force_vec + adj_change_info.force_shift.force_vec);

                total_quanity.add(adj_change_info.quantity_shift);
                total_quanity.subtract(add_vec);

                float solid_mass_delta = add_vec.solid_mass;
                base_exchange_data[offset_index] = solid_mass_delta;
                total_exchange_data += solid_mass_delta;
            }
            else{
                //assert(change_info.force_shift.force_vec == Vec3F(0,0,0));
                //is border cube
                QuantityInfo reflected_vector = add_vec;
                reflected_vector.vec = reflect_vector_along(add_vec.vec,cube_dir);

                total_quanity.add(reflected_vector);
                total_quanity.subtract(add_vec);
            }
        });

        base_exchange_data[STATIC_EXCH_IDX] = this->get(base_coord).data.solid_mass - total_exchange_data;
        assert(base_exchange_data[STATIC_EXCH_IDX] >= 0);
        normalize_exchange(base_exchange_data);
        //base_exchange_data[STATIC_EXCH_IDX] = max(base_exchange_data[STATIC_EXCH_IDX], 0.0f);

        //total_quanity.add();
        total_quanity.vec += total_accel_val + global_gravity_vector + bond_accel;

        if(counter % 1000 == 0){
        }
        counter++;
        update_data.get(base_coord).data = total_quanity;
    });

    visit_all_coords([&](CubeCoord base_coord){
        float * base_exchange_data = exch_data_at(all_exchange_data,base_coord);
        visit_all_coords_1_box([&](CubeCoord bond_offset){
            CubeCoord bond_eval_coord = base_coord + bond_offset;
            float new_bond_strength = 0;
            float * bond_ev_exch_data = exch_data_at(all_exchange_data,bond_eval_coord);
            visit_all_adjacent_plus_center([&](int old_base_idx, CubeCoord old_base_offset){
                CubeCoord old_base = base_coord + old_base_offset;
                float prop_leaving_old_base = base_exchange_data[old_base_idx];
                //float prop_leaving_old_base = amnt_leaving_old_base / max(0.00001f,this->get(old_base).data.solid_mass);
                visit_all_adjacent_plus_center([&](int old_adj_idx, CubeCoord old_adj_offset){
                    CubeCoord old_adj = bond_eval_coord + old_adj_offset;
                    float prop_leaving_old_adj = bond_ev_exch_data[old_adj_idx];
                    //float prop_leaving_old_adj = amnt_leaving_old_adj / max(0.00001f,this->get(old_adj).data.solid_mass);
                    CubeCoord dir = old_adj - old_base;
                    if(is_valid_bond(dir)){
                        float old_bond_strength = this->get_bond(old_base,dir);
                        float amnt_bond_moved = old_bond_strength * prop_leaving_old_base * prop_leaving_old_adj;
                        new_bond_strength += amnt_bond_moved;
                    }
                });
            });
            update_data.get_bond(base_coord,bond_offset) = new_bond_strength;

            if(counter % 10000 == 0){
                //cout << new_bond_strength <<endl;
            }
            if(new_bond_strength < 0){
                cout << new_bond_strength <<endl;
            }
            counter++;
        });
    });

    float max_bond_stren = 0;
    //bond similarity assertion
    visit_all_coords([&](CubeCoord base_coord){
        visit_all_coords_1_box([&](CubeCoord offset){
            CubeCoord other_coord = base_coord + offset;
            CubeCoord other_offset = - offset;
            float one_bond = update_data.get_bond(base_coord,offset);
            float other_bond = update_data.get_bond(other_coord,other_offset);
            assert(abs(one_bond - other_bond) < 0.0001f || abs(one_bond) * 0.99 <= abs(other_bond));
            float average_bond = (one_bond + other_bond) / 2;
            update_data.get_bond(base_coord,offset) = average_bond;
            update_data.get_bond(other_coord,other_offset) = average_bond;
            max_bond_stren = max(max_bond_stren,one_bond);
        });
    });
    cout << max_bond_stren << endl;
}
