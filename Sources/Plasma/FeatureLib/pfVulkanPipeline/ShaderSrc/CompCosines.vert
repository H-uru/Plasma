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

// Composites four cosine waves into a normal map for the water.
//
// Ported from vs_CompCosines (CompCosines.metal:65-82). This runs over a full
// screen quad already in clip space, so there is no transform; all it does is
// generate four sets of UVs from one input channel.

#version 450
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_scalar_block_layout : require

#include "plVulkanShaderTypes.h"

layout(scalar, set = kDescSetUniforms, binding = kBindingVertexShaderConsts)
uniform CompCosinesBlock {
    vec4 c0;
    vec4 c1;
    vec4 c2;
    vec4 c3;
    vec4 c4;
};

layout(location = kVtxAttrPosition) in vec3 inPosition;
layout(location = kVtxAttrTexcoord) in vec3 inTexCoord[kMaxLayers];

layout(location = 0) out vec4 outTexCoord0;
layout(location = 1) out vec4 outTexCoord1;
layout(location = 2) out vec4 outTexCoord2;
layout(location = 3) out vec4 outTexCoord3;

void main()
{
    const vec4 uvw = vec4(inTexCoord[0], 1.0);

    // Only x varies; y stays zero, so each wave samples along one axis.
    outTexCoord0 = vec4(dot(uvw, c0), 0.0, 0.0, 1.0);
    outTexCoord1 = vec4(dot(uvw, c1), 0.0, 0.0, 1.0);
    outTexCoord2 = vec4(dot(uvw, c2), 0.0, 0.0, 1.0);
    outTexCoord3 = vec4(dot(uvw, c3), 0.0, 0.0, 1.0);

    gl_Position = vec4(inPosition, 1.0);
}
