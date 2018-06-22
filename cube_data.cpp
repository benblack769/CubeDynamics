#include "cube_data.h"
#include <iostream>
#include "parameters.h"
#include "glm/gtx/string_cast.hpp"
using namespace std;

constexpr int NUM_BONDS_PER_CUBE = 13;

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
    data(int_pow3(size_cube)),
    bond_data(NUM_BONDS_PER_CUBE*int_pow3(size_cube+1)){
    for(auto & a : data){
        //a.data.solid_mass = 0;
    }
   /* visit_all_coords_between(CubeCoord{2,2,2},CubeCoord{6,6,6},[&](CubeCoord coord){
        this->get(coord).data.solid_mass = 100;
    });
    visit_all_coords_between(CubeCoord{2,12,2},CubeCoord{6,18,6},[&](CubeCoord coord){
        this->get(coord).data.solid_mass = 100;
    });*/
    //get(10,10,10).data.air_mass = 100;
    //get(5,5,5).data.liquid_mass = 100;
}

CubeInfo & CubeData::get(CubeCoord c){
    static CubeInfo border(true);
    return is_valid_cube(c) ?
                data.at(c.x*size_cube*size_cube + c.y*size_cube + c.z):
                border;
}
bool lexical_less_cmp(CubeCoord c1,CubeCoord c2){
    return c1.x < c2.x ||
            (c1.x == c2.x  &&
             c1.y < c2.y || (
                 c1.y == c2.y &&
                 c1.z < c2.z));
}
int bond_coord_offset(CubeCoord o){
    return o.x == -1 ?
                (o.y+1)*3 + (o.z+1) :
                o.y == -1 ?
                        9 + (o.z+1) :
                        13;
}
int bond_coord_offset(int x, int y, int z){
    return x == -1 ?
                (y+1)*3 + (z+1) :
                y == -1 ?
                        9 + (z+1) :
                        12;
}
int bond_coord_offset_alt2(int x, int y, int z){
    //TODO: test this before using it!!!!!!!!!
    int xn1 = x;//twos complement insures that if x == -1, it is equal to ~0
    int yn1 = y;
    return (xn1 & ((y+1)*3 + (z+1))) +
           (~xn1 & (
               (yn1 & ((z+1)+9)) +
               (~yn1 & 12)));
}
int get_coord_bond_block_start(CubeCoord c){
    return (c.x+1)*(size_cube+1)*(size_cube+1) + (c.y+1)*(size_cube+1) + (c.z+1);
}
float & CubeData::get_bond(CubeCoord coord, CubeCoord dir){
    /*
    Note that in bonds between cubes, a bond is specified by a pair of coordinates:

    ((x,y,z), (i,j,k))

    where i,j, and k have magnitude at most 1, and (i,j,k) != (0,0,0)

    Note that since bonds are bi-directional, the following values are equivalent:

    ((x,y,z), (i,j,k))
    ((x+i,y+j,z+k), (-i,-j,-k))

    So when finding a bond in memory, both alternatives are calculated, and
    the smaller one (compared with a lexical compare) is found.
    They cannot be equal since (i,j,k) != (0,0,0).

    The 13 bonds associated with that block are stored in a block owned by that smaller coordinate
    Note that the 0 border also has bonds.
    */
    assert(dir != CubeCoord(0,0,0));
    CubeCoord alt_coord = coord + dir;
    CubeCoord alt_dir = - dir;

    bool coord_less = lexical_less_cmp(coord,alt_coord);
    CubeCoord use_coord = coord_less ? coord : alt_coord;
    CubeCoord use_dir = coord_less ? dir : alt_dir;

    int loc = get_coord_bond_block_start(use_coord) + bond_coord_offset(use_dir);
    return this->bond_data.at(loc);
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
        Vec3F bond_accel(0,0,0);
        visit_all_coords_1_around(base_coord,[&](CubeCoord offset){
            CubeCoord new_coord = base_coord + offset;
            float bond_val = this->get_bond(base_coord,offset);
            bond_accel += energy_to_accel(bond_val,this->get(base_coord).data.mass()) * glm::normalize(coord_to_vec(offset));
        });
        QuantityInfo total_quanity = this->get(base_coord).data;
        Vec3F total_accel_val = Vec3F(0,0,0);
        visit_all_adjacent(base_coord,[&](CubeCoord adj_coord, Vec3F cube_dir){
            CubeChangeInfo change_info = this->get(base_coord).get_bordering_quantity_vel(this->get(adj_coord),cube_dir);

            QuantityInfo add_vec = change_info.quantity_shift;

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
        total_quanity.vec += total_accel_val + global_gravity_vector + bond_accel;
        update_data.get(base_coord).data = total_quanity;
    });
}
