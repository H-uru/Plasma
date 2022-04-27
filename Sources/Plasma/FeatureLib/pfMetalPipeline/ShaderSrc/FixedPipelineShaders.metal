/*==LICENSE==*

CyanWorlds.com Engine - MMOG client, server and tools
Copyright (C) 2011  Cyan Worlds, Inc.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/


#include <metal_stdlib>
using namespace metal;
// File for Metal kernel and shader functions

#include <metal_stdlib>
#include <simd/simd.h>

// Including header shared between this Metal shader code and Swift/C code executing Metal API commands
#include "ShaderVertex.h"
#include "ShaderTypes.h"

//copying this direction from hsGMatState because I am a horrible person but we can't import the header here in since it includes a lot of class stuff.
//FIXME: Come up with something better.
enum hsGMatMiscFlags: uint32_t {
    kMiscWireFrame          = 0x1,          // dev (running out of bits)
    kMiscDrawMeshOutlines   = 0x2,          // dev, currently unimplemented
    kMiscTwoSided           = 0x4,          // view,dev
    kMiscDrawAsSplats       = 0x8,          // dev? bwt
    kMiscAdjustPlane        = 0x10,
    kMiscAdjustCylinder     = 0x20,
    kMiscAdjustSphere       = 0x40,
    kMiscAdjust             = kMiscAdjustPlane | kMiscAdjustCylinder| kMiscAdjustSphere,
    kMiscTroubledLoner      = 0x80,
    kMiscBindSkip           = 0x100,
    kMiscBindMask           = 0x200,
    kMiscBindNext           = 0x400,
    kMiscLightMap           = 0x800,
    kMiscUseReflectionXform = 0x1000,       // Use the calculated reflection environment
                                            // texture transform instead of layer->GetTransform()
    kMiscPerspProjection    = 0x2000,
    kMiscOrthoProjection    = 0x4000,
    kMiscProjection         = kMiscPerspProjection | kMiscOrthoProjection,

    kMiscRestartPassHere    = 0x8000,       // Tells pipeline to start a new pass beginning with this layer
                                            // Kinda like troubledLoner, but only cuts off lower layers, not
                                            // higher ones (kMiscBindNext sometimes does this by implication)

    kMiscBumpLayer          = 0x10000,
    kMiscBumpDu             = 0x20000,
    kMiscBumpDv             = 0x40000,
    kMiscBumpDw             = 0x80000,
    kMiscBumpChans          = kMiscBumpDu | kMiscBumpDv | kMiscBumpDw,

    kMiscNoShadowAlpha      = 0x100000,
    kMiscUseRefractionXform = 0x200000, // Use a refraction-like hack.
    kMiscCam2Screen         = 0x400000, // Expects tex coords to be XYZ in camera space. Does a cam to screen (not NDC) projection
                                        // and swaps Z with W, so that the texture projection can produce projected 2D screen coordinates.

    kAllMiscFlags           = 0xffffffff
};
    
enum hsGMatBlendFlags: uint32_t {
    kBlendTest  = 0x1,                          // dev
    // Rest of blends are mutually exclusive
    kBlendAlpha                     = 0x2,      // dev
    kBlendMult                      = 0x4,      // dev
    kBlendAdd                       = 0x8,      // dev
    kBlendAddColorTimesAlpha        = 0x10,     // dev
    kBlendAntiAlias                 = 0x20,
    kBlendDetail                    = 0x40,
    kBlendNoColor                   = 0x80,     // dev
    kBlendMADD                      = 0x100,
    kBlendDot3                      = 0x200,
    kBlendAddSigned                 = 0x400,
    kBlendAddSigned2X               = 0x800,
    kBlendMask                      = kBlendAlpha
                                    | kBlendMult
                                    | kBlendAdd
                                    | kBlendAddColorTimesAlpha
                                    | kBlendDetail
                                    | kBlendMADD
                                    | kBlendDot3
                                    | kBlendAddSigned
                                    | kBlendAddSigned2X,
    kBlendInvertAlpha               = 0x1000,   // dev
    kBlendInvertColor               = 0x2000,   // dev
    kBlendAlphaMult                 = 0x4000,
    kBlendAlphaAdd                  = 0x8000,
    kBlendNoVtxAlpha                = 0x10000,
    kBlendNoTexColor                = 0x20000,
    kBlendNoTexAlpha                = 0x40000,
    kBlendInvertVtxAlpha            = 0x80000,  // Invert ONLY the vertex alpha source
    kBlendAlphaAlways               = 0x100000, // Alpha test always passes (even for alpha=0).
    kBlendInvertFinalColor          = 0x200000,
    kBlendInvertFinalAlpha          = 0x400000,
    kBlendEnvBumpNext               = 0x800000,
    kBlendSubtract                  = 0x1000000,
    kBlendRevSubtract               = 0x2000000,
    kBlendAlphaTestHigh             = 0x4000000,
    kBlendAlphaPremultiplied        = 0x8000000
};
    
enum plUVWSrcModifiers: uint32_t {
    kUVWPassThru                                = 0x00000000,
    kUVWIdxMask                                 = 0x0000ffff,
    kUVWNormal                                  = 0x00010000,
    kUVWPosition                                = 0x00020000,
    kUVWReflect                                 = 0x00030000
};

using namespace metal;
    
constant const uint8_t sourceType1 [[ function_constant(FunctionConstantSources + 0)    ]];
constant const uint8_t sourceType2 [[ function_constant(FunctionConstantSources + 1)    ]];
constant const uint8_t sourceType3 [[ function_constant(FunctionConstantSources + 2)    ]];
constant const uint8_t sourceType4 [[ function_constant(FunctionConstantSources + 3)    ]];
constant const uint8_t sourceType5 [[ function_constant(FunctionConstantSources + 4)    ]];
constant const uint8_t sourceType6 [[ function_constant(FunctionConstantSources + 5)    ]];
constant const uint8_t sourceType7 [[ function_constant(FunctionConstantSources + 6)    ]];
constant const uint8_t sourceType8 [[ function_constant(FunctionConstantSources + 7)    ]];
    
constant const uint32_t blendModes1 [[ function_constant(FunctionConstantBlendModes + 0)    ]];
constant const uint32_t blendModes2 [[ function_constant(FunctionConstantBlendModes + 1)    ]];
constant const uint32_t blendModes3 [[ function_constant(FunctionConstantBlendModes + 2)    ]];
constant const uint32_t blendModes4 [[ function_constant(FunctionConstantBlendModes + 3)    ]];
constant const uint32_t blendModes5 [[ function_constant(FunctionConstantBlendModes + 4)    ]];
constant const uint32_t blendModes6 [[ function_constant(FunctionConstantBlendModes + 5)    ]];
constant const uint32_t blendModes7 [[ function_constant(FunctionConstantBlendModes + 6)    ]];
constant const uint32_t blendModes8 [[ function_constant(FunctionConstantBlendModes + 7)    ]];
    
constant const uint32_t miscFlags1 [[ function_constant(FunctionConstantLayerFlags + 0)    ]];
constant const uint32_t miscFlags2 [[ function_constant(FunctionConstantLayerFlags + 1)    ]];
constant const uint32_t miscFlags3 [[ function_constant(FunctionConstantLayerFlags + 2)    ]];
constant const uint32_t miscFlags4 [[ function_constant(FunctionConstantLayerFlags + 3)    ]];
constant const uint32_t miscFlags5 [[ function_constant(FunctionConstantLayerFlags + 4)    ]];
constant const uint32_t miscFlags6 [[ function_constant(FunctionConstantLayerFlags + 5)    ]];
constant const uint32_t miscFlags7 [[ function_constant(FunctionConstantLayerFlags + 6)    ]];
constant const uint32_t miscFlags8 [[ function_constant(FunctionConstantLayerFlags + 7)    ]];
    
constant const uint32_t sampleType1 [[ function_constant(FunctionConstantSampleTypes + 0)    ]];
constant const uint32_t sampleType2 [[ function_constant(FunctionConstantSampleTypes + 1)    ]];
constant const uint32_t sampleType3 [[ function_constant(FunctionConstantSampleTypes + 2)    ]];
constant const uint32_t sampleType4 [[ function_constant(FunctionConstantSampleTypes + 3)    ]];
constant const uint32_t sampleType5 [[ function_constant(FunctionConstantSampleTypes + 4)    ]];
constant const uint32_t sampleType6 [[ function_constant(FunctionConstantSampleTypes + 5)    ]];
constant const uint32_t sampleType7 [[ function_constant(FunctionConstantSampleTypes + 6)    ]];
constant const uint32_t sampleType8 [[ function_constant(FunctionConstantSampleTypes + 7)    ]];

#define MAX_BLEND_PASSES 8
constant const uint8_t sourceTypes[MAX_BLEND_PASSES] = { sourceType1, sourceType2, sourceType3, sourceType4, sourceType5, sourceType6, sourceType7, sourceType8};
constant const uint32_t blendModes[MAX_BLEND_PASSES] = { blendModes1, blendModes2, blendModes3, blendModes4, blendModes5, blendModes6, blendModes7, blendModes8};
constant const uint32_t miscFlags[MAX_BLEND_PASSES] = { miscFlags1, miscFlags2, miscFlags3, miscFlags4, miscFlags5, miscFlags6, miscFlags7, miscFlags8};
constant const uint32_t sampleTypes[MAX_BLEND_PASSES] = { sampleType1, sampleType2, sampleType3, sampleType4, sampleType5, sampleType6, sampleType7, sampleType8};
constant const uint8_t passCount = (sourceType1 > 0) + (sourceType2 > 0) + (sourceType3 > 0) + (sourceType4 > 0) + (sourceType5 > 0) + (sourceType6 > 0) + (sourceType7 > 0) + (sourceType8 > 0);
    
typedef struct  {
    texture2d<half> textures  [[ texture(FragmentShaderArgumentAttributeTextures), function_constant(hasLayer1)  ]];
    texture2d<half> texture2  [[ texture(FragmentShaderArgumentAttributeTextures + 1), function_constant(hasLayer2)    ]];
    texture2d<half> texture3  [[ texture(FragmentShaderArgumentAttributeTextures + 2), function_constant(hasLayer3)    ]];
    texture2d<half> texture4  [[ texture(FragmentShaderArgumentAttributeTextures + 3), function_constant(hasLayer4)    ]];
    texture2d<half> texture5  [[ texture(FragmentShaderArgumentAttributeTextures + 4), function_constant(hasLayer5)    ]];
    texture2d<half> texture6  [[ texture(FragmentShaderArgumentAttributeTextures + 5), function_constant(hasLayer6)    ]];
    texture2d<half> texture7  [[ texture(FragmentShaderArgumentAttributeTextures + 6), function_constant(hasLayer7)    ]];
    texture2d<half> texture8  [[ texture(FragmentShaderArgumentAttributeTextures + 7), function_constant(hasLayer8)    ]];
    texturecube<half> cubicTextures  [[ texture(FragmentShaderArgumentAttributeCubicTextures), function_constant(hasLayer1)    ]];
    texturecube<half> cubicTexture2  [[ texture(FragmentShaderArgumentAttributeCubicTextures + 1), function_constant(hasLayer2)  ]];
    texturecube<half> cubicTexture3  [[ texture(FragmentShaderArgumentAttributeCubicTextures + 2), function_constant(hasLayer3)  ]];
    texturecube<half> cubicTexture4  [[ texture(FragmentShaderArgumentAttributeCubicTextures + 3), function_constant(hasLayer4)  ]];
    texturecube<half> cubicTexture5  [[ texture(FragmentShaderArgumentAttributeCubicTextures + 4), function_constant(hasLayer5)  ]];
    texturecube<half> cubicTexture6  [[ texture(FragmentShaderArgumentAttributeCubicTextures + 5), function_constant(hasLayer6)  ]];
    texturecube<half> cubicTexture7  [[ texture(FragmentShaderArgumentAttributeCubicTextures + 6), function_constant(hasLayer7)  ]];
    texturecube<half> cubicTexture8  [[ texture(FragmentShaderArgumentAttributeCubicTextures + 7), function_constant(hasLayer8)  ]];
    const constant plMetalFragmentShaderArgumentBuffer*     bufferedUniforms   [[ buffer(BufferIndexFragArgBuffer)   ]];
    sampler repeatSampler [[ sampler(0)   ]];
    sampler clampRepeatSampler [[ sampler(1)   ]];
    sampler repeatClampSampler [[ sampler(2)   ]];
    sampler clampSampler [[ sampler(3)   ]];
    half4 sampleLayer(const size_t index, const half4 vertexColor, const uint8_t passType, float3 sampleCoord) const;
} FragmentShaderArguments;

typedef struct
{
    float4 position [[position]];
    float3 texCoord1 [[function_constant(hasLayer1)]];
    float3 texCoord2 [[function_constant(hasLayer2)]];
    float3 texCoord3 [[function_constant(hasLayer3)]];
    float3 texCoord4 [[function_constant(hasLayer4)]];
    float3 texCoord5 [[function_constant(hasLayer5)]];
    float3 texCoord6 [[function_constant(hasLayer6)]];
    float3 texCoord7 [[function_constant(hasLayer7)]];
    float3 texCoord8 [[function_constant(hasLayer8)]];
    //float4 normal;
    half4 vtxColor [[ centroid_perspective ]];
    half4 fogColor;
    //float4 vCamNormal;
} ColorInOut;
    
    
typedef struct
{
    float4 position [[position, invariant]];
    float3 texCoord1;
} ShadowCasterInOut;

constant constexpr sampler colorSamplers[] = {
    sampler(mip_filter::linear,
            mag_filter::linear,
            min_filter::linear,
            address::repeat),
    sampler(mip_filter::linear,
            mag_filter::linear,
            min_filter::linear,
            s_address::clamp_to_edge,
            t_address::repeat),
    sampler(mip_filter::linear,
            mag_filter::linear,
            min_filter::linear,
            s_address::repeat,
            t_address::clamp_to_edge),
    sampler(mip_filter::linear,
            mag_filter::linear,
            min_filter::linear,
            address::clamp_to_edge),

};

vertex ColorInOut pipelineVertexShader(Vertex in [[stage_in]],
                                       constant VertexUniforms & uniforms [[ buffer(BufferIndexState) ]],
                                       constant float4x4 & blendMatrix1 [[ buffer(BufferIndexBlendMatrix1), function_constant(temp_hasOnlyWeight1) ]])
{
    ColorInOut out;
    //we should have been able to swizzle, but it didn't work in Xcode beta? Try again later.
    const half4 inColor = half4(in.color.b, in.color.g, in.color.r, in.color.a) / half4(255.0f);
    
    const half4 MAmbient = half4(mix(inColor, uniforms.ambientCol, uniforms.ambientSrc));
    const half4 MDiffuse = half4(mix(inColor, uniforms.diffuseCol, uniforms.diffuseSrc));
    const half4 MEmissive = half4(mix(inColor, uniforms.emissiveCol, uniforms.emissiveSrc));
    //const half4 MSpecular = half4(mix(inColor, uniforms.specularCol, uniforms.specularSrc));

    half4 LAmbient = half4(0.0, 0.0, 0.0, 0.0);
    half4 LDiffuse = half4(0.0, 0.0, 0.0, 0.0);

    float3 Ndirection = normalize(uniforms.localToWorldMatrix * float4(in.normal, 0.0)).xyz;

    for (size_t i = 0; i < 8; i++) {
        constant plMetalShaderLightSource *lightSource = &uniforms.lampSources[i];
        if(lightSource->scale == 0.0h)
            continue;
        
        float attenuation;
        float3 direction;

        if (lightSource->position.w == 0.0) {
            // Directional Light with no attenuation
            direction = -(lightSource->direction).xyz;
            attenuation = 1.0;
        } else {
            // Omni Light in all directions
            const float3 v2l = lightSource->position.xyz - float3(uniforms.localToWorldMatrix * float4(in.position, 1.0));
            const float distance = length(v2l);
            direction = normalize(v2l);

            attenuation = 1.0 / (lightSource->constAtten + lightSource->linAtten * distance + lightSource->quadAtten * pow(distance, 2.0));

            if (uniforms.lampSources[i].spotProps.x > 0.0) {
                // Spot Light with cone falloff
                const float a = dot(direction.xyz, normalize(-lightSource->direction).xyz);
                const float theta = lightSource->spotProps.y;
                const float phi = lightSource->spotProps.z;
                const float result = pow((a - phi) / (theta - phi), lightSource->spotProps.x);

                attenuation *= clamp(result, 0.0, 1.0);
            }
        }

        LAmbient.rgb = LAmbient.rgb + half3(attenuation * (uniforms.lampSources[i].ambient.rgb * uniforms.lampSources[i].scale));
        float3 dotResult = dot(Ndirection, direction);
        LDiffuse.rgb = LDiffuse.rgb + MDiffuse.rgb * (uniforms.lampSources[i].diffuse.rgb * uniforms.lampSources[i].scale) * half3(max(0.0, dotResult) * attenuation);
    }

    const half4 ambient = clamp((MAmbient) * (half4(uniforms.globalAmb) + LAmbient), 0.0, 1.0);
    const half4 diffuse = clamp(LDiffuse, 0.0, 1.0);
    const half4 material = clamp(ambient + diffuse + half4(MEmissive), 0.0, 1.0);

    out.vtxColor = half4(material.rgb, abs(uniforms.invVtxAlpha - MDiffuse.a));
    
    float4 position = (uniforms.localToWorldMatrix * float4(in.position, 1.0));
    if(temp_hasOnlyWeight1) {
        const float4 position2 = blendMatrix1 * float4(in.position, 1.0);
        position = (in.weight1 * position) + ((1.0f - in.weight1) * position2);
    }
    const float4 vCamPosition = uniforms.worldToCameraMatrix * position;
    //out.vCamNormal = uniforms.worldToCameraMatrix * (uniforms.localToWorldMatrix * float4(in.position, 0.0));
    
    //Fog
    out.fogColor.a = 1.0;
    if (uniforms.fogExponential > 0) {
        out.fogColor.a = exp(-pow(uniforms.fogValues.y * length(vCamPosition), uniforms.fogValues.x));
    } else {
        if (uniforms.fogValues.y > 0.0) {
            const float start = uniforms.fogValues.x;
            const float end = uniforms.fogValues.y;
            out.fogColor.a = (end - length(vCamPosition.xyz)) / (end - start);
        }
    }
    out.fogColor.rgb = uniforms.fogColor;
    
    const float4 normal = uniforms.worldToCameraMatrix * (uniforms.localToWorldMatrix * float4(in.normal, 0.0));
    
    if(hasLayer1)
        out.texCoord1 = uniforms.sampleLocation(0, &in.texCoord1, normal, vCamPosition);
    if(hasLayer2)
        out.texCoord2 = uniforms.sampleLocation(1, &in.texCoord1, normal, vCamPosition);
    if(hasLayer3)
        out.texCoord3 = uniforms.sampleLocation(2, &in.texCoord1, normal, vCamPosition);
    if(hasLayer4)
        out.texCoord4 = uniforms.sampleLocation(3, &in.texCoord1, normal, vCamPosition);
    if(hasLayer5)
        out.texCoord5 = uniforms.sampleLocation(4, &in.texCoord1, normal, vCamPosition);
    if(hasLayer6)
        out.texCoord5 = uniforms.sampleLocation(5, &in.texCoord1, normal, vCamPosition);
    if(hasLayer7)
        out.texCoord7 = uniforms.sampleLocation(6, &in.texCoord1, normal, vCamPosition);
    if(hasLayer8)
        out.texCoord8 = uniforms.sampleLocation(7, &in.texCoord1, normal, vCamPosition);
    
    out.position = uniforms.projectionMatrix * vCamPosition;

    return out;
}

constexpr void blendFirst(half4 srcSample, thread half4 &destSample, const uint32_t blendFlags);
constexpr void blend(half4 srcSample, thread half4 &destSample, uint32_t blendFlags);
    
float3 VertexUniforms::sampleLocation(size_t index, thread float3 *texCoords, const float4 normal, const float4 camPosition) constant {
    const uint32_t UVWSrc = uvTransforms[index].UVWSrc;
    float4x4 matrix = uvTransforms[index].transform;
    const uint32_t flags = miscFlags[index];
    //Note: If we want to require newer versions of Metal/newer hardware we could pass function pointers instead of doing these ifs.
    if (flags & (kMiscUseReflectionXform | kMiscUseRefractionXform)) {
        matrix = cameraToWorldMatrix;
        matrix[3][0] = matrix[3][1] = matrix[3][2] = 0;

        // This is just a rotation about X of Pi/2 (y = z, z = -y),
        // followed by flipping Z to reflect back towards us (z = -z).

        // swap mat[1][0] and mat[2][0]
        float temp;
        temp = matrix[0][1];
        matrix[0][1] = matrix[0][2];
        matrix[0][2] = temp;

        // swap mat[1][1] and mat[2][1]
        temp = matrix[1][1];
        matrix[1][1] = matrix[1][2];
        matrix[1][2] = temp;

        // swap mat[1][2] and mat[2][2]
        temp = matrix[2][1];
        matrix[2][1] = matrix[2][2];
        matrix[2][2] = temp;

        if (flags & kMiscUseRefractionXform) {
            // Same as reflection, but then matrix = matrix * scaleMatNegateZ.

            // mat[0][2] = -mat[0][2];
            matrix[2][0] = -matrix[2][0];

            // mat[1][2] = -mat[1][2];
            matrix[2][1] = -matrix[2][1];

            // mat[2][2] = -mat[2][2];
            matrix[2][2] = -matrix[2][2];
        }
    }
    else if (flags & kMiscCam2Screen) {
        
        matrix_float4x4 translationMatrix = matrix_float4x4(1.0);
        // mat.MakeScaleMat(hsVector3 camScale(0.5f, -0.5f, 1.f));
        translationMatrix[0][0] = 0.5f;
        translationMatrix[1][1] = -0.5f;
        
        matrix_float4x4 scaleMatrix = matrix_float4x4(1.0);

        // hsVector3 camTrans(0.5f, 0.5f, 0.f);
        scaleMatrix[3][0] = 0.5f;
        scaleMatrix[3][1] = -0.5f;
        
        matrix = scaleMatrix * translationMatrix;

        // The scale and trans move us from NDC to Screen space. We need to swap
        // the Z and W coordinates so that the texture projection will divide by W
        // and give us projected 2D coordinates.
        float temp;

        // swap mat[2][2] and mat[3][2]
        temp = matrix[2][2];
        matrix[2][2] = matrix[2][3];
        matrix[2][3] = temp;

        // swap mat[2][3] and mat[3][3]
        temp = matrix[3][2];
        matrix[3][2] = matrix[3][3];
        matrix[3][3] = temp;

        // Multiply by the projection matrix
        matrix = matrix * projectionMatrix;
    } else if (flags & kMiscProjection) {
        matrix_float4x4 cam2World = cameraToWorldMatrix;
        if( !(UVWSrc & kUVWPosition) ) {
            cam2World.columns[3][0] = 0;
            cam2World.columns[3][1] = 0;
            cam2World.columns[3][2] = 0;
        }
        
        matrix = matrix * cam2World;
    }
    
    float4 sampleCoord;
    
    switch (UVWSrc) {
    case kUVWNormal:
        {
            sampleCoord = matrix * normal;
        }
        break;
    case kUVWPosition:
        {
            sampleCoord = matrix * camPosition;
        }
        break;
    case kUVWReflect:
        {
            sampleCoord = matrix * reflect(normalize(camPosition), normalize(normal));
        }
        break;
    default:
        {
            const int index = UVWSrc & 0x0F;
            sampleCoord = matrix * float4(texCoords[index], 1.0);
        }
        break;
    }
    return sampleCoord.xyz;
}
    
half4 FragmentShaderArguments::sampleLayer(const size_t index, const half4 vertexColor, const uint8_t passType, float3 sampleCoord) const {
    
    if(passType == PassTypeColor) {
        return vertexColor;
    } else {
        /*
         Not using array based lookup here because the compiler
         seems to have an easier time unrolling this if each lookup is done
         with a constant. Using an array based lookup was hurting performance by
         about 1/3rd on Apple Silicon.
         */
        const uint32_t sampleType = sampleTypes[index];
        sampler colorSampler = repeatSampler;
        if(sampleType == 1) {
            colorSampler = clampRepeatSampler;
        } else if(sampleType == 2) {
            colorSampler = repeatClampSampler;
        } else if(sampleType == 3) {
            colorSampler = clampSampler;
        }
        
        if (miscFlags[index] & kMiscPerspProjection) {
            sampleCoord.xy /= sampleCoord.z;
        }
        
        //do the actual sample
        if(passType == PassTypeTexture) {
            return (&textures)[index].sample(colorSampler, sampleCoord.xy);
        } else if(passType == PassTypeCubicTexture) {
            return (&cubicTextures)[index].sample(colorSampler, sampleCoord.xyz);
        } else {
            return half4(0);
        }
    }
}

fragment half4 pipelineFragmentShader(ColorInOut in [[stage_in]],
                                      const FragmentShaderArguments fragmentShaderArgs)
{
    
    half4 currentColor = in.vtxColor;
    
    /*
     SPECIAL PLASMA RULE:
     If there is only one layer, and that layer is not a texture,
     skip straight to the vertex color and return it
     */
    if (!(passCount==1 && sourceTypes[0] == PassTypeColor)) {
        
        /*
         Note: For loop should be unrolled by the compiler, but it is very sensitive.
         Always use size_t for the loop interator type.
         */
        for(size_t layer=0; layer<passCount; layer++) {
            
            float3 sampleCoord = (&in.texCoord1)[layer];
            
            half4 color = fragmentShaderArgs.sampleLayer(layer, in.vtxColor, sourceTypes[layer], sampleCoord);
            
            if(layer==0) {
                blendFirst(color, currentColor, blendModes[layer]);
            } else {
                blend(color, currentColor, blendModes[layer]);
            }
        }
        
        currentColor = half4(in.vtxColor.rgb, 1.0h) * currentColor;
    }
    
    currentColor.rgb = mix(currentColor.rgb, in.fogColor.rgb, (1.0h - clamp((float)in.fogColor.a, 0.0f, 1.0f)) * (float)currentColor.a);
    
    if (currentColor.a < fragmentShaderArgs.bufferedUniforms->alphaThreshold) { discard_fragment(); }

    return currentColor;
}

constexpr void blendFirst(half4 srcSample, thread half4 &destSample, const uint32_t blendFlags) {
    
    // Local variable to store the color value
    if (blendFlags & kBlendInvertColor) {
        srcSample.rgb = 1.0h - srcSample.rgb;
    }
    
    // Leave fCurrColor null if we are blending without texture color
    if (!(blendFlags & kBlendNoTexColor)) {
        destSample.rgb = srcSample.rgb;
    }

    if (blendFlags & kBlendInvertAlpha) {
        // 1.0 - texture.a
        srcSample.a = 1.0h - srcSample.a;
    }

    if (!(blendFlags & kBlendNoTexAlpha)) {
        // Vertex alpha * base texture alpha
        destSample.a = destSample.a * srcSample.a;
    }
}

constexpr void blend(half4 srcSample, thread half4 &destSample, const uint32_t blendFlags) {
    // Local variable to store the color value
    if (blendFlags & kBlendInvertColor) {
        srcSample.rgb = 1.0h - srcSample.rgb;
    }
    
    switch (blendFlags & kBlendMask)
    {

        case kBlendAddColorTimesAlpha:
            //hsAssert(false, "Blend mode unsupported on upper layers");
            break;

        case kBlendAlpha:
        {
            if (!(blendFlags & kBlendNoTexColor)) {
                if (blendFlags & kBlendInvertAlpha) {
                    // color = texture.rgb + (texture.a * prev)
                    destSample.rgb = (srcSample.rgb + (srcSample.a * destSample.rgb));
                } else {
                    // color = mix(prev, texture.rgb, texture.a)
                    destSample.rgb = mix(destSample.rgb, srcSample.rgb, srcSample.a);
                }
            }

            if (blendFlags & kBlendInvertAlpha) {
                // 1.0 - texture.a
                srcSample.a = 1.0h - srcSample.a;
            } else {
                // texture.a
                srcSample.a = srcSample.a;
            }

            switch( blendFlags & ( kBlendAlphaAdd | kBlendAlphaMult ) ) {
                case 0:
                    destSample.a = destSample.a;
                    break;
                case kBlendAlphaAdd:
                    destSample.a = srcSample.a + destSample.a;
                    break;
                case kBlendAlphaMult:
                    destSample.a = srcSample.a * destSample.a;
                break;
            }
            break;
        }

        case kBlendAdd:
        {
            // color = texture.rgb + prev
            destSample.rgb = srcSample.rgb + destSample.rgb;

            break;
        }

        case kBlendMult:
        {
            // color = color * prev
            destSample.rgb = srcSample.rgb * destSample.rgb;
            break;
        }

        case kBlendDot3:
        {
            // color = (color.r * prev.r + color.g * prev.g + color.b * prev.b)
            destSample = dot(srcSample.rgb, destSample.rgb);
            break;
        }

        case kBlendAddSigned:
        {
            // color = color + prev - 0.5
            destSample.rgb = srcSample.rgb + destSample.rgb - 0.5h;
            break;
        }

        case kBlendAddSigned2X:
        {
            // color = (color + prev - 0.5) << 1
            // Note: using CALL here for multiplication to ensure parentheses
            destSample.rgb = 2.0h * (srcSample.rgb + destSample.rgb - 0.5h);
            break;
        }

        case 0:
        {
            // color = texture.rgb
            destSample.rgb = srcSample.rgb;
            break;
        }
    }
}
    
vertex ShadowCasterInOut shadowVertexShader(Vertex in [[stage_in]],
                                       constant VertexUniforms & uniforms [[ buffer(BufferIndexState) ]])
{
    ShadowCasterInOut out;
    
    const float4 vCamPosition = uniforms.worldToCameraMatrix * (uniforms.localToWorldMatrix * float4(in.position, 1.0));
    
    const float4x4 matrix = uniforms.uvTransforms[0].transform;
    
    out.texCoord1 = (matrix * vCamPosition).xyz;
    
    out.position = uniforms.projectionMatrix * vCamPosition;

    return out;
}
    
fragment half4 shadowFragmentShader(ShadowCasterInOut in [[stage_in]])
{
    //D3DTTFF_COUNT3, D3DTSS_TCI_CAMERASPACEPOSITION
    const half currentAlpha = in.texCoord1.x;

    return half4(1.0h, 1.0h, 1.0h, currentAlpha);
}

    
fragment half4 shadowCastFragmentShader(ColorInOut in [[stage_in]],
                                        texture2d<half> texture     [[ texture(16) ]],
                                        constant plMetalShadowCastFragmentShaderArgumentBuffer & fragmentUniforms [[ buffer(BufferIndexShadowCastFragArgBuffer) ]],
                                        FragmentShaderArguments layers,
                                        constant int & alphaSrc [[ buffer(FragmentShaderArgumentShadowAlphaSrc) ]])
{
    float3 sampleCoords = in.texCoord1;
    if(fragmentUniforms.pointLightCast) {
        sampleCoords.xy /= sampleCoords.z;
    }
    half4 currentColor = texture.sample(colorSamplers[3], sampleCoords.xy);
    currentColor.rgb *= in.vtxColor.rgb;
    
    const float2 LUTCoords = in.texCoord2.xy;
    const half4 LUTColor = half4(LUTCoords.x);
    
    currentColor.rgb = (1.0h - LUTColor.rgb) * currentColor.rgb;
    currentColor.a = LUTColor.a - currentColor.a;
    
    //only possible alpha sources are layers 0 or 1
    if(alphaSrc == 0 && passCount > 0) {
        
        half4 layerColor = layers.sampleLayer(0, in.vtxColor,sourceTypes[2], in.texCoord3);
        
        currentColor.rgb *= layerColor.a;
        currentColor.rgb *= in.vtxColor.a;
    } else if(alphaSrc == 1 && passCount > 1) {
        
        half4 layerColor = layers.sampleLayer(1, in.vtxColor, sourceTypes[2], in.texCoord3);
        
        currentColor.rgb *= layerColor.a;
        currentColor.rgb *= in.vtxColor.a;
    }
    
    //alpha blend goes here
    
    if(currentColor.a <= 0.0h)
        discard_fragment();
    
    return currentColor;
}
