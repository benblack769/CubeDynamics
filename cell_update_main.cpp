#include "cell_update_main.h"
#include "update.h"
#include "display_ops.h"

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
void update(QuantityInfo * source_data, QuantityInfo * update_data){
    visit_all_coords([&](CubeCoord base_coord){
        update_coords(source_data,update_data,base_coord.x,base_coord.y,base_coord.z);
    });
}

void cell_update_main_loop(){
    FrameRateControl cell_automata_update_count(1000.0);
    FrameRateControl update_speed_output_count(1.0);
    FrameRateControl cube_update_count(20.0);

    int num_cube_updates = 0;
    QuantityInfo * all_cubes = create_data();
    QuantityInfo * update_tmp = create_data();

    while(true){
        cout << "arg!" << endl;
        if(update_speed_output_count.should_render()){
            double duration_since_render = update_speed_output_count.duration_since_render();
            update_speed_output_count.rendered();
            cout << "frames per second = " << num_cube_updates / duration_since_render << endl;
            num_cube_updates = 0;
        }
        if(cell_automata_update_count.should_render()){
            cell_automata_update_count.rendered();
            update(all_cubes,update_tmp);
            swap(update_tmp,all_cubes);
            ++num_cube_updates;
        }
        if(cube_update_count.should_render()){
            cube_update_count.rendered();
            vector<FaceDrawInfo> draw_info = get_exposed_faces(all_cubes);
            vector<BYTE> cube_colors;
            vector<float> cube_verticies;
            for(FaceDrawInfo & info : draw_info){
                info.add_to_buffer(cube_colors,cube_verticies);
            }
            all_buffer_data.set_vals(cube_colors,cube_verticies);
        }
        if(!cube_update_count.should_render() &&
                !cell_automata_update_count.should_render() &&
                !update_speed_output_count.should_render()){
        }
    }
}
#ifdef RUN_NO_GRAPHICS
int main(){
    cell_update_main_loop();
}
#endif
