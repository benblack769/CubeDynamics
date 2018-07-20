g++ -O3 -pthread -DNO_GRAPHICS -o bin/mygl.exe -I "external/opencl-2.2" opencl_cell_update_main.cpp cube_coords.cpp update.cpp display_ops.cpp "/usr/lib64/libOpenCL.so.1" -lglfw -lGLEW -lGL  

