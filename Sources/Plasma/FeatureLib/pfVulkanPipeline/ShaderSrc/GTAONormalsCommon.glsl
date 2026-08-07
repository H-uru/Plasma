#include "GTAOCommon.glsl"

layout(r32ui, set = 0, binding = 12) uniform writeonly uimage2D outNormals;

float GTAOLoadRawDepth(ivec2 pixel);

void GTAONormalsMain()
{
    ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
    if (any(greaterThanEqual(pixel, gtao.viewportSize)))
        return;

    ivec2 maxPixel = gtao.viewportSize - 1;
    ivec2 leftPixel = clamp(pixel + ivec2(-1, 0), ivec2(0), maxPixel);
    ivec2 rightPixel = clamp(pixel + ivec2(1, 0), ivec2(0), maxPixel);
    ivec2 topPixel = clamp(pixel + ivec2(0, -1), ivec2(0), maxPixel);
    ivec2 bottomPixel = clamp(pixel + ivec2(0, 1), ivec2(0), maxPixel);
    float centerZ = GTAOLinearizeDepth(GTAOLoadRawDepth(pixel));
    float leftZ = GTAOLinearizeDepth(GTAOLoadRawDepth(leftPixel));
    float rightZ = GTAOLinearizeDepth(GTAOLoadRawDepth(rightPixel));
    float topZ = GTAOLinearizeDepth(GTAOLoadRawDepth(topPixel));
    float bottomZ = GTAOLinearizeDepth(GTAOLoadRawDepth(bottomPixel));

    vec2 centerUV = (vec2(pixel) + 0.5) * gtao.viewportPixelSize;
    vec4 edges = GTAOCalculateEdges(centerZ, leftZ, rightZ, topZ, bottomZ);
    vec3 normal = GTAOCalculateNormal(
        edges,
        GTAOViewPosition(centerUV, centerZ),
        GTAOViewPosition((vec2(leftPixel) + 0.5) * gtao.viewportPixelSize, leftZ),
        GTAOViewPosition((vec2(rightPixel) + 0.5) * gtao.viewportPixelSize, rightZ),
        GTAOViewPosition((vec2(topPixel) + 0.5) * gtao.viewportPixelSize, topZ),
        GTAOViewPosition((vec2(bottomPixel) + 0.5) * gtao.viewportPixelSize, bottomZ));
    imageStore(outNormals, pixel, uvec4(GTAOPackNormal(normal)));
}
