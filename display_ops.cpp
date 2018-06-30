#include "display_ops.h"


inline int int_pow3(int x){
    return x * x * x;
}
inline int sqr(int x){
    return x * x;
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

RGBVal color(QuantityInfo info){
    return RGBVal{1.0f-std::min(abs(info.air_mass)/2.0f,1.0f),1.0f-std::min(abs(info.solid_mass)/1000.0f,1.0f),1.0f-std::min(abs(info.liquid_mass)/400.0f,1.0f),1.0};
}
bool is_transparent(QuantityInfo info){
    return mass(&info) < 0.03;
}

QuantityInfo random_init(){
    QuantityInfo data;
    data.air_mass = 1*rand() / float(RAND_MAX);
    data.liquid_mass = 0*rand() / float(RAND_MAX);
    data.solid_mass = 0*rand() / float(RAND_MAX);
    data.vec = zero_vec();
    return data;
}
QuantityInfo * create_data(){
    const int data_size = int_pow3(size_cube+2);
    QuantityInfo * info = new QuantityInfo[data_size];
    for(int i = 0; i < data_size; i++){
        info[i] = QuantityInfo{0,0,0,zero_vec()};
    }
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
