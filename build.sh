#g++ -O3 -o bin/cl.exe -I "external\glm-0.9.7.1" -I "external\opencl-2.2" opencl_cell_update_main.cpp "C:/Windows/System32/OpenCL.dll"
#g++ -O3 -o bin/mygl.exe -I"external/glew-1.13.0/include" -I "external/glfw-3.1.2/include" -I "external\glm-0.9.7.1" -I "external\opencl-2.2" main.cpp cube_coords.cpp update.cpp display_ops.cpp  "C:/Windows/System32/OpenCL.dll" -L./lib64 -lglfw3 -lglew32 -lopengl32 -lgdi32
g++ -O3 -o bin/mygl.exe -I"external/glew-1.13.0/include" -I "external/glfw-3.1.2/include" -I "external/glm-0.9.7.1" -I "external/opencl-2.2" main.cpp cube_coords.cpp update.cpp display_ops.cpp triangularize.cpp update_all.cpp opencl_executor.cpp "C:/Windows/System32/OpenCL.dll"  -L./lib64 -lglfw3 -lglew32 -lopengl32 -lgdi32
#-DOPENCL_ACCEL 
#g++ -O3 -D NO_GRAPHICS -o bin/cl.exe -I "external\glm-0.9.7.1" -I "external\opencl-2.2" opencl_cell_update_main.cpp cube_coords.cpp update.cpp display_ops.cpp  "C:/Windows/System32/OpenCL.dll"
