// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>

// Include GLEW

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "glm/gtx/string_cast.hpp"
using namespace glm;

#include <stdio.h>
#include <string>
#include <thread>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include "cube_coords.h"
using namespace std;

#include <stdlib.h>
#include <string.h>
#include <chrono>
#include "cube_data.h"

constexpr int X_WIN_SIZE = 1024;
constexpr int Y_WIN_SIZE = 768;

GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path);
void setup_window();
class FrameRateControl{
    double desired_framerate;
    std::chrono::system_clock::time_point prev_time;
public:
    FrameRateControl(double in_desired_framerate){
        desired_framerate = in_desired_framerate;
        prev_time = chrono::system_clock::now();
    }
    void render_pause(){
        while(!this->should_render()){
            this->spin_sleep();
        }
        this->rendered();
    }
    void spin_sleep(){
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    void rendered(){
        prev_time = chrono::system_clock::now();
    }
    bool should_render(){
        using namespace chrono;
        system_clock::time_point cur_time = system_clock::now();

        auto duration = duration_cast<std::chrono::milliseconds>(cur_time - prev_time);

        const int MIL_PER_SEC = 1000;

        return duration.count() > MIL_PER_SEC/desired_framerate;
    }
};

struct CameraPosition{
    glm::vec3 pos;
    glm::vec3 zdir;
    CameraPosition(glm::vec3 init_pos, glm::vec3 init_dir):
        pos(init_pos),
        zdir(glm::normalize(init_dir)){}
    glm::mat4 veiw_mat(){
        zdir = glm::normalize(zdir);
        glm::mat4 View       = glm::lookAt(
                                    pos, // Camera is at (4,3,-3), in World Space
                                    pos+zdir, // and looks at the origin
                                    glm::normalize(glm::vec3(0,1,0))   // Head is up (set to 0,-1,0 to look upside-down)
                               );
        cout << glm::to_string(View) << endl;
        return View;
    }
    void update_dir(float xoffset, float yoffset){
        mat4 model_to_camera = veiw_mat();
        mat4 camera_to_model = glm::inverse(model_to_camera);
        vec4 new_camera_vec = (model_to_camera * vec4(zdir,0)) + vec4(xoffset,yoffset,0,0);
        vec4 new_z_dir = camera_to_model*new_camera_vec;
        vec3 new_dir(new_z_dir.x,new_z_dir.y,new_z_dir.z);
        zdir = glm::normalize(new_dir);
    }
    void move_pos_in_dir(float amt){
        pos += zdir * amt;
    }
    void move_right(float amt){
        mat4 model_to_camera = veiw_mat();
        mat4 camera_to_model = glm::inverse(model_to_camera);
        vec4 add_vec = (camera_to_model*vec4(1,0,0,0)) * amt;
        pos += vec3(add_vec.x,add_vec.y,add_vec.z);
    }
};
void move_cursor(CameraPosition & camera_pos){
    static double lastTime = glfwGetTime();
    double currentTime = glfwGetTime();
    double time_delta = currentTime - lastTime;
    lastTime = currentTime;

    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    //cout << xpos << "\t" << ypos <<  endl;
    float xmov = xpos/(X_WIN_SIZE/2) - 1.0;
    float ymov = -(ypos/(Y_WIN_SIZE/2) - 1.0);

    float MouseSpeed = 10.0f;
    float moveSpeed = 0.3;

    xmov *= MouseSpeed * time_delta;
    ymov *= MouseSpeed * time_delta;

    camera_pos.update_dir(xmov,ymov);

    // Move forward
    if (glfwGetKey( window, GLFW_KEY_UP ) == GLFW_PRESS){
        camera_pos.move_pos_in_dir(moveSpeed);
    }
    // Move backward
    if (glfwGetKey( window, GLFW_KEY_DOWN ) == GLFW_PRESS){
        camera_pos.move_pos_in_dir(-moveSpeed);
    }
    // Strafe right
    if (glfwGetKey( window, GLFW_KEY_RIGHT ) == GLFW_PRESS){
        camera_pos.move_right(moveSpeed);
    }
    // Strafe left
    if (glfwGetKey( window, GLFW_KEY_LEFT ) == GLFW_PRESS){
        camera_pos.move_right(-moveSpeed);
    }

    // Reset mouse position for next frame
    glfwSetCursorPos(window, X_WIN_SIZE/2, Y_WIN_SIZE/2);
}

int main( void )
{
    setup_window();
    //glDepthFunc(GL_LESS);
    // Dark blue background

    glfwPollEvents();
    glfwSetCursorPos(window, X_WIN_SIZE/2, Y_WIN_SIZE/2);

    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);


    // Create and compile our GLSL program from the shaders
    GLuint programID = LoadShaders( "SimpleVertexShader.vertexshader", "SimpleFragmentShader.fragmentshader" );

    // Get a handle for our buffers
    GLuint vertexPosition_modelspaceID = glGetAttribLocation(programID, "vertexPosition_modelspace");

	GLuint MatrixID = glGetUniformLocation(programID, "MVP_S");
	GLuint vertexColorID = glGetAttribLocation(programID, "vertexColor");

	// Projection matrix : 45� Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
    glm::mat4 Projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 300.0f);
    // Camera matrix
    // Model matrix : an identity matrix (model will be at the origin)
	glm::mat4 Model      = glm::mat4(1.0f);

    CameraPosition camera_pos(glm::vec3(50,20,-30),glm::vec3(-1,0,0));


    CubeData all_cubes;

    GLuint vertexbuffer;
    glGenBuffers(1, &vertexbuffer);

	GLuint colorbuffer;
	glGenBuffers(1, &colorbuffer);

    vector<BYTE> cube_colors;
    vector<float> cube_verticies;

    FrameRateControl basic_frame_count(80.0);
    FrameRateControl cube_update_count(1.0);
    FrameRateControl cell_automata_update_count(2.0);
    int frame_num = 0;
    do{
        //sleeps when frame was recently rendered to prevent spinning
       // basic_frame_count.render_pause();

        if(cell_automata_update_count.should_render()){
            cell_automata_update_count.rendered();
            all_cubes.update();
        }
        if(cube_update_count.should_render()){
            cube_update_count.rendered();

            vector<FaceDrawInfo> draw_info = all_cubes.get_exposed_faces();
            cube_colors.clear();
            cube_verticies.clear();
            for(FaceDrawInfo & info : draw_info){
                info.add_to_buffer(cube_colors,cube_verticies);
            }
            cout << "frame drawn" << ++frame_num << endl;

            glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
            glBufferData(GL_ARRAY_BUFFER, sizeof(float)*cube_verticies.size(), cube_verticies.data(), GL_STATIC_DRAW);

            glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
            glBufferData(GL_ARRAY_BUFFER, sizeof(BYTE)*cube_colors.size(), cube_colors.data(), GL_STATIC_DRAW);
        }
        if(basic_frame_count.should_render()){
            basic_frame_count.rendered();
            move_cursor(camera_pos);
            // Clear the screen
            glClear( GL_COLOR_BUFFER_BIT );

            // enables depth buffer correctly.
            glClear(GL_DEPTH_BUFFER_BIT);
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);

            // Use our shader
            glUseProgram(programID);

            glm::mat4 MVP      = Projection * camera_pos.veiw_mat() * Model; // Remember, matrix multiplication is the other way around
            //cout << glm::to_string(MVP) << endl;

            glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
            // 1rst attribute buffer : vertices
            glEnableVertexAttribArray(vertexPosition_modelspaceID);
            glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
            glVertexAttribPointer(
                vertexPosition_modelspaceID, // The attribute we want to configure
                3,                  // size
                GL_FLOAT,           // type
                GL_FALSE,           // normalized?
                0,                  // stride
                (void*)0            // array buffer offset
            );

            // 2nd attribute buffer : colors
            glEnableVertexAttribArray(vertexColorID);
            glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
            glVertexAttribPointer(
                vertexColorID,               // The attribute we want to configure
                4,                           // size
                GL_UNSIGNED_BYTE,                    // type
                GL_TRUE,                    // normalized?
                0,                           // stride
                (void*)0                     // array buffer offset
            );

            // Draw the triangle !
            glDrawArrays(GL_TRIANGLES, 0, cube_verticies.size()/3); // 3 indices starting at 0 -> 1 triangle

            glDisableVertexAttribArray(vertexPosition_modelspaceID);
            glDisableVertexAttribArray(vertexColorID);

            // Swap buffers
            glfwSwapBuffers(window);
            glfwPollEvents();

        }
        vector<float> cube_verticies;

        if(!basic_frame_count.should_render() &&
                !cube_update_count.should_render() &&
                !cell_automata_update_count.should_render()){
            basic_frame_count.spin_sleep();
        }

    } // Check if the ESC key was pressed or the window was closed
    while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0);


    // Cleanup VBO
    glDeleteProgram(programID);
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &colorbuffer);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}


GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path){

    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    // Read the Vertex Shader code from the file
    std::string VertexShaderCode;
    std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
    if(VertexShaderStream.is_open()){
        std::string Line = "";
        while(getline(VertexShaderStream, Line))
            VertexShaderCode += "\n" + Line;
        VertexShaderStream.close();
    }else{
        printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_file_path);
        getchar();
        return 0;
    }

    // Read the Fragment Shader code from the file
    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
    if(FragmentShaderStream.is_open()){
        std::string Line = "";
        while(getline(FragmentShaderStream, Line))
            FragmentShaderCode += "\n" + Line;
        FragmentShaderStream.close();
    }

    GLint Result = GL_FALSE;
    int InfoLogLength;


    // Compile Vertex Shader
    printf("Compiling shader : %s\n", vertex_file_path);
    char const * VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
    glCompileShader(VertexShaderID);

    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
        glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
        printf("%s\n", &VertexShaderErrorMessage[0]);
    }



    // Compile Fragment Shader
    printf("Compiling shader : %s\n", fragment_file_path);
    char const * FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
    glCompileShader(FragmentShaderID);

    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
        glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
        printf("%s\n", &FragmentShaderErrorMessage[0]);
    }



    // Link the program
    printf("Linking program\n");
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);

    // Check the program
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        std::vector<char> ProgramErrorMessage(InfoLogLength+1);
        glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
        printf("%s\n", &ProgramErrorMessage[0]);
    }


    glDetachShader(ProgramID, VertexShaderID);
    glDetachShader(ProgramID, FragmentShaderID);

    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

    return ProgramID;
}

void setup_window(){
    // Initialise GLFW
    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        getchar();
        exit(-1);
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);


    // Open a window and create its OpenGL context
    window = glfwCreateWindow( X_WIN_SIZE, Y_WIN_SIZE, "Tutorial 02 - Red triangle", NULL, NULL);
    if( window == NULL ){
        fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
        getchar();
        glfwTerminate();
        exit(-1);
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        exit(-1);
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
}
