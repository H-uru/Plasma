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

// Ported from pfMetalPipeline/ShaderSrc/FixedPipelineShaders.metal.
//
// The layer loop is Plasma's fixed-function texture combiner: layer 0 seeds the
// color through blendFirst, then each further layer folds in through blend()
// according to its own blend flags. The loop bound is a specialization constant,
// so a given permutation unrolls to exactly the layers it has.

#version 450
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_scalar_block_layout : require

#include "plVulkanShaderTypes.h"

layout(constant_id = kSpecNumLayers) const int numLayers = 1;

layout(constant_id = kSpecPassTypes + 0) const uint passType0 = kPassTypeTexture;
layout(constant_id = kSpecPassTypes + 1) const uint passType1 = kPassTypeTexture;
layout(constant_id = kSpecPassTypes + 2) const uint passType2 = kPassTypeTexture;
layout(constant_id = kSpecPassTypes + 3) const uint passType3 = kPassTypeTexture;
layout(constant_id = kSpecPassTypes + 4) const uint passType4 = kPassTypeTexture;
layout(constant_id = kSpecPassTypes + 5) const uint passType5 = kPassTypeTexture;
layout(constant_id = kSpecPassTypes + 6) const uint passType6 = kPassTypeTexture;
layout(constant_id = kSpecPassTypes + 7) const uint passType7 = kPassTypeTexture;

layout(constant_id = kSpecBlendModes + 0) const uint blendMode0 = 0;
layout(constant_id = kSpecBlendModes + 1) const uint blendMode1 = 0;
layout(constant_id = kSpecBlendModes + 2) const uint blendMode2 = 0;
layout(constant_id = kSpecBlendModes + 3) const uint blendMode3 = 0;
layout(constant_id = kSpecBlendModes + 4) const uint blendMode4 = 0;
layout(constant_id = kSpecBlendModes + 5) const uint blendMode5 = 0;
layout(constant_id = kSpecBlendModes + 6) const uint blendMode6 = 0;
layout(constant_id = kSpecBlendModes + 7) const uint blendMode7 = 0;

layout(constant_id = kSpecLayerFlags + 0) const uint layerFlags0 = 0;
layout(constant_id = kSpecLayerFlags + 1) const uint layerFlags1 = 0;
layout(constant_id = kSpecLayerFlags + 2) const uint layerFlags2 = 0;
layout(constant_id = kSpecLayerFlags + 3) const uint layerFlags3 = 0;
layout(constant_id = kSpecLayerFlags + 4) const uint layerFlags4 = 0;
layout(constant_id = kSpecLayerFlags + 5) const uint layerFlags5 = 0;
layout(constant_id = kSpecLayerFlags + 6) const uint layerFlags6 = 0;
layout(constant_id = kSpecLayerFlags + 7) const uint layerFlags7 = 0;

// Specialization constants cannot initialize a global const array.
uint passTypes(int i)
{
    switch (i) {
    case 0:  return passType0;
    case 1:  return passType1;
    case 2:  return passType2;
    case 3:  return passType3;
    case 4:  return passType4;
    case 5:  return passType5;
    case 6:  return passType6;
    default: return passType7;
    }
}

uint blendModes(int i)
{
    switch (i) {
    case 0:  return blendMode0;
    case 1:  return blendMode1;
    case 2:  return blendMode2;
    case 3:  return blendMode3;
    case 4:  return blendMode4;
    case 5:  return blendMode5;
    case 6:  return blendMode6;
    default: return blendMode7;
    }
}

uint miscFlags(int i)
{
    switch (i) {
    case 0:  return layerFlags0;
    case 1:  return layerFlags1;
    case 2:  return layerFlags2;
    case 3:  return layerFlags3;
    case 4:  return layerFlags4;
    case 5:  return layerFlags5;
    case 6:  return layerFlags6;
    default: return layerFlags7;
    }
}

layout(constant_id = kSpecPerPixelLighting) const bool perPixelLighting = false;

layout(scalar, set = kDescSetUniforms, binding = kBindingMaterial)
uniform MaterialBlock { plMaterialLightingDescriptor material; };

layout(scalar, set = kDescSetUniforms, binding = kBindingLights)
readonly buffer LightBlock { plShaderLightSource lights[]; };

/**
 * Ported from calcLitMaterialColor (FixedPipelineShaders.metal:187-247).
 *
 * The *Src fields are 0/1 mix selectors between the vertex color and the
 * material color, which is how Plasma's three kLiteMask lighting models are
 * expressed without branching.
 */
vec4 calcLitMaterialColor(vec4 materialColor, vec4 position, vec3 normal)
{
    vec3 LAmbient = vec3(0.0);
    vec3 LDiffuse = vec3(0.0);

    const vec3 MAmbient = mix(materialColor.rgb, material.ambientCol, float(material.ambientSrc));
    const vec4 MDiffuse = mix(materialColor, material.diffuseCol, float(material.diffuseSrc));
    const vec3 MEmissive = mix(materialColor.rgb, material.emissiveCol, float(material.emissiveSrc));

    for (uint i = 0u; i < material.lightCount; i++) {
        const uint index = material.activeLights[i].index;
        const float lightScale = material.activeLights[i].scale;
        if (lightScale == 0.0)
            continue;

        // w carries the attenuation for this fragment.
        vec4 direction;

        if (lights[index].position.w == 0.0) {
            // Directional: no attenuation, no range test.
            direction = vec4(-lights[index].direction.xyz, 1.0);
        } else {
            const vec3 v2l = lights[index].position.xyz - position.xyz;
            const float dist = length(v2l);

            if (dist > lights[index].range)
                continue;

            direction.xyz = normalize(v2l);
            direction.w = 1.0 / (lights[index].constAtten +
                                 lights[index].linAtten * dist +
                                 lights[index].quadAtten * dist * dist);

            if (lights[index].spotProps.x > 0.0) {
                const float theta = dot(direction.xyz, normalize(-lights[index].direction.xyz));
                const float gamma = lights[index].spotProps.y;  // inner cutoff
                const float phi = lights[index].spotProps.z;    // outer cutoff
                const float intensity = clamp((theta - phi) / (gamma - phi), 0.0, 1.0);

                direction.w *= pow(intensity, lights[index].spotProps.x);
            }
        }

        LAmbient += direction.w * (lights[index].ambient.rgb * lightScale);
        LDiffuse += MDiffuse.rgb * (lights[index].diffuse.rgb * lightScale) *
                    max(0.0, dot(normal, direction.xyz)) * direction.w;
    }

    const vec3 ambient = MAmbient * clamp(material.globalAmb.rgb + LAmbient, 0.0, 1.0);
    const vec3 diffuse = clamp(LDiffuse, 0.0, 1.0);

    return clamp(vec4(ambient + diffuse + MEmissive,
                      abs(float(material.invertAlpha) - MDiffuse.a)), 0.0, 1.0);
}


// Separate images and samplers, so the four clamp-mode samplers can be shared
// across every material rather than baked into combined descriptors.
layout(set = kDescSetTextures, binding = kBindingTextures2D)   uniform texture2D   textures2D[kMaxLayers];
layout(set = kDescSetTextures, binding = kBindingTexturesCube) uniform textureCube texturesCube[kMaxLayers];
layout(set = kDescSetTextures, binding = kBindingSamplers)     uniform sampler     samplers[kNumSamplerSlots];

layout(location = 0) in vec4 inColor;
layout(location = 1) in vec4 inFogColor;
layout(location = 2) in vec3 inTexCoord[kMaxLayers];
layout(location = 10) in vec4 inWorldPos;
layout(location = 11) in vec3 inNormal;

layout(location = 0) out vec4 outColor;

/** Ported from FragmentShaderArguments::sampleLayer. */
vec4 sampleLayer(int index, vec4 vertexColor, uint passType, vec3 sampleCoord)
{
    if (passType == kPassTypeColor)
        return vertexColor;

    if ((miscFlags(index) & kMiscPerspProjection) != 0)
        sampleCoord.xy /= sampleCoord.z;

    // Sampler selection by clamp mode is a per-layer decision the backend makes;
    // index 0 is the default (no clamping) until that is plumbed through.
    if (passType == kPassTypeTexture)
        return texture(sampler2D(textures2D[index], samplers[index]), sampleCoord.xy);
    if (passType == kPassTypeCubicTexture)
        return texture(samplerCube(texturesCube[index], samplers[index]), sampleCoord.xyz);

    return vec4(0.0);
}

/**
 * Folds an upper layer into the running color.
 *
 * Ported from blend (FixedPipelineShaders.metal:519-611). kBlendMask selects the
 * combiner; the remaining bits modify it.
 */
void blend(vec4 srcSample, inout vec4 destSample, uint blendFlags)
{
    if ((blendFlags & kBlendInvertColor) != 0)
        srcSample.rgb = vec3(1.0) - srcSample.rgb;

    switch (blendFlags & kBlendMask) {
    case kBlendAddColorTimesAlpha:
        // Not meaningful above the base layer; Plasma asserts on it.
        break;

    case kBlendAlpha:
        if ((blendFlags & kBlendNoTexColor) == 0) {
            if ((blendFlags & kBlendInvertAlpha) != 0)
                destSample.rgb = srcSample.rgb + (srcSample.a * destSample.rgb);
            else
                destSample.rgb = mix(destSample.rgb, srcSample.rgb, srcSample.a);
        }

        if ((blendFlags & kBlendInvertAlpha) != 0)
            srcSample.a = 1.0 - srcSample.a;

        switch (blendFlags & (kBlendAlphaAdd | kBlendAlphaMult)) {
        case kBlendAlphaAdd:
            destSample.a = srcSample.a + destSample.a;
            break;
        case kBlendAlphaMult:
            destSample.a = srcSample.a * destSample.a;
            break;
        default:
            break;
        }
        break;

    case kBlendAdd:
        destSample.rgb = srcSample.rgb + destSample.rgb;
        break;

    case kBlendMult:
        destSample.rgb = srcSample.rgb * destSample.rgb;
        break;

    case kBlendDot3:
        destSample = vec4(dot(srcSample.rgb, destSample.rgb));
        break;

    case kBlendAddSigned:
        destSample.rgb = srcSample.rgb + destSample.rgb - 0.5;
        break;

    case kBlendAddSigned2X:
        destSample.rgb = 2.0 * (srcSample.rgb + destSample.rgb - 0.5);
        break;

    case 0:
        destSample.rgb = srcSample.rgb;
        break;

    default:
        break;
    }
}

/** Ported from blendFirst (FixedPipelineShaders.metal:496-517). */
void blendFirst(vec4 srcSample, inout vec4 destSample, uint blendFlags)
{
    if ((blendFlags & kBlendInvertColor) != 0)
        srcSample.rgb = vec3(1.0) - srcSample.rgb;

    // Leave the color alone if we are blending without texture color.
    if ((blendFlags & kBlendNoTexColor) == 0)
        destSample.rgb = srcSample.rgb;

    if ((blendFlags & kBlendInvertAlpha) != 0)
        srcSample.a = 1.0 - srcSample.a;

    if ((blendFlags & kBlendNoTexAlpha) == 0)
        destSample.a = destSample.a * srcSample.a;
}

void main()
{
    // The vertex stage already lit this unless we were asked to do it per pixel.
    const vec4 lightingContribution = perPixelLighting
                                    ? calcLitMaterialColor(inColor, inWorldPos, normalize(inNormal))
                                    : inColor;
    vec4 currentColor = lightingContribution;

    // Plasma rule: a lone non-texture layer is simply the vertex color.
    if (!(numLayers == 1 && passTypes(0) == kPassTypeColor)) {
        for (int layer = 0; layer < numLayers; layer++) {
            const vec4 layerSample =
                sampleLayer(layer, inColor, passTypes(layer), inTexCoord[layer]);

            if (layer == 0) {
                // Only seed from a texture; a color-only base leaves the
                // vertex color in place.
                if (passTypes(0) != kPassTypeColor)
                    blendFirst(layerSample, currentColor, blendModes(0));
            } else {
                blend(layerSample, currentColor, blendModes(layer));
            }
        }

        currentColor = vec4(lightingContribution.rgb, 1.0) * currentColor;
    }

    currentColor.rgb = mix(inFogColor.rgb, currentColor.rgb, clamp(inFogColor.a, 0.0, 1.0));

    if (currentColor.a < material.alphaThreshold)
        discard;

    outColor = currentColor;
}
