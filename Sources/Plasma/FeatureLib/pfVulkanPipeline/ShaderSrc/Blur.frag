#version 450
#extension GL_GOOGLE_include_directive : require

#include "plVulkanShaderTypes.h"

layout(push_constant) uniform BlurConstants { float axisSigma; } blur;

layout(set = kDescSetTextures, binding = kBindingTextures2D) uniform texture2D textures2D[kMaxLayers];
layout(set = kDescSetTextures, binding = kBindingSamplers)   uniform sampler samplers[kNumSamplerSlots];

layout(location = 0) in vec2 inTexCoord;
layout(location = 0) out vec4 outColor;

void main()
{
    float sigma = max(abs(blur.axisSigma), 0.5);
    int radius = clamp(int(ceil(sigma * 2.0)), 1, 16);
    vec2 texel = 1.0 / vec2(textureSize(sampler2D(textures2D[0], samplers[0]), 0));
    vec2 axis = blur.axisSigma >= 0.0 ? vec2(texel.x, 0.0) : vec2(0.0, texel.y);

    vec4 sum = texture(sampler2D(textures2D[0], samplers[0]), inTexCoord);
    float weightSum = 1.0;
    for (int i = 1; i <= radius; ++i) {
        float weight = exp(-float(i * i) / (2.0 * sigma * sigma));
        vec2 offset = axis * float(i);
        sum += texture(sampler2D(textures2D[0], samplers[0]), inTexCoord - offset) * weight;
        sum += texture(sampler2D(textures2D[0], samplers[0]), inTexCoord + offset) * weight;
        weightSum += 2.0 * weight;
    }
    outColor = sum / weightSum;
}
