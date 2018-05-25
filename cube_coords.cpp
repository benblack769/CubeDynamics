#include <vector>
#include <cassert>
#include <iostream>
#include "cube_coords.h"

using namespace std;

struct VertexCoord{
    float x;
    float y;
    float z;
    VertexCoord(CubeCoord c):
        x(c.x),
        y(c.y),
        z(c.z){}
    VertexCoord(float ix,float iy,float iz):
        x(ix),
        y(iy),
        z(iz){}
    VertexCoord operator +(VertexCoord o){
        return VertexCoord{
            x + o.x,
            y + o.y,
            z + o.z,
        };
    }
    void add_to_buffer(vector<float> & vertex_buffer){
        vertex_buffer.push_back(x);
        vertex_buffer.push_back(y);
        vertex_buffer.push_back(z);
    }
    bool operator ==(VertexCoord o){
        return
            o.x == x &&
            o.y == y &&
            o.z == z;
    }
    bool operator !=(VertexCoord o){
        return !(*this == o);
    }
    VertexCoord flip1(){
        return VertexCoord(
            1-x,
            1-y,
            1-z
        );
    }
};

struct TriCoords{
    VertexCoord c1;
    VertexCoord c2;
    VertexCoord c3;
    TriCoords(VertexCoord ic1,VertexCoord ic2,VertexCoord ic3):
        c1(ic1),
        c2(ic2),
        c3(ic3){}
    void add_to_buffer(vector<float> & vertex_buffer){
        c1.add_to_buffer(vertex_buffer);
        c2.add_to_buffer(vertex_buffer);
        c3.add_to_buffer(vertex_buffer);
    }
    TriCoords translate(VertexCoord t){
        return TriCoords(
            c1 + t,
            c2 + t,
            c3 + t
        );
    }
    TriCoords flip1(){
        return TriCoords(
            c1.flip1(),
            c2.flip1(),
            c3.flip1()
        );
    }
};
vector<TriCoords> get_cube_additions(){
    vector<TriCoords> faces_0;
    vector<VertexCoord> basis =  {VertexCoord(1,0,0),VertexCoord(0,1,0),VertexCoord(0,0,1)};
    for(int i = 0; i < 3; i++){
        for(int j = 0; j < i; j++){
            VertexCoord c1 = basis[i];
            VertexCoord c2 = basis[j];
            VertexCoord c3_1 = c1 + c2;
            faces_0.push_back(TriCoords(c1,c2,c3_1));
            VertexCoord c3_2(0,0,0);
            faces_0.push_back(TriCoords(c1,c2,c3_2));
        }
    }
    vector<TriCoords> all_faces = faces_0;
    for(TriCoords coord : faces_0){
        all_faces.push_back(coord.flip1());
    }
    return all_faces;
}
VertexCoord to_vertex_coord(CubeCoord x){
    return VertexCoord(
        x.x,
        x.y,
        x.z
    );
}
VertexCoord axis_basis(int axis){
    //axis ranges from
    assert(axis >= 0 && axis < 3);
    VertexCoord res(0,0,0);
    switch(axis){
        case 0:res.x = 1;break;
        case 1:res.y = 1;break;
        case 2:res.z = 1;break;
    }
    return res;
}
void FaceInfo::buffer_verticies(vector<float> & vertex_buffer){
    VertexCoord zero_v(0,0,0);
    VertexCoord axis_vec = axis_basis(this->axis_1());
    VertexCoord next_axis_vec = axis_basis(this->axis_2());
    VertexCoord double_axis = axis_vec + next_axis_vec;
    TriCoords t1(zero_v,axis_vec,double_axis);
    TriCoords t2(zero_v,next_axis_vec, double_axis);
    if(this->reversed){
        t1 = t1.flip1();
        t2 = t2.flip1();
    }
    VertexCoord base_p = to_vertex_coord(this->base_coord);
    t1.translate(base_p).add_to_buffer(vertex_buffer);
    t2.translate(base_p).add_to_buffer(vertex_buffer);
}

/*void add_cube_vertex(CubeCoord cube,vector<float> & vertex_buffer){
    vector<TriCoords> base_coords = get_cube_additions();
    VertexCoord cube_base = to_vertex_coord(cube);
    for(TriCoords tc : base_coords){
        tc.translate(cube_base).add_to_buffer(vertex_buffer);
    }
}
*/
