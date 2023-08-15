//
//  GrassShader.metal
//  plGLClient
//
//  Created by Colin Cornaby on 1/1/22.
//

#include <metal_stdlib>
using namespace metal;

#include "ShaderVertex.h"

//ignoring the int and pi constants here and using whats built in
//but reserving space for them in the buffer
typedef struct  {
    matrix_float4x4 Local2NDC;
    float4 intConstants;
    float4 time;
    float4 piConstants;
    float4 sinConstants;
    float4 waveDistortX;
    float4 waveDistortY;
    float4 waveDistortZ;
    float4 waveDirX;
    float4 waveDirY;
    float4 waveSpeed;
} vs_GrassUniforms;
    
typedef struct {
    float4 position [[position]];
    float4 color;
    float4 texCoord;
} vs_GrassInOut;

vertex vs_GrassInOut vs_GrassShader(Vertex in [[stage_in]],
                                           constant vs_GrassUniforms & uniforms [[ buffer(BufferIndexUniforms) ]]) {
    vs_GrassInOut out;
    
    float4 r0 = (in.position.x * uniforms.waveDirX) + (in.position.y * uniforms.waveDirX);
    
    r0 += (uniforms.time.x * uniforms.waveSpeed); // scale by speed and add to X,Y input
    r0 = fract(r0);
    
    r0 = (r0 - 0.5) * M_PI_F * 2;
    
    float4 pow2 = r0 * r0;
    float4 pow3 = pow2 * r0;
    float4 pow5 = pow2 * pow3;
    float4 pow7 = pow2 * pow5;
    float4 pow9 = pow2 * pow7;
    
    r0 += pow3 * uniforms.sinConstants.x;
    r0 += pow5 * uniforms.sinConstants.y;
    r0 += pow7 * uniforms.sinConstants.z;
    r0 += pow9 * uniforms.sinConstants.w;
    
    float3 offset = float3(
                             dot(r0, uniforms.waveDistortX),
                             dot(r0, uniforms.waveDistortY),
                             dot(r0, uniforms.waveDistortZ)
                             );
    
    offset *= (2.0 * (1.0 - in.texCoord1.y)); // mult by Y tex coord. So the waves only affect the top verts
    
    float4 position = float4(in.position.xyz + offset, 1);
    out.position = position * uniforms.Local2NDC;
    
    out.color = float4(in.color.r, in.color.g, in.color.b, in.color.a) / 255.0;
    out.texCoord = float4(in.texCoord1, 0.0);
    
    return out;
}

fragment float4 ps_GrassShader(vs_GrassInOut in [[stage_in]],
                             texture2d<float> t0 [[ texture(0) ]]) {
    constexpr sampler colorSampler = sampler(mip_filter::linear,
                              mag_filter::linear,
                              min_filter::linear,
                              address::repeat);
    
    float4 out = t0.sample(colorSampler, in.texCoord.xy);
    out *= in.color;
    if(out.a <= 0.1)
        discard_fragment();
    return out;
}
