#include <argparse/argparse.hpp>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <iostream>
#include <string>
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
        .help("number of lines (size) to display")
        .default_value(8)
        .scan<'i', int>();
    // add selected characters
    argument.add_argument("-c", "--chars")
        .help("characters to use (dark to light)")
        .default_value(".*o@");
    // get arguments
    try
    {
        argument.parse_args(argc, argv);
    }
    catch(const std::runtime_error& err)
    {
        std::cerr << err.what() << std::endl;
        std::cout << argument;
        exit(0);
    }
    bool verbose = argument.get<bool>("--verbose");
    
    // load image
    int imgW, imgH, imgC;
    unsigned char* img = stbi_load(argument.get<std::string>("filepath").c_str(), &imgW, &imgH, &imgC, 0);
    if(!img)
    {
        std::cerr << "Failed to load " << argument.get<std::string>("filepath") << std::endl;
        exit(-1);
    }

    // init opengl context



    return 0;
}
