#include "update_all.h"
#include "framerate_control.h"
#include "update.h"
#include "display_ops.h"
#include "opencl_executor.h"

using namespace  std;


CubeSharedData::CubeSharedData(size_t size):
    cube_data(size){
}
void CubeSharedData::read_into(std::vector<QuantityInfo> & cube_buffer){
    lock.lock();
    assert(cube_buffer.size() == cube_data.size());
    copy(cube_data.begin(),cube_data.end(),cube_buffer.begin());
    lock.unlock();
}
void CubeSharedData::write(std::vector<QuantityInfo> & buffer){
    lock.lock();
    assert(buffer.size() == cube_data.size());
    copy(buffer.begin(),buffer.end(),cube_data.begin());
    lock.unlock();
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
void update_data_continuously_cpu(CubeSharedData & data_to_update){
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
                update_coord_quantity(all_cubes,update_tmp,base_coord,num_cube_updates);
            });

            swap(update_tmp,all_cubes);
            ++num_cube_updates;
        }
        if(cube_update_count.should_render()){
            cube_update_count.rendered();
            data_to_update.write(all_cubes_vec);
        }
        if(!cube_update_count.should_render() &&
                !cell_automata_update_count.should_render() &&
                !update_speed_output_count.should_render()){
        }
    }
}

void concat_files(string outfilename,string filename1, string filename2){
    std::ifstream file1( filename1 ) ;
    std::ifstream file2( filename2 ) ;
    std::ofstream combined_file( outfilename ) ;
    combined_file << file1.rdbuf() << file2.rdbuf() ;
}
void opencl_accelerated_update_data(CubeSharedData & data_to_update){

    int all_cube_size = data_size();
    vector<QuantityInfo> quant_cpu_buf = create_quantity_data_vec();

    data_to_update.write(quant_cpu_buf);

    concat_files("full_cl.cl","parameters.h","opencl_ops3.cl");
    OpenCLExecutor executor("full_cl.cl");
    CLBuffer<QuantityInfo> all_quant_buf = executor.new_clbuffer<QuantityInfo>(all_cube_size);
    CLBuffer<QuantityInfo> update_quant_buf = executor.new_clbuffer<QuantityInfo>(all_cube_size);

    CLKernel update_quant_kern = executor.new_clkernel(
                "update_coord_quantity",
                CL_NDRange(size_cube,size_cube,size_cube),
                CL_NDRange(),
                {all_quant_buf.k_arg(),update_quant_buf.k_arg()});

    FrameRateControl cell_automata_update_count(1000.0);
    FrameRateControl update_speed_output_count(1.0);
    FrameRateControl cube_update_count(20.0);

    all_quant_buf.write_buffer(quant_cpu_buf);
    update_quant_buf.write_buffer(quant_cpu_buf);

    int num_cube_updates = 0;

    while(true){
        //cout << "arg!" << endl;
        if(cube_update_count.should_render()){
            cube_update_count.rendered();
            all_quant_buf.read_buffer(quant_cpu_buf);
            data_to_update.write(quant_cpu_buf);
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
            all_quant_buf.copy_buffer(update_quant_buf);

            ++num_cube_updates;
        }
        if(!cube_update_count.should_render() &&
                !cell_automata_update_count.should_render() &&
                !update_speed_output_count.should_render()){
            cell_automata_update_count.spin_sleep();
        }
    }
}
//#define OPENCL_ACCEL
void update_data_continuously(CubeSharedData * data_to_update){
#ifdef OPENCL_ACCEL
    opencl_accelerated_update_data(*data_to_update);
#else
    update_data_continuously_cpu(*data_to_update);
#endif
}
