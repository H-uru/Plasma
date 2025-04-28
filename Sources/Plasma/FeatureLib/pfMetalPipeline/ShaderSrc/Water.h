//
//  Water.h
//  Plasma
//
//  Created by Colin Cornaby on 12/29/22.
//

#ifndef Water_h
#define Water_h
#include <metal_stdlib>

float3 CalcDepthFilter(const float4 depthOffset, const float4 depthScale, const float4 wPos, const float4 minAtten);

#endif /* Water_h */
