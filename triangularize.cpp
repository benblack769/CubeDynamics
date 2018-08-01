#include "triangularize.h"
#include "framerate_control.h"
#include "quantity_info.h"
#include "cube_coords.h"
#include "display_ops.h"
#include "update_all.h"

using namespace std;

bool RenderBufferData::update_check(){
    if(has_written_lock.try_lock()){
        if(has_written){
            has_written = false;

            cube_colors.swap(write_cube_colors);
            cube_verticies.swap(write_cube_verticies);
        }
        has_written_lock.unlock();
        return true;
    }
    else{
        return false;
    }
}
void RenderBufferData::set_vals(std::vector<BYTE> & colors,std::vector<float> & verticies){
    if(has_written_lock.try_lock()){
        has_written = false;
        has_written_lock.unlock();
    }
    write_cube_colors = colors;
    write_cube_verticies = verticies;
    has_written_lock.lock();
    has_written = true;
    has_written_lock.unlock();
}
void triangularize_continuously(CubeSharedData * in_data,RenderBufferData * out_data){
    FrameRateControl cube_update_count(10.0);

    vector<QuantityInfo> cube_buf = create_quantity_data_vec();
    while(true){
        if(cube_update_count.should_render()){
            cube_update_count.rendered();
            in_data->read_into(cube_buf);
            vector<FaceDrawInfo> draw_info = get_exposed_faces(cube_buf.data());
            vector<BYTE> cube_colors;
            cout << get(cube_buf.data(),CubeCoord{3,1,7})->liquid_mass << endl;
            vector<float> cube_verticies;
            for(FaceDrawInfo & info : draw_info){
                info.add_to_buffer(cube_colors,cube_verticies);
            }
            out_data->set_vals(cube_colors,cube_verticies);
        }
        if(!cube_update_count.should_render()){
            cube_update_count.spin_sleep();
        }
    }
}
