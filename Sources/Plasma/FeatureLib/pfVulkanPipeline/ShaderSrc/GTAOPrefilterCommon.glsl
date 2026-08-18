#include "GTAOCommon.glsl"

layout(r32f, set = 0, binding = 7) uniform writeonly image2D outDepth0;
layout(r32f, set = 0, binding = 8) uniform writeonly image2D outDepth1;
layout(r32f, set = 0, binding = 9) uniform writeonly image2D outDepth2;
layout(r32f, set = 0, binding = 10) uniform writeonly image2D outDepth3;
layout(r32f, set = 0, binding = 11) uniform writeonly image2D outDepth4;

shared float scratchDepth[8][8];

float GTAOLoadRawDepth(ivec2 pixel);

void GTAOPrefilterMain()
{
    ivec2 base = ivec2(gl_GlobalInvocationID.xy);
    ivec2 pixel = base * 2;
    ivec2 maxPixel = gtao.viewportSize - 1;
    float d0 = GTAOLinearizeDepth(GTAOLoadRawDepth(clamp(pixel, ivec2(0), maxPixel)));
    float d1 = GTAOLinearizeDepth(GTAOLoadRawDepth(clamp(pixel + ivec2(1, 0), ivec2(0), maxPixel)));
    float d2 = GTAOLinearizeDepth(GTAOLoadRawDepth(clamp(pixel + ivec2(0, 1), ivec2(0), maxPixel)));
    float d3 = GTAOLinearizeDepth(GTAOLoadRawDepth(clamp(pixel + ivec2(1, 1), ivec2(0), maxPixel)));

    if (all(lessThan(pixel, gtao.viewportSize))) imageStore(outDepth0, pixel, vec4(d0));
    if (all(lessThan(pixel + ivec2(1, 0), gtao.viewportSize))) imageStore(outDepth0, pixel + ivec2(1, 0), vec4(d1));
    if (all(lessThan(pixel + ivec2(0, 1), gtao.viewportSize))) imageStore(outDepth0, pixel + ivec2(0, 1), vec4(d2));
    if (all(lessThan(pixel + ivec2(1, 1), gtao.viewportSize))) imageStore(outDepth0, pixel + ivec2(1, 1), vec4(d3));

    float mip1 = GTAODepthMipFilter(d0, d1, d2, d3);
    if (all(lessThan(base, imageSize(outDepth1)))) imageStore(outDepth1, base, vec4(mip1));
    scratchDepth[gl_LocalInvocationID.x][gl_LocalInvocationID.y] = mip1;
    barrier();

    uvec2 local = gl_LocalInvocationID.xy;
    if (all(equal(local % 2u, uvec2(0)))) {
        float value = GTAODepthMipFilter(scratchDepth[local.x][local.y],
                                         scratchDepth[local.x + 1][local.y],
                                         scratchDepth[local.x][local.y + 1],
                                         scratchDepth[local.x + 1][local.y + 1]);
        if (all(lessThan(base / 2, imageSize(outDepth2)))) imageStore(outDepth2, base / 2, vec4(value));
        scratchDepth[local.x][local.y] = value;
    }
    barrier();
    if (all(equal(local % 4u, uvec2(0)))) {
        float value = GTAODepthMipFilter(scratchDepth[local.x][local.y],
                                         scratchDepth[local.x + 2][local.y],
                                         scratchDepth[local.x][local.y + 2],
                                         scratchDepth[local.x + 2][local.y + 2]);
        if (all(lessThan(base / 4, imageSize(outDepth3)))) imageStore(outDepth3, base / 4, vec4(value));
        scratchDepth[local.x][local.y] = value;
    }
    barrier();
    if (all(equal(local % 8u, uvec2(0)))) {
        float value = GTAODepthMipFilter(scratchDepth[0][0], scratchDepth[4][0],
                                         scratchDepth[0][4], scratchDepth[4][4]);
        if (all(lessThan(base / 8, imageSize(outDepth4)))) imageStore(outDepth4, base / 8, vec4(value));
    }
}
