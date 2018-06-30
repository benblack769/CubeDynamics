cmake_minimum_required(VERSION 3.5)
#include(ExternalProject)

#ExternalProject_Add(glew_lib PREFIX ${CMAKE_SOURCE_DIR} GIT_REPOSITORY "https://github.com/glfw/glfw")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#link_directories(${CMAKE_SOURCE_DIR}/lib64)

#find_library(GLFW3 glfw3 PATHS ${CMAKE_SOURCE_DIR}/lib64)

include_directories(external/glew-1.13.0/include)
include_directories(external/glfw-3.1.2/include)
include_directories(external/glm-0.9.7.1)


add_executable(draw_squares
    main.cpp
    cube_coords.cpp
    display_ops.cpp
    update.cpp
    cell_update_main.cpp
)
target_link_libraries(draw_squares PRIVATE
    glfw3
    glew
    opengl
    gdi
)
