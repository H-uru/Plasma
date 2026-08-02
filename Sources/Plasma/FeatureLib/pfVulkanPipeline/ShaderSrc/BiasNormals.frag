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

// Ported from ps_BiasNormals (BiasNormals.metal:93-124).
//
// Both maps hold a normal biased into 0..1, so each is recentred by subtracting
// a half before they are summed, then scaled and biased back for storage.

#version 450
#extension GL_GOOGLE_include_directive : require

#include "plVulkanShaderTypes.h"

layout(set = kDescSetTextures, binding = kBindingTextures2D) uniform texture2D textures2D[kMaxLayers];
layout(set = kDescSetTextures, binding = kBindingSamplers)   uniform sampler   samplers[kNumSamplerSlots];

layout(location = 0) in vec4 inTexCoord0;
layout(location = 1) in vec4 inTexCoord1;
layout(location = 2) in vec4 inScale;
layout(location = 3) in vec4 inBias;

layout(location = 0) out vec4 outColor;

void main()
{
    // Both samples use coordinate set 0. That looks like a bug -- the vertex
    // stage goes to the trouble of generating a second set -- but it is what
    // Metal does, and the second map is authored to be sampled this way.
    const vec4 sample0 = texture(sampler2D(textures2D[0], samplers[0]), inTexCoord0.xy);
    const vec4 sample1 = texture(sampler2D(textures2D[1], samplers[1]), inTexCoord0.xy);

    vec4 color = vec4(sample0.rgb - 0.5 + sample1.rgb - 0.5, sample0.a + sample1.a);
    color.rgb = (color.rgb * inScale.rgb) + inBias.rgb;

    outColor = color;
}
