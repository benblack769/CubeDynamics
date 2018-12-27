#include "opencl_executor.h"
#include <vector>
#include <iostream>
#include <mutex>
#include <thread>

#include "cell_update_main.h"
#include "update.h"
#include "display_ops.h"


RenderBufferData all_buffer_data;

inline int int_pow3(int x){
    return x * x * x;
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
} cube_shared_data(int_pow3(size_cube+2));

void cell_triagulize_main_loop(){
    FrameRateControl cube_update_count(10.0);

    vector<QuantityInfo> cube_buf = create_data_vec();
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
    int all_cube_size = int_pow3(size_cube+2);
    int all_fold_size = int_pow3(NUM_FOLDS+2);
    vector<QuantityInfo> cpu_buf = create_data_vec();
    cube_shared_data.write(cpu_buf);

    std::thread renderize_thread(cell_triagulize_main_loop);
    renderize_thread.detach();

    concat_files("full_cl.cl","parameters.h","opencl_ops2.cl");
    OpenCLExecutor executor("full_cl.cl");
    CLBuffer<QuantityInfo> all_cubes_buf = executor.new_clbuffer<QuantityInfo>(all_cube_size);
    CLBuffer<QuantityInfo> update_buf = executor.new_clbuffer<QuantityInfo>(all_cube_size);
    CLBuffer<bool> should_update_buf = executor.new_clbuffer<bool>(all_cube_size);
    CLBuffer<bool> should_update_folds_buf = executor.new_clbuffer<bool>(all_fold_size);
    CLBuffer<bool> should_update_folds_update_buf = executor.new_clbuffer<bool>(all_fold_size);
    CLKernel update_kern = executor.new_clkernel("update_coords",CL_NDRange(size_cube,size_cube,size_cube),{all_cubes_buf.k_arg(),update_buf.k_arg(),should_update_folds_buf.k_arg()});
    CLKernel calc_should_update_kern = executor.new_clkernel("calc_should_update",CL_NDRange(size_cube,size_cube,size_cube),{all_cubes_buf.k_arg(),should_update_buf.k_arg()});
    CLKernel fold_should_update_kern = executor.new_clkernel("reduce_should_update",CL_NDRange(NUM_FOLDS,NUM_FOLDS,NUM_FOLDS),{should_update_buf.k_arg(),should_update_folds_buf.k_arg()});
    CLKernel calc_should_update_large_kern = executor.new_clkernel("calc_should_update_large",CL_NDRange(NUM_FOLDS,NUM_FOLDS,NUM_FOLDS),{should_update_folds_buf.k_arg(),should_update_folds_update_buf.k_arg()});
    CLKernel zero_fold_array_kern = executor.new_clkernel("zero_fold_array",CL_NDRange(all_fold_size),{should_update_folds_buf.k_arg()});

    FrameRateControl cell_automata_update_count(1000.0);
    FrameRateControl update_speed_output_count(1.0);
    FrameRateControl cube_update_count(20.0);

    all_cubes_buf.write_buffer(cpu_buf);
    update_buf.write_buffer(cpu_buf);

    int num_cube_updates = 0;

    while(true){
        //cout << "arg!" << endl;
        if(cube_update_count.should_render()){
            cube_update_count.rendered();
            all_cubes_buf.read_buffer(cpu_buf);
            cube_shared_data.write(cpu_buf);
        }
        if(update_speed_output_count.should_render()){
            double duration_since_render = update_speed_output_count.duration_since_render();
            update_speed_output_count.rendered();
            cout << "frames per second = " << num_cube_updates / duration_since_render << endl;
            num_cube_updates = 0;
        }
        if(cell_automata_update_count.should_render()){
            cell_automata_update_count.rendered();
            if(num_cube_updates%SIZE_FOLD == 0){
                calc_should_update_kern.run();
                fold_should_update_kern.run();
                calc_should_update_large_kern.run();
                should_update_folds_buf.copy_buffer(should_update_folds_update_buf);
                calc_should_update_large_kern.run();
                should_update_folds_buf.copy_buffer(should_update_folds_update_buf);
            }
            update_kern.run();
            all_cubes_buf.copy_buffer(update_buf);
            ++num_cube_updates;
        }
        if(!cube_update_count.should_render() &&
                !cell_automata_update_count.should_render() &&
                !update_speed_output_count.should_render()){
            //cell_automata_update_count.spin_sleep();
        }
    }
}
#ifdef NO_GRAPHICS
int main(){
    cell_update_main_loop();
}
#endif
