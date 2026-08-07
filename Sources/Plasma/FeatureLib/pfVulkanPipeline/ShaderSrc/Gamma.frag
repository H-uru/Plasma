#version 450
#extension GL_GOOGLE_include_directive : require

#include "plVulkanShaderTypes.h"

layout(set = kDescSetTextures, binding = kBindingTextures2D) uniform texture2D textures2D[kMaxLayers];
layout(set = kDescSetTextures, binding = kBindingSamplers)   uniform sampler samplers[kNumSamplerSlots];

layout(location = 0) in vec2 inTexCoord;
layout(location = 0) out vec4 outColor;

void main()
{
    vec4 color = texture(sampler2D(textures2D[0], samplers[0]), inTexCoord);
    ivec2 lutSize = textureSize(sampler2D(textures2D[1], samplers[1]), 0);
    int last = lutSize.x - 1;
    int r = int(round(clamp(color.r, 0.0, 1.0) * float(last)));
    int g = int(round(clamp(color.g, 0.0, 1.0) * float(last)));
    int b = int(round(clamp(color.b, 0.0, 1.0) * float(last)));
    outColor = vec4(
        texelFetch(sampler2D(textures2D[1], samplers[1]), ivec2(r, 0), 0).r,
        texelFetch(sampler2D(textures2D[1], samplers[1]), ivec2(g, 1), 0).r,
        texelFetch(sampler2D(textures2D[1], samplers[1]), ivec2(b, 2), 0).r,
        1.0);
}
