#include "cube_data.h"
#include <iostream>
#include "parameters.h"
#include "glm/gtx/string_cast.hpp"
#include "glm/gtx/norm.hpp"
using namespace std;

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
void visit_all_coords_1_around(CubeCoord center,visit_fn_ty visit_fn){
    for(int i = center.x-1; i <= center.x+1; i++){
        for(int j = center.y-1; j <= center.y+1; j++){
            for(int k = center.z-1; k <= center.z+1; k++){
                if(i != 0 || j != 0 || k != 0){
                    visit_fn(CubeCoord{i,j,k});
                }
            }
        }
    }
}

template<class visit_fn_ty>
void visit_all_adjacent(CubeCoord cube,visit_fn_ty visit_fn){
    int i = cube.x, j = cube.y, k = cube.z;
    for(int n = -1; n <= 1; n += 2){
        visit_fn(CubeCoord{i+n,j,k},Vec3F(n,0,0));
        visit_fn(CubeCoord{i,j+n,k},Vec3F(0,n,0));
        visit_fn(CubeCoord{i,j,k+n},Vec3F(0,0,n));
    }
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
    data(int_pow3(size_cube)){
}

CubeInfo & CubeData::get(CubeCoord c){
    static CubeInfo border(true);
    return is_valid_cube(c) ?
                data.at(c.x*size_cube*size_cube + c.y*size_cube + c.z):
                border;
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
    return sqrt(2.0f * energy_val / mass);
}
void CubeData::update(CubeData & update_data){
    Vec3F global_gravity_vector = Vec3F(0,-gravity_constant * seconds_per_calc,0);
    visit_all_coords([&](CubeCoord base_coord){
        QuantityInfo total_quanity = this->get(base_coord).data;

        Vec3F total_accel_val = Vec3F(0,0,0);
        visit_all_adjacent(base_coord,[&](CubeCoord adj_coord, Vec3F cube_dir){
            CubeChangeInfo change_info = this->get(base_coord).get_bordering_quantity_vel(this->get(adj_coord),cube_dir);

            QuantityInfo add_vec = change_info.quantity_shift;

            QuantityInfo shift_across_border = {1,0,0,Vec3F(0,0,0)};

            if(is_valid_cube(adj_coord)){
                CubeChangeInfo adj_change_info = this->get(adj_coord).get_bordering_quantity_vel(this->get(base_coord),-cube_dir);

                total_accel_val += mass_force_coef(this->get(base_coord).data.mass()) *
                        (- change_info.force_shift.force_vec + adj_change_info.force_shift.force_vec);

                total_quanity.add(adj_change_info.quantity_shift);
                total_quanity.subtract(add_vec);
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
        total_quanity.vec += total_accel_val + global_gravity_vector;
        update_data.get(base_coord).data = total_quanity;
    });
}
