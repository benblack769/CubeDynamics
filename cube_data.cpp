#include "cube_data.h"
#include <iostream>
#include "parameters.h"
#include "glm/gtx/string_cast.hpp"
#include "glm/gtx/norm.hpp"
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
void visit_all_adjacent(visit_fn_ty visit_fn){
    for(int n = -1; n <= 1; n += 2){
        visit_fn(CubeCoord{n,0,0});
        visit_fn(CubeCoord{0,n,0});
        visit_fn(CubeCoord{0,0,n});
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
bool is_in_index_bounds(int val){
    return val >= -1 && val <= size_cube;
}
bool is_valid_cube(CubeCoord c){
    return
        is_in_axis_bounds(c.x) &&
        is_in_axis_bounds(c.y) &&
        is_in_axis_bounds(c.z);
}
bool is_valid_index(CubeCoord c){
    return
        is_in_index_bounds(c.x) &&
        is_in_index_bounds(c.y) &&
        is_in_index_bounds(c.z);
}

int int_pow3(int x){
    return x * x * x;
}
QuantityInfo * get(QuantityInfo * data,CubeCoord c){
    assert(is_valid_index(c));
    return data + (((c.x+1)*(size_cube+1) + (c.y+1))*(size_cube+1) + (c.z+1));
}
QuantityInfo * create_data(){
    const int data_size = int_pow3(size_cube+2);
    QuantityInfo * info = new QuantityInfo[data_size]();
    visit_all_coords([&](CubeCoord c){
        *get(info,c) = random_init();
    });
    return info;
}
std::vector<FaceDrawInfo> get_exposed_faces(QuantityInfo * all_data){
    std::vector<FaceDrawInfo> info;
    visit_all_coords([&](CubeCoord coord){
        if(!is_transparent(*get(all_data,coord))){
            visit_all_faces(coord,[&](FaceInfo face){
                if(!is_valid_cube(face.cube_facing()) || is_transparent(*get(all_data,face.cube_facing()))){
                    info.push_back(FaceDrawInfo{color(*get(all_data,coord)),face});
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
int counter = 0;
void update(QuantityInfo * source_data, QuantityInfo * update_data){
    Vec3F global_gravity_vector = build_vec(0,-gravity_constant * seconds_per_calc,0);
    visit_all_coords([&](CubeCoord base_coord){
        QuantityInfo base_orig_quanity = *get(source_data,base_coord);
        QuantityInfo total_quanity = base_orig_quanity;

        Vec3F total_accel_val = zero_vec();
        visit_all_adjacent([&](CubeCoord offset){
            Vec3F cube_dir = coord_to_vec(offset);
            CubeCoord adj_coord = add(base_coord,offset);
            QuantityInfo adj_orig_quanity = *get(source_data,adj_coord);
            CubeChangeInfo change_info = get_bordering_quantity_vel(base_orig_quanity,adj_orig_quanity,cube_dir);

            QuantityInfo add_vec = change_info.quantity_shift;

            if(is_valid_cube(adj_coord)){
                CubeChangeInfo adj_change_info = get_bordering_quantity_vel(adj_orig_quanity,base_orig_quanity,-cube_dir);

                total_accel_val += (seconds_per_calc / (0.0001f+mass(&base_orig_quanity))) *
                        (- change_info.force_shift.force_vec + adj_change_info.force_shift.force_vec);

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
    });
    counter++;
    if(counter % 1000 == 0){
    }
}
