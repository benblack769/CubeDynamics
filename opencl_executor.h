#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <cassert>

#include <cl.hpp>

class OpenCLExecutor;
class CLKernel{
protected:
    cl::CommandQueue myqueue;
    cl::Program prog;
    cl::Kernel kern;
    cl::NDRange run_range;
public:
    friend class OpenCLExecutor;
    CLKernel(cl::Program in_prog,cl::CommandQueue in_queue,const char * kern_name,cl::NDRange in_run_range,std::vector<cl::Buffer> args):
        myqueue(in_queue),
        prog(in_prog),
        kern(in_prog,kern_name),
        run_range(in_run_range)
    {
        int idx = 0;
        for(cl::Buffer buf : args){
            this->kern.setArg(idx,buf);
            idx++;
        }
    }
    void run(){
        myqueue.enqueueBarrierWithWaitList();
        myqueue.enqueueNDRangeKernel(kern,cl::NullRange,run_range,cl::NullRange);
    }
};

template<typename item_ty>
class CLBuffer{
protected:
    int bufsize;
    cl::Buffer buf;
    cl::Context mycontext;
    cl::CommandQueue myqueue;
public:
    CLBuffer(cl::Context context,cl::CommandQueue queue,size_t size):
        buf(context,CL_MEM_READ_WRITE,sizeof(item_ty)*size),
        bufsize(size),
        mycontext(context),
        myqueue(queue){}

    void write_buffer(std::vector<item_ty>& data){
        assert(data.size() == bufsize);
        myqueue.enqueueWriteBuffer(buf,CL_TRUE,0,data.size()*sizeof(item_ty),data.data());
    }
    std::vector<item_ty> read_buffer(){
        std::vector<item_ty> res(bufsize);
        myqueue.enqueueReadBuffer(buf,CL_TRUE,0,res.size()*sizeof(item_ty),res.data());
        return res;
    }
    cl::Buffer & k_arg(){
        return buf;
    }
    void copy_buffer(CLBuffer<item_ty> src_buf){
        assert(src_buf.bufsize == this->bufsize);
        myqueue.enqueueBarrierWithWaitList();
        myqueue.enqueueCopyBuffer(src_buf.buf,this->buf,0,0,bufsize*sizeof(item_ty));
    }
};
class OpenCLExecutor{
protected:
    std::string source_path;
    cl::Platform platform;
    cl::Device device;
    cl::Context context;
    cl::Program::Sources sources;
    cl::Program program;
    cl::CommandQueue queue;
public:

    OpenCLExecutor(std::string in_source_path)
    {
        source_path = in_source_path;
        build_program();
    }
    template<typename item_ty>
    CLBuffer<item_ty> new_clbuffer(size_t size){
        return CLBuffer<item_ty>(context,queue,size);
    }
    CLKernel new_clkernel(const char * kern_name,cl::NDRange run_range,std::vector<cl::Buffer> buflist){
        return CLKernel(program,queue,kern_name,run_range,buflist);
    }

protected:
    void get_main_device(){
        std::vector<cl::Platform> all_platforms;
        cl::Platform::get(&all_platforms);
        if(all_platforms.size()==0){
            std::cout<<" No platforms found. Check OpenCL installation!\n";
            exit(1);
        }
        platform=all_platforms[0];
        std::cout << "Using platform: "<<platform.getInfo<CL_PLATFORM_NAME>()<<"\n";

        //get default device of the default platform
        std::vector<cl::Device> all_devices;
        //todo: change CL_DEVICE_TYPE_ALL to something that specifies GPU or accelerator
        cl_int err = platform.getDevices(CL_DEVICE_TYPE_ALL, &all_devices);
        if(err){
            std::cout<<" No devices found. Check OpenCL installation!\n";
            exit(1);
        }
        device=all_devices[0];
        std::cout<< "Using device: "<<device.getInfo<CL_DEVICE_NAME>()<<"\n";
    }
    void build_program(){
        get_main_device();
        context = cl::Context({device});

        queue = cl::CommandQueue(context,device);

        BuildSource();

        program = cl::Program(context,sources);
        if(program.build({device})!=CL_SUCCESS){
            std::cout<<" Error building: "<<program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device)<<"\n";
            exit(1);
        }
    }

    void BuildSource(){
        std::ifstream file(source_path);
        if(!file){
            std::cout << "the file " << source_path << " is missing!\n";
            exit(1);
        }
        //slow way to read a file (but file size is small)
        std::string fstr((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

        file.close();

        sources.clear();
        sources.push_back(std::make_pair(fstr.c_str(),fstr.length()));
    }

    std::string get_error_string(cl_int err){
         switch(err){
             case 0: return "CL_SUCCESS";
             case -1: return "CL_DEVICE_NOT_FOUND";
             case -2: return "CL_DEVICE_NOT_AVAILABLE";
             case -3: return "CL_COMPILER_NOT_AVAILABLE";
             case -4: return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
             case -5: return "CL_OUT_OF_RESOURCES";
             case -6: return "CL_OUT_OF_HOST_MEMORY";
             case -7: return "CL_PROFILING_INFO_NOT_AVAILABLE";
             case -8: return "CL_MEM_COPY_OVERLAP";
             case -9: return "CL_IMAGE_FORMAT_MISMATCH";
             case -10: return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
             case -11: return "CL_BUILD_PROGRAM_FAILURE";
             case -12: return "CL_MAP_FAILURE";

             case -30: return "CL_INVALID_VALUE";
             case -31: return "CL_INVALID_DEVICE_TYPE";
             case -32: return "CL_INVALID_PLATFORM";
             case -33: return "CL_INVALID_DEVICE";
             case -34: return "CL_INVALID_CONTEXT";
             case -35: return "CL_INVALID_QUEUE_PROPERTIES";
             case -36: return "CL_INVALID_COMMAND_QUEUE";
             case -37: return "CL_INVALID_HOST_PTR";
             case -38: return "CL_INVALID_MEM_OBJECT";
             case -39: return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
             case -40: return "CL_INVALID_IMAGE_SIZE";
             case -41: return "CL_INVALID_SAMPLER";
             case -42: return "CL_INVALID_BINARY";
             case -43: return "CL_INVALID_BUILD_OPTIONS";
             case -44: return "CL_INVALID_PROGRAM";
             case -45: return "CL_INVALID_PROGRAM_EXECUTABLE";
             case -46: return "CL_INVALID_KERNEL_NAME";
             case -47: return "CL_INVALID_KERNEL_DEFINITION";
             case -48: return "CL_INVALID_KERNEL";
             case -49: return "CL_INVALID_ARG_INDEX";
             case -50: return "CL_INVALID_ARG_VALUE";
             case -51: return "CL_INVALID_ARG_SIZE";
             case -52: return "CL_INVALID_KERNEL_ARGS";
             case -53: return "CL_INVALID_WORK_DIMENSION";
             case -54: return "CL_INVALID_WORK_GROUP_SIZE";
             case -55: return "CL_INVALID_WORK_ITEM_SIZE";
             case -56: return "CL_INVALID_GLOBAL_OFFSET";
             case -57: return "CL_INVALID_EVENT_WAIT_LIST";
             case -58: return "CL_INVALID_EVENT";
             case -59: return "CL_INVALID_OPERATION";
             case -60: return "CL_INVALID_GL_OBJECT";
             case -61: return "CL_INVALID_BUFFER_SIZE";
             case -62: return "CL_INVALID_MIP_LEVEL";
             case -63: return "CL_INVALID_GLOBAL_WORK_SIZE";
             default: return "Unknown OpenCL error";
         }
     }
     void handle_error(cl_int err){
        if (err){
            std::cout << "Error: " << get_error_string(err) << "\n";
        }
    }
};
