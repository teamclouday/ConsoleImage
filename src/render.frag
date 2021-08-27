R"(
#version 330 core
#extension GL_ARB_separate_shader_objects : require

layout (location = 0) in vec2 baseUV;
layout (location = 0) out vec3 color;
layout (location = 1) out float grey;

uniform sampler2D image;

void main()
{
    vec4 img = texture(image, baseUV);
    // convert to greyscale
    // reference: https://community.khronos.org/t/how-to-texture-a-rgb-image-in-grayscale/36420
    const vec3 multiplier = vec3(0.3, 0.59, 0.11);
    grey = clamp(dot(img.rgb, multiplier), 0.0, 1.0);
    // determine the closest color
    // reference: https://chrisyeh96.github.io/2020/03/28/terminal-colors.html
    // const int colorsIdx[16] = int[](30, 31, 32, 33, 34, 35, 36, 37, 90, 91, 92, 93, 94, 95, 96, 97);
    // const vec3 colors[16] = vec3[](
    //     vec3(0.0, 0.0, 0.0), // black = 30
    //     vec3(0.8, 0.0, 0.0), // red = 31
    //     vec3(0.3, 0.6, 0.0), // green = 32
    //     vec3(0.8, 0.6, 0.0), // yellow = 33,
    //     vec3(0.4, 0.6, 0.8), // blue = 34,
    //     vec3(0.5, 0.3, 0.5), // magenta = 35,
    //     vec3(0.0, 0.6, 0.6), // cyan = 36,
    //     vec3(0.8, 0.8, 0.8), // white = 37,
    //     vec3(0.3, 0.3, 0.3), // gray = 90,
    //     vec3(0.9, 0.2, 0.2), // bright_red = 91,
    //     vec3(0.5, 0.9, 0.2), // bright_green = 92,
    //     vec3(1.0, 0.9, 0.3), // bright_yellow = 93,
    //     vec3(0.2, 0.7, 1.0), // bright_blue = 94,
    //     vec3(0.7, 0.5, 0.7), // bright_magenta = 95,
    //     vec3(0.2, 0.9, 0.9), // bright_cyan = 96,
    //     vec3(1.0, 1.0, 1.0)  // bright_white = 97
    // );
    // color = colorsIdx[0];
    // float dist = distance(img.rgb, colors[0]);
    // for(int i = 1; i < 16; i++)
    // {
    //     float d = distance(img.rgb, colors[i]);
    //     if(d < dist)
    //     {
    //         dist = d;
    //         color = colorsIdx[i];
    //     }
    // }
    color = img.rgb;
}
)"
