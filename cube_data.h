#pragma once
#include "cube_color.h"
#include "cube_coords.h"
#include <cstdlib>

class CubeInfo{
    float type;
public:
    CubeInfo(){
        type = rand() / float(RAND_MAX);
    }
    void update(){
        type = rand() / float(RAND_MAX);
    }
    RGBVal color(){
        return RGBVal{1.0f-type,1.0,1.0};
    }
    bool is_solid(){
        return type > 0.2;
    }
    bool is_transparent(){
        return type < 0.1;
    }
};

constexpr int size_cube = 128;
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
class CubeData{
    vector<CubeInfo> data;
public:
    CubeData():
        data(size_cube*size_cube*size_cube){}
    CubeInfo & get(int x, int y, int z){
        return data.at(x*size_cube*size_cube + y*size_cube + z);
    }
    CubeInfo & get(CubeCoord c){
        return get(c.x,c.y,c.z);
    }
    vector<FaceDrawInfo> get_exposed_faces(){
        vector<FaceDrawInfo> info;
        visit_all_coords([&](CubeCoord coord){
            visit_all_faces(coord,[&](FaceInfo face){
                if(!is_valid_cube(face.cube_facing())){
                    info.push_back(FaceDrawInfo{this->get(coord).color(),face});
                }
            });
        });
        return info;
    }
    void update(){
        for(CubeInfo & info : data){
            info.update();
        }
    }
};
