#include "triangularize.h"
#include "update_all.h"
#include <thread>

using namespace std;

void cell_update_main_loop(){
    RenderBufferData buffer_data;
    CubeSharedData cube_data(data_size());

    std::thread renderize_thread(triangularize_continuously,&cube_data,&buffer_data);
    renderize_thread.detach();

    update_data_continuously(&cube_data);
}
int main(){
    cell_update_main_loop();
}
