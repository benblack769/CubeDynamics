#include "opencl_executor.h"
#include <vector>
#include <iostream>
#include <mutex>
#include <algorithm>
#include <thread>

#include "cell_update_main.h"
#include "update.h"
#include "display_ops.h"
using namespace  std;

RenderBufferData all_buffer_data;

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
void concat_files(string outfilename,string filename1, string filename2){
    std::ifstream file1( filename1 ) ;
    std::ifstream file2( filename2 ) ;
    std::ofstream combined_file( outfilename ) ;
    combined_file << file1.rdbuf() << file2.rdbuf() ;
}
void cell_update_main_loop(){
    int all_cube_size = data_size();
    vector<QuantityInfo> quant_cpu_buf = create_quantity_data_vec();
    vector<float> bonds_cpu_buf = create_bond_vec(quant_cpu_buf.data());

    cube_shared_data.write(quant_cpu_buf);

    std::thread renderize_thread(cell_triagulize_main_loop);
    renderize_thread.detach();

    concat_files("full_cl.cl","parameters.h","opencl_ops3.cl");
    OpenCLExecutor executor("full_cl.cl");
    CLBuffer<QuantityInfo> all_quant_buf = executor.new_clbuffer<QuantityInfo>(all_cube_size);
    CLBuffer<QuantityInfo> update_quant_buf = executor.new_clbuffer<QuantityInfo>(all_cube_size);

    CLBuffer<float> all_bonds_buf = executor.new_clbuffer<float>(all_cube_size*NUM_BONDS_PER_CUBE);
    CLBuffer<float> update_bonds_buf = executor.new_clbuffer<float>(all_cube_size*NUM_BONDS_PER_CUBE);

    CLBuffer<float> exchange_buf = executor.new_clbuffer<float>(all_cube_size*EXCHANGE_LEN);

    CLKernel update_quant_kern = executor.new_clkernel(
                "update_coord_quantity",
                CL_NDRange(size_cube,size_cube,size_cube),
                {all_quant_buf.k_arg(),all_bonds_buf.k_arg(),update_quant_buf.k_arg(),exchange_buf.k_arg()});

    CLKernel update_bond_kern = executor.new_clkernel(
                "update_bonds",
                CL_NDRange(size_cube,size_cube,size_cube),
                {all_quant_buf.k_arg(),update_quant_buf.k_arg(),all_bonds_buf.k_arg(),exchange_buf.k_arg(),update_bonds_buf.k_arg()});

    FrameRateControl cell_automata_update_count(1000.0);
    FrameRateControl update_speed_output_count(1.0);
    FrameRateControl cube_update_count(20.0);

    all_quant_buf.write_buffer(quant_cpu_buf);
    update_quant_buf.write_buffer(quant_cpu_buf);

    all_bonds_buf.write_buffer(bonds_cpu_buf);
    update_bonds_buf.write_buffer(bonds_cpu_buf);

    int num_cube_updates = 0;

    while(true){
        //cout << "arg!" << endl;
        if(cube_update_count.should_render()){
            cube_update_count.rendered();
            all_quant_buf.read_buffer(quant_cpu_buf);
            cube_shared_data.write(quant_cpu_buf);
        }
        if(update_speed_output_count.should_render()){
            double duration_since_render = update_speed_output_count.duration_since_render();
            update_speed_output_count.rendered();
            cout << "frames per second = " << num_cube_updates / duration_since_render << endl;
            num_cube_updates = 0;
        }
        if(cell_automata_update_count.should_render()){
            cell_automata_update_count.rendered();
            update_quant_kern.run();
            update_bond_kern.run();
            all_quant_buf.copy_buffer(update_quant_buf);
            all_bonds_buf.copy_buffer(update_bonds_buf);

            all_bonds_buf.read_buffer(bonds_cpu_buf);
            cout << "max bond str: " << *max_element(bonds_cpu_buf.begin(),bonds_cpu_buf.end()) << endl;

            ++num_cube_updates;
        }
        if(!cube_update_count.should_render() &&
                !cell_automata_update_count.should_render() &&
                !update_speed_output_count.should_render()){
            cell_automata_update_count.spin_sleep();
        }
    }
}
//#define NO_GRAPHICS
#ifdef NO_GRAPHICS
int main(){
    cell_update_main_loop();
}
#endif
