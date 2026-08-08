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

// Ported from shadowCastFragmentShader (FixedPipelineShaders.metal:695-738).
//
// Coordinate set 0 samples the shadow map, set 1 the falloff LUT, and set 2 the
// material layer that supplies alpha -- IRenderShadowsOntoSpan builds all three.

#version 450
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_scalar_block_layout : require

#include "plVulkanShaderTypes.h"

layout(constant_id = kSpecNumLayers) const int numLayers = 1;

// A point light's shadow map is projected, so its coordinates need the divide.
layout(constant_id = kSpecPointLightCast) const bool pointLightCast = false;

// Which material layer supplies alpha: 0, 1, or -1 for none.
layout(constant_id = kSpecShadowAlphaSrc) const int alphaSrc = -1;

layout(set = kDescSetTextures, binding = kBindingTextures2D) uniform texture2D textures2D[kMaxLayers];
layout(set = kDescSetTextures, binding = kBindingSamplers)   uniform sampler   samplers[kNumSamplerSlots];
layout(set = kDescSetTextures, binding = kBindingShadowMap)  uniform texture2D shadowMap;

layout(location = 0) in vec4 inColor;
layout(location = 1) in vec4 inFogColor;
layout(location = 2) in vec3 inTexCoord[kMaxLayers];

layout(location = 0) out vec4 outColor;

void main()
{
    vec3 sampleCoords = inTexCoord[0];
    if (pointLightCast)
        sampleCoords.xy /= sampleCoords.z;

    // The shadow map is always clamped, whatever the material's layers do.
    vec4 currentColor =
        texture(sampler2D(shadowMap, samplers[kSamplerClampBoth]), sampleCoords.xy);
    currentColor.rgb *= inColor.rgb;

    // The LUT is a ramp, so its value is just the coordinate.
    const vec4 lutColor = clamp(vec4(inTexCoord[1].x), 0.0, 1.0);

    currentColor.rgb = (1.0 - lutColor.rgb) * currentColor.rgb;
    currentColor.a = lutColor.a - currentColor.a;

    // Only layers 0 and 1 can be the alpha source. Metal routes these through
    // sampleLayer so a cubic or color layer would work; an alpha source is
    // always a 2D texture in practice, so this samples one directly.
    if (alphaSrc == 0 && numLayers > 0) {
        const vec4 layerColor =
            texture(sampler2D(textures2D[0], samplers[0]), inTexCoord[2].xy);
        currentColor.rgb *= layerColor.a * inColor.a;
    } else if (alphaSrc == 1 && numLayers > 1) {
        const vec4 layerColor =
            texture(sampler2D(textures2D[1], samplers[1]), inTexCoord[2].xy);
        currentColor.rgb *= layerColor.a * inColor.a;
    }

    if (currentColor.a <= 0.0)
        discard;

    outColor = currentColor;
}
