#include "opencl_executor.h"
#include <vector>
#include <iostream>
using namespace  std;
#include "cell_update_main.h"
#include "update.h"
#include "display_ops.h"

RenderBufferData all_buffer_data;

inline int int_pow3(int x){
    return x * x * x;
}
void cell_update_main_loop(){
    int all_cube_size = int_pow3(size_cube+2);
    OpenCLExecutor executor("update_cube.cl");
    CLBuffer<QuantityInfo> all_cubes_buf = executor.new_clbuffer<QuantityInfo>(all_cube_size);
    CLBuffer<QuantityInfo> update_buf = executor.new_clbuffer<QuantityInfo>(all_cube_size);
    CLKernel update_kern = executor.new_clkernel("set_quantities",cl::NDRange(size_cube,size_cube,size_cube),{all_cubes_buf.k_arg(),update_buf.k_arg()});

    FrameRateControl cell_automata_update_count(1000.0);
    FrameRateControl update_speed_output_count(1.0);
    FrameRateControl cube_update_count(10.0);

    vector<QuantityInfo> cpu_buf = create_data_vec();
    all_cubes_buf.write_buffer(cpu_buf);
    //update_buf.write_buffer(cpu_buf);
    cout << get(cpu_buf.data(),CubeCoord{3,1,7})->liquid_mass << endl;

    int num_cube_updates = 0;

    while(true){
        //cout << "arg!" << endl;
        if(cube_update_count.should_render()){
            cube_update_count.rendered();
            vector<QuantityInfo> cube_data = all_cubes_buf.read_buffer();
            vector<FaceDrawInfo> draw_info = get_exposed_faces(cube_data.data());
            vector<BYTE> cube_colors;
            cout << get(cube_data.data(),CubeCoord{3,1,7})->liquid_mass << endl;
            vector<float> cube_verticies;
            for(FaceDrawInfo & info : draw_info){
                info.add_to_buffer(cube_colors,cube_verticies);
            }
            all_buffer_data.set_vals(cube_colors,cube_verticies);
        }
        if(update_speed_output_count.should_render()){
            double duration_since_render = update_speed_output_count.duration_since_render();
            update_speed_output_count.rendered();
            cout << "frames per second = " << num_cube_updates / duration_since_render << endl;
            num_cube_updates = 0;
        }
        if(true || cell_automata_update_count.should_render()){
            cell_automata_update_count.rendered();
            update_kern.run();
            all_cubes_buf.copy_buffer(update_buf);
            ++num_cube_updates;
        }
        if(!cube_update_count.should_render() &&
                !cell_automata_update_count.should_render() &&
                !update_speed_output_count.should_render()){
            cell_automata_update_count.render_pause();
        }
    }
}
int main(){
    cell_update_main_loop();
}
