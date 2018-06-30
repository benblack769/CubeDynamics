#include "opencl_executor.h"
#include <vector>
#include <iostream>
using namespace  std;

int main(){
    OpenCLExecutor executor("clear.cl");
    
    int clear_size = 1000;
    CLBuffer<float> clear_buf = executor.new_clbuffer<float>(clear_size);
    CLKernel clear_kern = executor.new_clkernel("clear_buffer",cl::NDRange(clear_size),{clear_buf.k_arg()});

    vector<float> buf_to_clear(clear_size,0);
    clear_buf.write_buffer(buf_to_clear);
    clear_kern.run();
    vector<float> res = clear_buf.read_buffer();
    cout << res[10] << endl;
    cout << res[0] << endl;
    cout << res[999] << endl;
}
