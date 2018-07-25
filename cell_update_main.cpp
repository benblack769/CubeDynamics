#include "cell_update_main.h"
#include "update.h"
#include "display_ops.h"
#include <algorithm>

RenderBufferData all_buffer_data;

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

class CubeSharedData{
private:
    vector<QuantityInfo> cube_data;
    mutex lock;
public:
    CubeSharedData(size_t size):
        cube_data(size){
    }
    void read_into(vector<QuantityInfo> & cube_buffer){
        lock.lock();
        assert(cube_buffer.size() == cube_data.size());
        copy(cube_data.begin(),cube_data.end(),cube_buffer.begin());
        lock.unlock();
    }
    void write(vector<QuantityInfo> & buffer){
        lock.lock();
        assert(buffer.size() == cube_data.size());
        copy(buffer.begin(),buffer.end(),cube_data.begin());
        lock.unlock();
    }
} cube_shared_data(data_size());

void cell_triagulize_main_loop(){
    FrameRateControl cube_update_count(10.0);

    vector<QuantityInfo> cube_buf = create_quantity_data_vec();
    while(true){
        if(cube_update_count.should_render()){
            cube_update_count.rendered();
            cube_shared_data.read_into(cube_buf);
            vector<FaceDrawInfo> draw_info = get_exposed_faces(cube_buf.data());
            vector<BYTE> cube_colors;
            cout << get(cube_buf.data(),CubeCoord{3,1,7})->liquid_mass << endl;
            vector<float> cube_verticies;
            for(FaceDrawInfo & info : draw_info){
                info.add_to_buffer(cube_colors,cube_verticies);
            }
            all_buffer_data.set_vals(cube_colors,cube_verticies);
        }
        if(!cube_update_count.should_render()){
            cube_update_count.spin_sleep();
        }
    }
}

void cell_update_main_loop(){
    std::thread renderize_thread(cell_triagulize_main_loop);
    renderize_thread.detach();
    
    FrameRateControl cell_automata_update_count(1000.0);
    FrameRateControl update_speed_output_count(1.0);
    FrameRateControl cube_update_count(20.0);

    int num_cube_updates = 0;
    vector<QuantityInfo> all_cubes_vec = create_quantity_data_vec();
    vector<QuantityInfo> update_tmp_vec = all_cubes_vec;
    QuantityInfo * all_cubes = all_cubes_vec.data();
    QuantityInfo * update_tmp = update_tmp_vec.data();

    while(true){
        if(update_speed_output_count.should_render()){
            double duration_since_render = update_speed_output_count.duration_since_render();
            update_speed_output_count.rendered();
            cout << "frames per second = " << num_cube_updates / duration_since_render << endl;
            num_cube_updates = 0;
        }
        if(true || cell_automata_update_count.should_render()){
            cell_automata_update_count.rendered();

            visit_all_coords([&](CubeCoord base_coord){
                update_coord_quantity(all_cubes,update_tmp,base_coord);
            });

            swap(update_tmp,all_cubes);
            ++num_cube_updates;
        }
        if(cube_update_count.should_render()){
            cube_update_count.rendered();
            cube_shared_data.write(all_cubes_vec);
        }
        if(!cube_update_count.should_render() &&
                !cell_automata_update_count.should_render() &&
                !update_speed_output_count.should_render()){
        }
    }
}
//#define RUN_NO_GRAPHICS
#ifdef RUN_NO_GRAPHICS
int main(){
    cell_update_main_loop();
}
#endif
