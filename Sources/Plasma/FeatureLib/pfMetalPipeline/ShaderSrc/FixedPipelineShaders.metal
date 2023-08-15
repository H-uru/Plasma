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
enum hsGMatMiscFlags {
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
    
enum hsGMatBlendFlags {
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
    
enum plUVWSrcModifiers {
    kUVWPassThru                                = 0x00000000,
    kUVWIdxMask                                 = 0x0000ffff,
    kUVWNormal                                  = 0x00010000,
    kUVWPosition                                = 0x00020000,
    kUVWReflect                                 = 0x00030000
};

using namespace metal;

typedef struct  {
    array<texture2d<half>, 8> textures  [[ texture(FragmentShaderArgumentAttributeTextures)  ]];
    array<texturecube<half>, 8> cubicTextures  [[ texture(FragmentShaderArgumentAttributeCubicTextures)  ]];
    constant float4* colors   [[ buffer(FragmentShaderArgumentAttributeColors)   ]];
    constant plMetalFragmentShaderArgumentBuffer*     bufferedUniforms   [[ buffer(BufferIndexFragArgBuffer)   ]];
} FragmentShaderArguments;

inline float3 sampleLocation(thread float3 *texCoords, matrix_float4x4 matrix, const uint UVWSrc, const uint flags, const float4 normal, const float4 camPosition, const matrix_float4x4 camToWorldMatrix, const matrix_float4x4 projectionMatrix);

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
    half4 vtxColor;
    half4 fogColor;
    //float4 vCamNormal;
} ColorInOut;

vertex ColorInOut pipelineVertexShader(Vertex in [[stage_in]],
                                       constant VertexUniforms & uniforms [[ buffer(BufferIndexState) ]],
                                       uint v_id [[vertex_id]])
{
    ColorInOut out;
    //we should have been able to swizzle, but it didn't work in Xcode beta? Try again later.
    float4 inColor = float4(in.color.b, in.color.g, in.color.r, in.color.a) / float4(255.0f);
    
    float4 MAmbient = mix(inColor, uniforms.ambientCol, uniforms.ambientSrc);
    float4 MDiffuse = mix(inColor, uniforms.diffuseCol, uniforms.diffuseSrc);
    float4 MEmissive = mix(inColor, uniforms.emissiveCol, uniforms.emissiveSrc);
    float4 MSpecular = mix(inColor, uniforms.specularCol, uniforms.specularSrc);

    float4 LAmbient = float4(0.0, 0.0, 0.0, 0.0);
    float4 LDiffuse = float4(0.0, 0.0, 0.0, 0.0);

    float3 Ndirection = normalize(uniforms.worldToLocalMatrix * float4(in.normal, 0.0)).xyz;

    for (uint i = 0; i < 8; i++) {
        plMetalShaderLightSource lightSource = uniforms.lampSources[i];
        if(lightSource.scale == 0)
            continue;
        
        float attenuation;
        float3 direction;

        if (lightSource.position.w == 0.0) {
            // Directional Light with no attenuation
            direction = -(lightSource.direction).xyz;
            attenuation = 1.0;
        } else {
            // Omni Light in all directions
            float3 v2l = lightSource.position.xyz - float3(uniforms.localToWorldMatrix * float4(in.position, 1.0));
            float distance = length(v2l);
            direction = normalize(v2l);

            attenuation = 1.0 / (lightSource.constAtten + lightSource.linAtten * distance + lightSource.quadAtten * pow(distance, 2.0));

            if (uniforms.lampSources[i].spotProps.x > 0.0) {
                // Spot Light with cone falloff
                float a = dot(direction.xyz, normalize(-lightSource.direction).xyz);
                float theta = lightSource.spotProps.y;
                float phi = lightSource.spotProps.z;
                float result = pow((a - phi) / (theta - phi), lightSource.spotProps.x);

                attenuation *= clamp(result, 0.0, 1.0);
            }
        }

        LAmbient.rgb = LAmbient.rgb + attenuation * (uniforms.lampSources[i].ambient.rgb * uniforms.lampSources[i].scale);
        float3 dotResult = dot(Ndirection, direction);
        LDiffuse.rgb = LDiffuse.rgb + MDiffuse.rgb * (uniforms.lampSources[i].diffuse.rgb * uniforms.lampSources[i].scale) * max(0.0, dotResult) * attenuation;
    }

    float4 ambient = clamp(float4(MAmbient) * (uniforms.globalAmb + LAmbient), 0.0, 1.0);
    float4 diffuse = clamp(LDiffuse, 0.0, 1.0);
    float4 material = clamp(ambient + diffuse + float4(MEmissive), 0.0, 1.0);

    out.vtxColor = half4(float4(material.rgb, abs(uniforms.invVtxAlpha - MDiffuse.a)));
    
    float4 vCamPosition = uniforms.worldToCameraMatrix * (uniforms.localToWorldMatrix * float4(in.position, 1.0));
    //out.vCamNormal = uniforms.worldToCameraMatrix * (uniforms.localToWorldMatrix * float4(in.position, 0.0));
    
    //Fog
    out.fogColor.a = 1.0;
    if (uniforms.fogExponential > 0) {
        out.fogColor.a = exp(-pow(uniforms.fogValues.y * length(vCamPosition), uniforms.fogValues.x));
    } else {
        if (uniforms.fogValues.y > 0.0) {
            float start = uniforms.fogValues.x;
            float end = uniforms.fogValues.y;
            out.fogColor.a = (end - length(vCamPosition.xyz)) / (end - start);
        }
    }
    out.fogColor.rgb = half3(uniforms.fogColor);
    
    float4 normal = uniforms.worldToCameraMatrix * (uniforms.localToWorldMatrix * float4(in.normal, 0.0));
    
    if(hasLayer1)
        out.texCoord1 = sampleLocation(&in.texCoord1, uniforms.uvTransforms[0].transform, uniforms.uvTransforms[0].UVWSrc, uniforms.uvTransforms[0].flags, normal, vCamPosition, uniforms.cameraToWorldMatrix, uniforms.projectionMatrix);
    if(hasLayer2)
        out.texCoord2 = sampleLocation(&in.texCoord1, uniforms.uvTransforms[1].transform, uniforms.uvTransforms[1].UVWSrc, uniforms.uvTransforms[1].flags, normal, vCamPosition, uniforms.cameraToWorldMatrix, uniforms.projectionMatrix);
    if(hasLayer3)
        out.texCoord3 = sampleLocation(&in.texCoord1, uniforms.uvTransforms[2].transform, uniforms.uvTransforms[2].UVWSrc, uniforms.uvTransforms[2].flags, normal, vCamPosition, uniforms.cameraToWorldMatrix, uniforms.projectionMatrix);
    if(hasLayer4)
        out.texCoord4 = sampleLocation(&in.texCoord1, uniforms.uvTransforms[3].transform, uniforms.uvTransforms[3].UVWSrc, uniforms.uvTransforms[3].flags, normal, vCamPosition, uniforms.cameraToWorldMatrix, uniforms.projectionMatrix);
    if(hasLayer5)
        out.texCoord5 = sampleLocation(&in.texCoord1, uniforms.uvTransforms[4].transform, uniforms.uvTransforms[4].UVWSrc, uniforms.uvTransforms[4].flags, normal, vCamPosition, uniforms.cameraToWorldMatrix, uniforms.projectionMatrix);
    if(hasLayer6)
        out.texCoord5 = sampleLocation(&in.texCoord1, uniforms.uvTransforms[5].transform, uniforms.uvTransforms[5].UVWSrc, uniforms.uvTransforms[5].flags, normal, vCamPosition, uniforms.cameraToWorldMatrix, uniforms.projectionMatrix);
    if(hasLayer7)
        out.texCoord7 = sampleLocation(&in.texCoord1, uniforms.uvTransforms[6].transform, uniforms.uvTransforms[6].UVWSrc, uniforms.uvTransforms[6].flags, normal, vCamPosition, uniforms.cameraToWorldMatrix, uniforms.projectionMatrix);
    if(hasLayer8)
        out.texCoord8 = sampleLocation(&in.texCoord1, uniforms.uvTransforms[7].transform, uniforms.uvTransforms[7].UVWSrc, uniforms.uvTransforms[7].flags, normal, vCamPosition, uniforms.cameraToWorldMatrix, uniforms.projectionMatrix);
    
    out.position = uniforms.projectionMatrix * vCamPosition;

    return out;
}

inline void blendFirst(half4 srcSample, thread half4 &destSample, const uint32_t blendFlags);
inline void blend(half4 srcSample, thread half4 &destSample, uint32_t blendFlags);
    
inline float3 sampleLocation(thread float3 *texCoords, matrix_float4x4 matrix, const uint UVWSrc, const uint flags, const float4 normal, const float4 camPosition, const matrix_float4x4 camToWorldMatrix, const matrix_float4x4 projectionMatrix) {
    //Note: If we want to require newer versions of Metal/newer hardware we could pass function pointers instead of doing these ifs.
    if (flags & (kMiscUseReflectionXform | kMiscUseRefractionXform)) {
        matrix = camToWorldMatrix;
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
        matrix_float4x4 cam2World = camToWorldMatrix;
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
            int index = UVWSrc & 0x0f;
            sampleCoord = matrix * float4(texCoords[index], 1.0);
        }
        break;
    }
    return sampleCoord.xyz;
}

half4 blendLayer(plFragmentShaderLayer layer, float3 sampleCoord,  half4 color, texture2d<half> texture, thread texturecube<half> *cubicTexture) {
    
    constexpr sampler colorSamplers[] = {
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
    
    ushort passType = layer.passType;
    
    if(passType == PassTypeColor) {
        return color;
    } else {
        
        if (layer.miscFlags & kMiscPerspProjection) {
            sampleCoord.xy = sampleCoord.xy / sampleCoord.z;
        }
        
        int colorSamplerIndex = layer.sampleType;
        //do the actual sample
        if(passType == PassTypeTexture) {
            texture2d<half> colorMap = texture;
            return colorMap.sample(colorSamplers[colorSamplerIndex], sampleCoord.xy);
        } else if(passType == PassTypeCubicTexture) {
            thread texturecube<half> *colorMap = cubicTexture;
            return colorMap->sample(colorSamplers[colorSamplerIndex], sampleCoord.xyz);
        } else {
            return half4(0);
        }
    }
}

fragment half4 pipelineFragmentShader(ColorInOut in [[stage_in]],
                                       constant VertexUniforms & uniforms [[ buffer(BufferIndexState) ]],
                                       FragmentShaderArguments fragmentShaderArgs)
{
    half4 currentColor = in.vtxColor;
    
    uint i = 0;
    for(i=i; i< num_layers; i++) {
        plFragmentShaderLayer layer = fragmentShaderArgs.bufferedUniforms->layers[i];
        
        thread texturecube<half>* cubicTexture =  &(fragmentShaderArgs.cubicTextures[i]);
        half4 color = blendLayer(layer, (&in.texCoord1)[i], currentColor, fragmentShaderArgs.textures[i], cubicTexture);
        if(i==0) {
            blendFirst(color, currentColor, layer.blendMode);
        } else {
            blend(color, currentColor, layer.blendMode);
        }
    }
    
    currentColor = half4(in.vtxColor.rgb, 1.0) * currentColor;
    currentColor.rgb = mix(currentColor.rgb, in.fogColor.rgb * currentColor.a, 1.0f - clamp((float)in.fogColor.a, 0.0f, 1.0f));
    
    if (currentColor.a < fragmentShaderArgs.bufferedUniforms->alphaThreshold) { discard_fragment(); }

    return currentColor;
}

inline void blendFirst(half4 srcSample, thread half4 &destSample, const uint32_t blendFlags) {
    // Local variable to store the color value
    if (blendFlags & kBlendInvertColor) {
        srcSample.rgb = 255 - srcSample.rgb;
    }
    
    // Leave fCurrColor null if we are blending without texture color
    if (!(blendFlags & kBlendNoTexColor)) {
        destSample.rgb = srcSample.rgb;
    }

    if (blendFlags & kBlendInvertAlpha) {
        // 1.0 - texture.a
        srcSample.a = 255 - srcSample.a;
    }

    if (!(blendFlags & kBlendNoTexAlpha)) {
        // Vertex alpha * base texture alpha
        destSample.a = destSample.a * srcSample.a;
    }
}

inline void blend(half4 srcSample, thread half4 &destSample, const uint32_t blendFlags) {
    // Local variable to store the color value
    if (blendFlags & kBlendInvertColor) {
        srcSample.rgb = 255 - srcSample.rgb;
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
                srcSample.a = 1.0 - srcSample.a;
            } else {
                // texture.a
                srcSample.a = srcSample.a;
            }

            if (blendFlags & kBlendAlphaAdd) {
                // alpha = alphaVal + prev
                destSample.a = srcSample.a + destSample.a;
            } else if (blendFlags & kBlendAlphaMult) {
                // alpha = alphaVal * prev
                destSample.a = srcSample.a * destSample.a;
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
            destSample.rgb = srcSample.rgb + destSample.rgb - 0.5;
            break;
        }

        case kBlendAddSigned2X:
        {
            // color = (color + prev - 0.5) << 1
            // Note: using CALL here for multiplication to ensure parentheses
            destSample.rgb = 2 * (srcSample.rgb + destSample.rgb - 0.5);
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
    
fragment float4 shadowFragmentShader(ColorInOut in [[stage_in]],
                                       constant VertexUniforms & uniforms [[ buffer(BufferIndexState) ]],
                                        texture2d<ushort> colorMap     [[ texture(0) ]])
{
    constexpr sampler colorSamplers[] = {
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
    
    //D3DTTFF_COUNT3, D3DTSS_TCI_CAMERASPACEPOSITION
    ushort4 currentColor = colorMap.sample(colorSamplers[3], in.texCoord1.xy);

    return float4(1.0, 1.0, 1.0, float(currentColor.a)/255.0f);
}

    
fragment float4 shadowCastFragmentShader(ColorInOut in [[stage_in]],
                                        constant VertexUniforms & uniforms [[ buffer(BufferIndexState) ]],
                                        texture2d<float> texture     [[ texture(16) ]],
                                        texture2d<ushort> LUT     [[ texture(17) ]],
                                        constant plMetalShadowCastFragmentShaderArgumentBuffer & fragmentUniforms [[ buffer(BufferIndexShadowCastFragArgBuffer) ]],
                                        FragmentShaderArguments layers,
                                        constant int & alphaSrc [[ buffer(FragmentShaderArgumentShadowAlphaSrc) ]])
{
    
    constexpr sampler colorSamplers[] = {
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
    
    float3 sampleCoords = in.texCoord1;
    if(fragmentUniforms.pointLightCast) {
        sampleCoords.xy /= sampleCoords.z;
    }
    float4 currentColor = float4(texture.sample(colorSamplers[3], sampleCoords.xy));
    currentColor.rgb *= float3(in.vtxColor.rgb);
    
    float3 LUTCoords = in.texCoord2;
    float4 LUTColor = float4(LUT.sample(colorSamplers[3], LUTCoords.xy))/255.0f;
    
    currentColor.rgb = (1.0 - LUTColor.rgb) * currentColor.rgb;
    currentColor.a = LUTColor.a - currentColor.a;
    
    if(alphaSrc != -1) {
        half4 layerColor = blendLayer(layers.bufferedUniforms->layers[alphaSrc], in.texCoord3, half4(layers.colors[alphaSrc]), layers.textures[alphaSrc], nullptr);
        
        currentColor.rgb *= layerColor.a;
        currentColor.rgb *= uniforms.diffuseCol.a;
    }
    
    //alpha blend goes here
    
    if(currentColor.a <= 0.0)
        discard_fragment();
    
    return currentColor;
}
