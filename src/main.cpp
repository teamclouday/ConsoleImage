#include <GL/glew.h>
#include <argparse/argparse.hpp>
#include <cpp-terminal/terminal.h>
#include <GLFW/glfw3.h>
#include <istream>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#ifdef _WIN32
#include <Windows.h>
#endif

#include <vector>
#include <iostream>
#include <string>
#include <locale>
#include <codecvt>
#include <cmath>
#include <cstdlib>
#include <stdexcept>

struct Vec3
{
    float r;
    float g;
    float b;
    static int clamp(int val)
    {
        val = val < 0 ? 0 : val;
        val = val > 255 ? 255 : val;
        return val;
    }
};

int main(int argc, char** argv)
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    // build argument parser
    argparse::ArgumentParser argument("ConsoleImage");
    // add input image path
    argument.add_argument("filepath")
        .help("image path to display")
        .required();
    // add verbose option
    argument.add_argument("-v", "--verbose")
        .help("show processing details")
        .default_value(false)
        .implicit_value(true);
    // add colored option
    argument.add_argument("--color")
        .help("colored output")
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
        .default_value(std::vector<std::string>())
        .append();
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
    auto chars = argument.get<std::vector<std::string>>("--chars");
    if(!argument.is_used("--chars"))
    {
        // chars = {" ", "░", "▒", "▓"};
        chars = {" ", "▏", "▎", "▍", "▌", "▋", "▊", "▉"};
    }
    int levels = (int)chars.size();
    if(levels <= 0)
    {
        std::cerr << "Invalid chars argument: cannot be empty" << std::endl;
        std::cout << argument;
        exit(-1);
    }
    bool colored = argument.get<bool>("--color");
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
    outputW = (int)((float)outputH * ((float)imgW / (float)imgH)) * 2;

    // init opengl context
    if(!glfwInit())
    {
        if(verbose) std::cerr << "Failed to init glfw" << std::endl;
        exit(-1);
    }
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* context = glfwCreateWindow(outputW, outputH, "", nullptr, nullptr);
    if(!context)
    {
        if(verbose) std::cerr << "Failed to create GLFW context" << std::endl;
        exit(-1);
    }
    glfwMakeContextCurrent(context);
    glewExperimental = GL_TRUE;
    if(glewInit() != GLEW_OK)
    {
        if(verbose) std::cerr << "Failed to init OpenGL context" << std::endl;
        exit(-1);
    }
    glEnable(GL_TEXTURE_2D);
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
    // check if image too large
    GLint limitSize = 0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &limitSize);
    if (imgW > limitSize || imgH > limitSize)
    {
        if (verbose) std::cerr << "Image too large (limit: " << limitSize << "x" << limitSize << ")" << std::endl;
        exit(-1);
    }
    // prepare original image texture
    GLuint imgTex;
    glGenTextures(1, &imgTex);
    glBindTexture(GL_TEXTURE_2D, imgTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    GLenum imgFormat = GL_RGBA;
    switch (imgC)
    {
    case 1:
        imgFormat = GL_RED;
        break;
    case 2:
        imgFormat = GL_RG;
        break;
    case 3:
        imgFormat = GL_RGB;
        break;
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imgW, imgH, 0, imgFormat, GL_UNSIGNED_BYTE, img);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(img);
    // prepare shaders
    GLuint shaderVert = glCreateShader(GL_VERTEX_SHADER);
    GLuint shaderFrag = glCreateShader(GL_FRAGMENT_SHADER);
    const char* shaderVertContent =
    #include "render.vert.glsl"
    ;
    const char* shaderFragContent =
    #include "render.frag.glsl"
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
    GLuint FBOtex[2];
    glGenTextures(2, FBOtex);
    glBindTexture(GL_TEXTURE_2D, FBOtex[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, outputW, outputH, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, FBOtex[0], 0);
    glBindTexture(GL_TEXTURE_2D, FBOtex[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, outputW, outputH, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, FBOtex[1], 0);
    GLenum DrawBuffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, DrawBuffers);
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
    std::vector<Vec3> mem1(outputW * outputH);
    std::vector<float> mem2(outputW * outputH);
    glBindTexture(GL_TEXTURE_2D, FBOtex[0]);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, mem1.data());
    glBindTexture(GL_TEXTURE_2D, FBOtex[1]);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, mem2.data());
    // process data to terminal output
    float step = 1.0f / levels;
    std::vector<std::string> output(outputW * outputH);
    std::transform(mem2.begin(), mem2.end(), std::begin(output),
            [chars, step](const float val){
                int idx = (int)std::round(val / step);
                idx = idx >= (int)chars.size() ? ((int)chars.size() - 1) : idx;
                idx = idx < 0 ? 0 : idx;
                return chars[idx];
            });
    if(verbose) std::cout << "Image processed" << std::endl;

    // output to terminal
    using Term::bg;
    using Term::color;
    using Term::color24_bg;
    for(int i = 0; i < outputH; i++)
    {
        for(int j = 0; j < outputW; j++)
        {
            if(colored)
            {
                auto& vec = mem1[j + i * outputW];
                std::cout << color24_bg(
                    Vec3::clamp((int)(vec.r * 255.0f)),
                    Vec3::clamp((int)(vec.g * 255.0f)),
                    Vec3::clamp((int)(vec.b * 255.0f))
                );
            }
            std::cout << output[j + i * outputW];
            if(j == outputW - 1) 
            {
                if(colored) std::cout << color(bg::reset);
                std::cout << std::endl;
            }
        }
    }
    if(colored) std::cout << color(bg::reset);

    glfwDestroyWindow(context);
    glfwTerminate();

    return 0;
}
