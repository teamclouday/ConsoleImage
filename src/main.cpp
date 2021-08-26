#include <GL/glew.h>
#include <argparse/argparse.hpp>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <vector>
#include <iostream>
#include <string>
#include <cmath>
#include <cstdlib>
#include <stdexcept>


int main(int argc, char** argv)
{
    // build argument parser
    argparse::ArgumentParser argument("ConsoleImage");
    // add input image path
    argument.add_argument("filepath")
        .help("image path to display")
        .required();
    // add verbose option
    argument.add_argument("--verbose")
        .help("show processing details")
        .default_value(false)
        .implicit_value(true);
    // add image size
    argument.add_argument("-s", "--size")
        .help("number of lines (size) to display (val > 0)")
        .default_value(20)
        .scan<'i', int>();
    // add selected characters
    argument.add_argument("-c", "--chars")
        .help("characters to use (dark to light)")
        .default_value(std::string(".-*+o&8@"));
    // get arguments
    try
    {
        argument.parse_args(argc, argv);
    }
    catch(const std::runtime_error& err)
    {
        std::cerr << err.what() << std::endl;
        std::cout << argument;
        exit(-1);
    }
    std::string chars = argument.get<std::string>("--chars");
    int levels = chars.length();
    if(levels <= 0)
    {
        std::cerr << "Invalid chars argument: cannot be empty" << std::endl;
        std::cout << argument;
        exit(-1);
    }
    bool verbose = argument.get<bool>("--verbose");
    if(verbose) std::cout << "Arguments parsed" << std::endl;
    
    // load image
    int imgW, imgH, imgC;
    unsigned char* img = stbi_load(argument.get<std::string>("filepath").c_str(), &imgW, &imgH, &imgC, 0);
    if(!img)
    {
        if(verbose) std::cerr << "Failed to load " << argument.get<std::string>("filepath") << std::endl;
        exit(-1);
    }
    if(verbose) std::cout << "Image loaded: " << argument.get<std::string>("filepath") << std::endl;

    // compute output size
    int outputH, outputW;
    outputH = argument.get<int>("--size");
    if(outputH <= 0)
    {
        std::cerr << "Invalid size argument" << std::endl;
        std::cout << argument;
        exit(-1);
    }
    outputW = (int)((float)outputH * ((float)imgW / (float)imgH));

    // init opengl context
    if(!glfwInit())
    {
        if(verbose) std::cerr << "Failed to init glfw" << std::endl;
        exit(-1);
    }
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* context = glfwCreateWindow(outputW, outputH, "ConsoleImage", nullptr, nullptr);
    if(!context)
    {
        if(verbose) std::cerr << "Failed to create GLFW context" << std::endl;
        exit(-1);
    }
    glfwMakeContextCurrent(context);
    if(glewInit() != GLEW_OK)
    {
        if(verbose) std::cerr << "Failed to init OpenGL context" << std::endl;
        exit(-1);
    }
    if(verbose) std::cout << "OpenGL context created" << std::endl;

    // render buffer
    const float vertex[] = {
        -1.0f, -1.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 1.0f, 0.0f,
         1.0f,  1.0f, 1.0f, 1.0f,
        -1.0f,  1.0f, 0.0f, 1.0f,
    };
    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex), vertex, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(0));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // prepare original image texture
    GLuint imgTex;
    glGenTextures(1, &imgTex);
    glBindTexture(GL_TEXTURE_2D, imgTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imgW, imgH, 0, imgC > 3 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, img);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(img);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // prepare shaders
    GLuint shaderVert = glCreateShader(GL_VERTEX_SHADER);
    GLuint shaderFrag = glCreateShader(GL_FRAGMENT_SHADER);
    const char* shaderVertContent =
    #include "render.vert"
    ;
    const char* shaderFragContent =
    #include "render.frag"
    ;
    glShaderSource(shaderVert, 1, &shaderVertContent, nullptr);
    glShaderSource(shaderFrag, 1, &shaderFragContent, nullptr);
    glCompileShader(shaderVert);
    glCompileShader(shaderFrag);
    GLint compileSuccess;
    glGetShaderiv(shaderVert, GL_COMPILE_STATUS, &compileSuccess);
    if(!compileSuccess)
    {
        GLint infoLen;
        glGetShaderiv(shaderVert, GL_INFO_LOG_LENGTH, &infoLen);
        std::vector<GLchar> infoLog(infoLen+1);
        glGetShaderInfoLog(shaderVert, infoLen, nullptr, &infoLog[0]);
        if(verbose) std::cerr << "Vertex shader failed to compile: " << infoLog[0] << std::endl;
        exit(-1);
    }
    glGetShaderiv(shaderFrag, GL_COMPILE_STATUS, &compileSuccess);
    if(!compileSuccess)
    {
        GLint infoLen;
        glGetShaderiv(shaderFrag, GL_INFO_LOG_LENGTH, &infoLen);
        std::vector<GLchar> infoLog(infoLen+1);
        glGetShaderInfoLog(shaderFrag, infoLen, nullptr, &infoLog[0]);
        if(verbose) std::cerr << "Fragment shader failed to compile: " << infoLog[0] << std::endl;
        exit(-1);
    }
    GLuint shader = glCreateProgram();
    glAttachShader(shader, shaderVert);
    glAttachShader(shader, shaderFrag);
    glLinkProgram(shader);
    glGetProgramiv(shader, GL_LINK_STATUS, &compileSuccess);
    if(!compileSuccess)
    {
        GLint infoLen;
        glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
        std::vector<GLchar> infoLog(infoLen+1);
        glGetProgramInfoLog(shader, infoLen, nullptr, &infoLog[0]);
        if(verbose) std::cerr << "Shader program failed to link: " << infoLog[0] << std::endl;
        exit(-1);
    }
    glDeleteShader(shaderVert);
    glDeleteShader(shaderFrag);
    // prepare render buffer and frame buffer
    GLuint FBO;
    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    GLuint FBOtex;
    glGenTextures(1, &FBOtex);
    glBindTexture(GL_TEXTURE_2D, FBOtex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, outputW, outputH, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, FBOtex, 0);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        if(verbose) std::cerr << "Failed to create OpenGL framebuffer" << std::endl;
        exit(-1);
    }
    // ready to render
    glViewport(0, 0, outputW, outputH);
    glUseProgram(shader);
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(glGetUniformLocation(shader, "image"), 0);
    glBindTexture(GL_TEXTURE_2D, imgTex);
    glBindVertexArray(VAO);
    glDrawArrays(GL_QUADS, 0, 4);

    // get data from texture
    std::vector<float> mem(outputW * outputH);
    glBindTexture(GL_TEXTURE_2D, FBOtex);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, mem.data());
    // process data to terminal output
    float step = 1.0f / levels;
    std::vector<char> output(outputW * outputH);
    std::transform(mem.begin(), mem.end(), std::begin(output),
            [chars, step](const float val){
                int idx = std::round(val / step);
                return chars[idx];
            });
    if(verbose) std::cout << "Image processed" << std::endl;

    // output to terminal
    for(int i = 0; i < outputH; i++)
    {
        for(int j = 0; j < outputW; j++)
        {
            std::cout << output[j + i * outputW] << ' ';
            if(j == outputW - 1) std::cout << std::endl;
        }
    }

    glfwDestroyWindow(context);
    glfwTerminate();

    return 0;
}
