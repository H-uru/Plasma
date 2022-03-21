//
//  Avatar.metal
//  plClient
//
//  Created by Colin Cornaby on 3/2/22.
//

#include <metal_stdlib>
using namespace metal;


typedef struct {
    float4 position [[position]];
    float2 uvPosition;
} PreprocessAvatarTexturesInOut;

typedef struct
{
    float2 position      [[attribute(0)]];
    float2 uvPostion     [[attribute(1)]];
} PreprocessAvatarVertex;

vertex PreprocessAvatarTexturesInOut PreprocessAvatarVertexShader(PreprocessAvatarVertex in [[stage_in]]) {
    return { float4(in.position.x, in.position.y, 0.0, 1.0 ), in.uvPostion };
}

fragment half4 PreprocessAvatarFragmentShader(PreprocessAvatarTexturesInOut in [[stage_in]],
                                              texture2d<half> layer            [[ texture(0) ]])
{
    constexpr sampler colorSampler(mip_filter::linear,
                                   mag_filter::linear,
                                   min_filter::linear,
                                   address::clamp_to_zero);

    half4 colorSample = layer.sample(colorSampler, in.uvPosition.xy);
    
    return colorSample;
}
