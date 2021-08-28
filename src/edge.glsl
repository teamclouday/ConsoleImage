R"(
// edge detection functions

// sobel edge detector
// https://en.wikipedia.org/wiki/Sobel_operator
// https://en.wikipedia.org/wiki/Canny_edge_detector
// https://computergraphics.stackexchange.com/questions/3646/opengl-glsl-sobel-edge-detection-filter
float edge(sampler2D tex, vec2 coords, float winW, float winH)
{
    vec2 c = vec2(1.0 / winW, 1.0 / winH);
    const mat3 kernelX = mat3(
        -1.0, 0.0, 1.0,
        -2.0, 0.0, 2.0,
        -1.0, 0.0, 1.0
    );
    const mat3 kernelY = mat3(
        -1.0, -2.0, -1.0,
         0.0,  0.0,  0.0,
         1.0,  2.0,  1.0
    );
    // get image
    mat3 img;
    for(int i = 0; i < 3; i++)
    {
        for(int j = 0; j < 3; j++)
        {
            vec3 c = texture(tex, coords + c * vec2(i - 1, j - 1)).rgb;
            img[i][j] = length(c);
        }
    }
    // compute gradients
    float gX = 0.0;
    float gY = 0.0;
    for(int i = 0; i < 3; i++)
    {
        gX += dot(kernelX[i], img[i]);
        gY += dot(kernelY[i], img[i]);
    }
    // compute gradient magnitude
    float mag = sqrt(gX * gX + gY * gY);
    return mag;
}
)"
