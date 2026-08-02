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

// Ported from ps_CompCosines (CompCosines.metal:93-133).
//
// Each input map holds cosine(pix) for one wave, as (cos, cos, 1, 1). The
// constants encode the derivative of each wave, so summing the scaled samples
// gives the surface normal:
//      Nx = -freq * amp * dirX * cos(pix)
//      Ny = -freq * amp * dirY * cos(pix)
// which makes c[i].x = -freq[i] * amp[i] * dirX[i], and so on. c0.z is 1 while
// every other c[i].z is 0; c4 and c5 bias the result back into 0..1.

#version 450
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_scalar_block_layout : require

#include "plVulkanShaderTypes.h"

layout(scalar, set = kDescSetUniforms, binding = kBindingFragmentShaderConsts)
uniform CompCosinesBlock {
    vec4 c0;
    vec4 c1;
    vec4 c2;
    vec4 c3;
    vec4 c4;
    vec4 c5;
};

layout(set = kDescSetTextures, binding = kBindingTextures2D) uniform texture2D textures2D[kMaxLayers];
layout(set = kDescSetTextures, binding = kBindingSamplers)   uniform sampler   samplers[kNumSamplerSlots];

layout(location = 0) in vec4 inTexCoord0;
layout(location = 1) in vec4 inTexCoord1;
layout(location = 2) in vec4 inTexCoord2;
layout(location = 3) in vec4 inTexCoord3;

layout(location = 0) out vec4 outColor;

void main()
{
    vec4 color = 2.0 * (texture(sampler2D(textures2D[0], samplers[0]),
                                fract(inTexCoord0.xy)) - 0.5) * c0;
    color += 2.0 * (texture(sampler2D(textures2D[1], samplers[1]),
                            fract(inTexCoord1.xy)) - 0.5) * c1;
    color += 2.0 * (texture(sampler2D(textures2D[2], samplers[2]),
                            fract(inTexCoord2.xy)) - 0.5) * c2;
    color += 2.0 * (texture(sampler2D(textures2D[3], samplers[3]),
                            fract(inTexCoord3.xy)) - 0.5) * c3;

    // Back into 0..1 so it can be stored in an 8-bit target.
    color *= c4;
    color += c5;
    color.b = 1.0;
    color.a = 1.0;

    outColor = color;
}
