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
void visit_coords_around(CubeCoord start, CubeCoord end, visit_fn_ty visit_fn){
    for(int i = start.x; i <= end.x; i++){
        for(int j = start.y; j <= end.y; j++){
            for(int k = start.z; k <= end.z; k++){
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
    data.air_mass = 0*rand() / float(RAND_MAX);
    data.liquid_mass = 200*rand() / float(RAND_MAX);
    data.solid_mass = 0*rand() / float(RAND_MAX);
    data.vec = zero_vec();
    return data;
}
int data_size(){
    return int_pow3(size_cube+2);
}
void init_data(QuantityInfo * data){
    for(int i = 0; i < data_size(); i++){
        data[i] = QuantityInfo{0,0,0,0,zero_vec()};
    }
    int size = size_cube/2;
    int start = size_cube/4;
    int end = (size_cube*3)/4;
    visit_coords_around(CubeCoord{start,start,start},CubeCoord{end,end,end},[&](CubeCoord c){
        *get(data,c) = random_init();
    });
    /*
    visit_coords_around(CubeCoord{0,0,0},CubeCoord{99,20,99},[&](CubeCoord c){
        *get(data,c) = random_init();
        //get(data,c)->vec = build_vec(50,0,0);
    });
    visit_coords_around(CubeCoord{35,50,35},CubeCoord{60,75,60},[&](CubeCoord c){
        *get(data,c) = random_init();
        get(data,c)->vec = build_vec(0,-500,0);
    });
    */
}
std::vector<QuantityInfo> create_data_vec(){
    std::vector<QuantityInfo> quant_vec(data_size());
    init_data(quant_vec.data());
    return quant_vec;
}
QuantityInfo * create_data(){
    QuantityInfo * info = new QuantityInfo[data_size()];
    init_data(info);
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
