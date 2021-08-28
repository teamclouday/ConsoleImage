R"(
#version 330 core
#extension GL_ARB_separate_shader_objects : require

layout (location = 0) in vec2 baseUV;
layout (location = 0) out vec3 color;
layout (location = 1) out float grey;

uniform sampler2D image;
uniform float winW;
uniform float winH;
uniform vec3 edgeColor;
uniform int edgeEnable;

)" + std::string(
#include "edge.glsl"
) +
R"(

void main()
{
    vec3 img = texture(image, baseUV).rgb;
    // get edge
    float e = edge(image, baseUV, winW, winH);
    e = smoothstep(0.0, 5.66, e);
    img = mix(img, edgeColor, e * edgeEnable);
    // img = mix(edgeColor, img, e * edgeEnable);
    // convert to greyscale
    // reference: https://community.khronos.org/t/how-to-texture-a-rgb-image-in-grayscale/36420
    const vec3 multiplier = vec3(0.3, 0.59, 0.11);
    grey = clamp(dot(img, multiplier), 0.0, 1.0);
    // set output color
    color = img;
    }
)"
