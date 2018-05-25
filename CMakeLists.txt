cmake_minimum_required(VERSION 3.5)
include_directories(external/glew-1.13.0/include)
include_directories(external/glfw-3.1.2/include)
include_directories(external/glm-0.9.7.1)

link_directories(${CMAKE_SOURCE_DIR}/lib64)

add_executable(squares main.cpp)
target_link_libraries(squares
    glfw3
    glew32
    opengl32
    gdi32
)
