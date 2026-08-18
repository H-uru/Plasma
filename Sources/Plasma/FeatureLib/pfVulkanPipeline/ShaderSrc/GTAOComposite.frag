#version 450

layout(set = 0, binding = 16) uniform usampler2D finalAO;
layout(location = 0) in vec2 outTexCoord;
layout(location = 0) out vec4 outColor;

void main()
{
    float visibility = float(texture(finalAO, outTexCoord).r) / 255.0;
    outColor = vec4(vec3(visibility), 1.0);
}
