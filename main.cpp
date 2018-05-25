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



GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path);
void setup_window();
class FrameRateControl{
    const double desired_framerate = 60.0;
    std::chrono::system_clock::time_point prev_time;
public:
    FrameRateControl(){

    }
    void render_pause(){
        while(!should_render()){
            std::this_thread::sleep_for(std::chrono::milliseconds(30));
        }
        prev_time = chrono::system_clock::now();
    }
    bool should_render(){
        using namespace chrono;
        system_clock::time_point cur_time = system_clock::now();

        auto duration = duration_cast<std::chrono::milliseconds>(cur_time - prev_time);

        const int MIL_PER_SEC = 1000;

        return duration.count() > desired_framerate/MIL_PER_SEC;
    }
};


/*struct CubeData{
    RGBVal color;
    CubeCoord coord;
};
void add_cube(CubeData cube, std::vector<float> & color_buffer, std::vector<float> & vertex_buffer){
    add_cube_colors(cube.color,color_buffer);
    add_cube_vertex(cube.coord,vertex_buffer);
}*/

int main( void )
{
    setup_window();
    //glDepthFunc(GL_LESS);
    // Dark blue background
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

    // Create and compile our GLSL program from the shaders
    GLuint programID = LoadShaders( "SimpleVertexShader.vertexshader", "SimpleFragmentShader.fragmentshader" );

    // Get a handle for our buffers
    GLuint vertexPosition_modelspaceID = glGetAttribLocation(programID, "vertexPosition_modelspace");

    /*static const GLfloat g_vertex_buffer_data[] = {
        -1.0f,  1.0f, 0.0f,
        -1.0f,  0.9f, 0.0f,
        -0.9f,  1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f,
         1.0f, -1.0f, 0.0f,
         0.0f,  1.0f, 0.0f,
    };*/
    /*for(int i = 0; i < 12; i++){
        for(int j = 0; j < 3; j++){
            for(int k = 0; k < 3; k++){
                cout << cube_verticies[i*9+j*3+k] << "\t";
            }
            cout << endl;
        }
        cout << endl;
    }*/

	GLuint MatrixID = glGetUniformLocation(programID, "MVP_S");
	GLuint vertexColorID = glGetAttribLocation(programID, "vertexColor");

	// Projection matrix : 45� Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	glm::mat4 Projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
	// Camera matrix
	glm::mat4 View       = glm::lookAt(
								glm::vec3(50,20,-30), // Camera is at (4,3,-3), in World Space
								glm::vec3(0,0,0), // and looks at the origin
								glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
						   );
	// Model matrix : an identity matrix (model will be at the origin)
	glm::mat4 Model      = glm::mat4(1.0f);
	// Our ModelViewProjection : multiplication of our 3 matrices
	glm::mat4 MVP        = Projection * View * Model; // Remember, matrix multiplication is the other way around


    CubeData all_cubes;

    GLuint vertexbuffer;
    glGenBuffers(1, &vertexbuffer);

	GLuint colorbuffer;
	glGenBuffers(1, &colorbuffer);

    FrameRateControl count;
    do{
        vector<BYTE> cube_colors;
        vector<float> cube_verticies;
        cout << "megama" << endl;

        vector<FaceDrawInfo> draw_info = all_cubes.get_exposed_faces();
        cout << "arlkajsd" << endl;
        for(FaceDrawInfo & info : draw_info){
            info.add_to_buffer(cube_colors,cube_verticies);
        }
        cout << draw_info.size() << endl;
        //cout <<

        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float)*cube_verticies.size(), cube_verticies.data(), GL_STREAM_DRAW);

    	glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
    	glBufferData(GL_ARRAY_BUFFER, sizeof(BYTE)*cube_colors.size(), cube_colors.data(), GL_STREAM_DRAW);

        //sleeps when frame was recently rendered to prevent spinning
        count.render_pause();

        // Clear the screen
        glClear( GL_COLOR_BUFFER_BIT );

        // enables depth buffer correctly.
        glClear(GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        // Use our shader
        glUseProgram(programID);

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
    	glDeleteBuffers(1, &colorbuffer);

    } // Check if the ESC key was pressed or the window was closed
    while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0);


    // Cleanup VBO
    glDeleteProgram(programID);
    glDeleteBuffers(1, &vertexbuffer);

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
    window = glfwCreateWindow( 1024, 768, "Tutorial 02 - Red triangle", NULL, NULL);
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
