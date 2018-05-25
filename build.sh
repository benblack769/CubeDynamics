g++ -O3 -o bin/mygl.exe -I"external/glew-1.13.0/include" -I "external/glfw-3.1.2/include" -I "external\glm-0.9.7.1" main.cpp cube_coords.cpp -L./lib64 -lglfw3 -lglew32 -lopengl32 -lgdi32
