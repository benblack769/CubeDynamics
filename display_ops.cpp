#include "display_ops.h"
#include "parameters.h"

constexpr float solid_mass_start_val = 60;

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
    for(int i = lower.x; i <= upper.x; i++){
        for(int j = lower.y; j <= upper.y; j++){
            for(int k = lower.z; k <= upper.z; k++){
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
    data.liquid_mass = 0*rand() / float(RAND_MAX);
    data.solid_mass = 0.01*rand() / float(RAND_MAX);
    data.vec = zero_vec();
    return data;
}
void init_quantity_data(QuantityInfo * data){
    for(int i = 0; i < data_size(); i++){
        data[i] = QuantityInfo{0,0,0,0,zero_vec()};
    }
    visit_all_coords([&](CubeCoord c){
        *get(data,c) = random_init();
    });
    visit_all_coords_between(CubeCoord{2,2,2},CubeCoord{15,15,15},[&](CubeCoord coord){
        get(data,coord)->solid_mass = solid_mass_start_val + rand()/float(RAND_MAX);
    });
    visit_all_coords_between(CubeCoord{17,4,4},CubeCoord{19,6,6},[&](CubeCoord coord){
        get(data,coord)->solid_mass = solid_mass_start_val + rand()/float(RAND_MAX);
        //get(data,coord)->vec = build_vec(-1000,0,0);
    });
}
std::vector<QuantityInfo> create_quantity_data_vec(){
    std::vector<QuantityInfo> quant_vec(data_size());
    init_quantity_data(quant_vec.data());
    return quant_vec;
}
std::vector<float> create_exchange_vec(){
    return std::vector<float>(data_size() * EXCHANGE_LEN);
}
std::vector<float> create_bond_vec(QuantityInfo * quantities){
    std::vector<float> res(data_size() * NUM_BONDS_PER_CUBE);

    visit_all_coords([&](CubeCoord coord){
        visit_all_coords_1_box([&](CubeCoord offset){
            CubeCoord new_coord = add_coord(coord, offset);
            float bond_strength = bond_strength_coef *
                    (get(quantities,coord)->solid_mass *
                    get(quantities,new_coord)->solid_mass /
                    (square(solid_mass_start_val)));
            *get_bond(res.data(),coord,offset) = bond_strength;
        });
    });
    return res;
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
