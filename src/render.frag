R"(
#version 330 core
#extension GL_ARB_separate_shader_objects : require

layout (location = 0) in vec2 baseUV;
layout (location = 0) out float color;

uniform sampler2D image;

void main()
{
    vec4 img = texture(image, baseUV);
    // convert to greyscale
    // reference: https://community.khronos.org/t/how-to-texture-a-rgb-image-in-grayscale/36420
    const vec3 multiplier = vec3(0.3, 0.59, 0.11);
    color = clamp(dot(img.rgb, multiplier), 0.0, 1.0);
}
)"
