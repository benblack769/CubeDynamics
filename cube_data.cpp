#include "cube_data.h"
#include <iostream>
#include "parameters.h"
#include "glm/gtx/string_cast.hpp"
using namespace std;


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
void visit_all_adjacent(CubeCoord cube,visit_fn_ty visit_fn){
    int i = cube.x, j = cube.y, k = cube.z;
    for(int n = -1; n <= 1; n += 2){
        visit_fn(CubeCoord{i+n,j,k},glm::vec3(n,0,0));
        visit_fn(CubeCoord{i,j+n,k},glm::vec3(0,n,0));
        visit_fn(CubeCoord{i,j,k+n},glm::vec3(0,0,n));
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

CubeData::CubeData():
    data(size_cube*size_cube*size_cube){
    for(auto & a : data){
        a.data.solid_mass = 0;
    }
    visit_all_coords_between(CubeCoord{2,2,2},CubeCoord{6,6,6},[&](CubeCoord coord){
        this->get(coord).data.solid_mass = 100;
    });
    visit_all_coords_between(CubeCoord{2,12,2},CubeCoord{6,18,6},[&](CubeCoord coord){
        this->get(coord).data.solid_mass = 100;
    });
    //get(10,10,10).data.air_mass = 100;
    //get(5,5,5).data.liquid_mass = 100;
}

CubeInfo & CubeData::get(int x, int y, int z){
    static CubeInfo border(true);
    return is_valid_cube(CubeCoord{x,y,z}) ?
                data.at(x*size_cube*size_cube + y*size_cube + z):
                border;
}
CubeInfo & CubeData::get(CubeCoord c){
    return get(c.x,c.y,c.z);
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
glm::vec3 reflect_vector_along(glm::vec3 vector, glm::vec3 cube_dir){
    //reflects the vector in opposite direction of the cube_dir
    float mag_incident = glm::dot(vector,cube_dir);
    //assert(mag_incident >= 0);
    float dampen_value = 0.1f;
    glm::vec3 refl_vec = vector - cube_dir * mag_incident * (2.0f - dampen_value);
    return refl_vec;
}
void quantities_echange_force(QuantityInfo & one, QuantityInfo & other,glm::vec3 force_vector){
    glm::vec3 accel_1 = force_vector / (0.0001f+one.mass());
    glm::vec3 accel_2 = force_vector / (0.0001f+other.mass());
    glm::vec3 speed_1 = accel_1 * seconds_per_calc;
    glm::vec3 speed_2 = accel_2 * seconds_per_calc;
    one.vec -= speed_1;
    other.vec += speed_2;
}
void CubeData::update(){
    CubeData new_iter = *this;
    visit_all_coords([&](CubeCoord base_coord){
        visit_all_adjacent(base_coord,[&](CubeCoord adj_coord, glm::vec3 cube_dir){
            CubeChangeInfo change_info = this->get(base_coord).get_bordering_quantity_vel(this->get(adj_coord),cube_dir);
            QuantityInfo add_vec = change_info.quantity_shift;

            if(is_valid_cube(adj_coord)){
                new_iter.get(adj_coord).data.add(add_vec);
                new_iter.get(base_coord).data.subtract(add_vec);

                quantities_echange_force(new_iter.get(base_coord).data,new_iter.get(adj_coord).data,change_info.force_shift.force_vec);
            }
            else{
                //assert(change_info.force_shift.force_vec == glm::vec3(0,0,0));
                //is border cube
                QuantityInfo reflected_vector = add_vec;
                reflected_vector.vec = reflect_vector_along(add_vec.vec,cube_dir);

                new_iter.get(base_coord).data.add(reflected_vector);
                new_iter.get(base_coord).data.subtract(add_vec);
            }
        });
    });
    for(CubeInfo & info : new_iter.data){
        info.update_velocity_global();
    }
    this->data.swap(new_iter.data);
}
