#include "cube_data.h"
#include <iostream>
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
      //  a.quantity = 0.5;
    }
    //get(10,10,10).quantity = 100;
    //get(5,5,5).quantity = 100;
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
int counter = 0;
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
                counter++;
                //is border cube
                //cout << to_string(cube_dir) << endl;
                //cout << adj_coord << endl;
                float mag_incident = glm::dot(add_vec.vec,cube_dir);
                assert(mag_incident >= 0);
                //cout << mag_incident << endl;
                float reflection_friction = 0.0;
                glm::vec3 refl_vec = add_vec.vec - cube_dir * mag_incident * (2.0f - reflection_friction);
                //cout << "refl" << endl;
                //cout << glm::to_string(add_vec.vec) << endl;
                //cout << glm::to_string(cube_dir) << endl;
                //cout << glm::to_string(refl_vec) << endl;
                MassVec reflected_vector{add_vec.mass,refl_vec};
                /*if(this->get(base_coord).quantity > 100){
                    cout << "start" << endl;
                    cout << to_string(cube_dir) << endl;
                    cout << to_string(add_vec.vec) << endl;
                    cout << to_string(add_vec.mass) << endl;
                    cout << to_string(refl_vec) << endl;
                    cout << to_string(this->get(base_coord).quantity) << endl;
                    cout << to_string(this->get(base_coord).velocity) << endl;
                    cout << to_string(new_iter.get(base_coord).quantity) << endl;
                    cout << to_string(new_iter.get(base_coord).velocity) << endl;
                }*/
                new_iter.get(base_coord).subtract_mass(add_vec);
                new_iter.get(base_coord).add_massvec(reflected_vector);
                /*if(this->get(base_coord).quantity > 100){
                    cout << to_string(new_iter.get(base_coord).quantity) << endl;
                    cout << to_string(new_iter.get(base_coord).velocity) << endl;
                }*/
            }
        });
    });
    for(CubeInfo & info : new_iter.data){
        info.update_velocity_global();
    }
    *this = new_iter;
}
