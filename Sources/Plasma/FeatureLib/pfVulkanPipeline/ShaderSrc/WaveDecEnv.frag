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

// Ported from ps_WaveDecEnv (WaveDecEnv.metal:367-398).
//
// The three coordinate sets are the rows of texture-to-world with the eye ray in
// their w components, so a normal-map sample becomes a world-space normal by
// three dot products.

#version 450
#extension GL_GOOGLE_include_directive : require

#include "plVulkanShaderTypes.h"

layout(set = kDescSetTextures, binding = kBindingTextures2D)   uniform texture2D   textures2D[kMaxLayers];
layout(set = kDescSetTextures, binding = kBindingTexturesCube) uniform textureCube texturesCube[kMaxLayers];
layout(set = kDescSetTextures, binding = kBindingSamplers)     uniform sampler     samplers[kNumSamplerSlots];

layout(location = 0) in vec4 inColor;
layout(location = 1) in vec4 inTexCoord0;
layout(location = 2) in vec4 inTexCoord1;
layout(location = 3) in vec4 inTexCoord2;
layout(location = 4) in vec4 inTexCoord3;
layout(location = 5) in float inFog;

layout(location = 0) out vec4 outColor;

void main()
{
    // The normal map is biased into 0..1, so recentre it.
    const vec4 normalSample =
        2.0 * (texture(sampler2D(textures2D[0], samplers[0]), inTexCoord0.xy) - 0.5);

    const vec3 N = vec3(dot(inTexCoord1.xyz, normalSample.xyz),
                        dot(inTexCoord2.xyz, normalSample.xyz),
                        dot(inTexCoord3.xyz, normalSample.xyz));

    const vec3 E = vec3(inTexCoord1.w, inTexCoord2.w, inTexCoord3.w);

    // E points from the surface to the eye, so it is negated to become the
    // incident ray before reflecting.
    const vec3 coord = reflect(-E, N);

    // Attenuation was already folded into the vertex color, so the reflection
    // just takes it as-is. Alpha comes from the normal map's own alpha.
    vec4 color = texture(samplerCube(texturesCube[1], samplers[1]), coord);
    color.rgb *= inColor.rgb;
    color.a = normalSample.a * inColor.a;

    outColor = color;
}
