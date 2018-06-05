#include "cube_data.h"
#include <iostream>
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
        //if(!this->get(coord).is_transparent()){
            visit_all_faces(coord,[&](FaceInfo face){
                if(!is_valid_cube(face.cube_facing())){// || this->get(face.cube_facing()).is_transparent()){
                    info.push_back(FaceDrawInfo{this->get(coord).color(),face});
                }
            });
        //}
    });
    return info;
}
void CubeData::update(){
    CubeData new_iter(*this);
    visit_all_coords([&](CubeCoord base_coord){
        visit_all_adjacent(base_coord,[&](CubeCoord adj_coord, glm::vec3 cube_dir){
            MassVec add_vec = this->get(base_coord).get_bordering_quantity_vel(cube_dir);
            if(is_valid_cube(adj_coord)){
                new_iter.get(adj_coord).add_massvec(add_vec);
                new_iter.get(base_coord).subtract_mass(add_vec);
            }
            else{
                //is border cube
                float mag_incident = glm::dot(add_vec.vec,cube_dir);
                if(mag_incident > 0){
                    //cout << mag_incident << endl;
                    glm::vec3 refl_vec = add_vec.vec - cube_dir * mag_incident * 2.0f;
                    MassVec reflected_vector{add_vec.mass,refl_vec};
                    new_iter.get(base_coord).subtract_mass(reflected_vector);
                    new_iter.get(base_coord).add_massvec(reflected_vector);
                }
            }
        });
    });
    for(CubeInfo & info : new_iter.data){
        info.update_velocity_global();
    }
    *this = new_iter;
}
