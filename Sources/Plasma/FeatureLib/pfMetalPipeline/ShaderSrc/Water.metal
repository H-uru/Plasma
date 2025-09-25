//
//  Water.metal
//  plClient
//
//  Created by Colin Cornaby on 12/29/22.
//

#include <metal_stdlib>
#include "Water.h"
using namespace metal;

// Depth filter channels control:
// dFilter.x => overall opacity
// dFilter.y => reflection strength
// dFilter.z => wave height
float3 CalcDepthFilter(const float4 depthOffset, const float4 depthScale, const float4 wPos, const float4 minAtten) {
    float3 dFilter = float3(depthOffset.xyz) - wPos.zzz;

    dFilter *= float3(depthScale.xyz);
    dFilter += minAtten.xyz;
    dFilter = clamp(dFilter, 0, 1);

    return dFilter;
}
