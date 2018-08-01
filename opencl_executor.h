#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <cassert>

//#include <cl.hpp>
#define CL_TARGET_OPENCL_VERSION 110
#include <CL/cl.h>
//#include <CL/cl_platform.h>

std::string get_error_string(cl_int err);
void CheckErrorAt(cl_int err,const char * source_info);
cl_int CLEnqueueBarrier(cl_command_queue command_queue);
cl_command_queue CLCreateCommandQueue(cl_context context, cl_device_id device, cl_int * errcode_ret);
#define STR_HELPER(x) #x
#define CONST_STR(x) STR_HELPER(x)
#define CheckError(err) {CheckErrorAt(err,("File: " __FILE__ ", Line: " CONST_STR(__LINE__)));}

template<typename item_ty>
class CLBuffer{
protected:
    int bufsize;
    cl_mem buf;
    cl_context mycontext;
    cl_command_queue myqueue;
public:
    CLBuffer(cl_context context,cl_command_queue queue,size_t size){
        bufsize = size;
        mycontext = context;
        myqueue = queue;

        cl_int error = CL_SUCCESS;
        buf = clCreateBuffer(context,CL_MEM_READ_WRITE,bytes(),nullptr,&error);
        CheckError(error);
    }

    void write_buffer(std::vector<item_ty>& data){
        assert(data.size() == bufsize);
        CheckError(clEnqueueWriteBuffer(myqueue,
                             buf,
                             CL_TRUE,
                             0,bufsize*sizeof(item_ty),
                             data.data(),
                             0,nullptr,
                             nullptr));
    }
    void read_buffer(std::vector<item_ty> & read_into){
        assert(read_into.size() == bufsize);
        CheckError(clEnqueueReadBuffer(myqueue,
                             buf,
                             CL_TRUE,
                             0,bufsize*sizeof(item_ty),
                             read_into.data(),
                             0,nullptr,
                             nullptr));
    }
    cl_mem k_arg(){
        return buf;
    }
    size_t bytes(){
        return bufsize * sizeof(item_ty);
    }
    void copy_buffer(CLBuffer<item_ty> src_buf){
        assert(src_buf.bufsize == this->bufsize);
        assert(src_buf.myqueue == this->myqueue);
        assert(src_buf.mycontext == this->mycontext);
        CheckError(CLEnqueueBarrier(myqueue));
        CheckError(clEnqueueCopyBuffer(myqueue,
                            src_buf.buf,this->buf,
                            0,0,
                            bytes(),
                            0,nullptr,
                            nullptr));
    }
};
class CL_NDRange{
private:
    size_t x;
    size_t y;
    size_t z;
    cl_uint ndim;
public:
    CL_NDRange(size_t in_x,size_t in_y, size_t in_z);
    CL_NDRange(size_t in_x,size_t in_y);
    CL_NDRange(size_t in_x);
    CL_NDRange();
    size_t * array_view();
    cl_uint dim();
};
class CLKernel{
protected:
    cl_command_queue myqueue;
    cl_program program;
    cl_kernel kern;
    CL_NDRange run_range;
    CL_NDRange work_group_range;
public:
    CLKernel(cl_program in_prog,cl_command_queue in_queue,const char * kern_name,CL_NDRange in_run_range,CL_NDRange in_work_group_range,std::vector<cl_mem> args);
    void run();
};


class OpenCLExecutor{
protected:
    std::string source_path;
    cl_platform_id platform;
    cl_device_id device;
    cl_context context;
    cl_program program;
    cl_command_queue queue;
public:

    OpenCLExecutor(std::string in_source_path);
    ~OpenCLExecutor();
    template<typename item_ty>
    CLBuffer<item_ty> new_clbuffer(size_t size){
        return CLBuffer<item_ty>(context,queue,size);
    }
    CLKernel new_clkernel(const char * kern_name,CL_NDRange run_range,CL_NDRange work_range,std::vector<cl_mem> buflist){
        return CLKernel(program,queue,kern_name,run_range,work_range,buflist);
    }

protected:
    void get_main_device();
    void create_context();
    void create_queue();
    void build_program();

    std::string get_source();

    void CreateProgram ();

     std::string GetPlatformName (cl_platform_id id);

     std::string GetDeviceName (cl_device_id id);
};
