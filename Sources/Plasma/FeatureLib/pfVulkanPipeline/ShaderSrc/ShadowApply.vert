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

// Applies a shadow map to a span that receives it.
//
// Ported from shadowCastVertexShader (FixedPipelineShaders.metal:650-693).
//
// This does not light the span; the span was already drawn lit. It computes how
// much the shadow darkens each vertex -- N-dot-L against the casting light,
// scaled by the shadow's own opacity and falloff -- and generates three sets of
// coordinates the fragment stage needs: the shadow map, the falloff LUT, and
// whichever material layer supplies the alpha.

#version 450
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_scalar_block_layout : require

#include "plVulkanShaderTypes.h"
#include "FixedFunctionVertex.glsl"

layout(scalar, set = kDescSetUniforms, binding = kBindingShadowState)
uniform ShadowBlock { plShadowState shadow; };

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outFogColor;
layout(location = 2) out vec3 outTexCoord[kMaxLayers];

void main()
{
    const vec4 position = vec4(inPosition, 1.0) * uniforms.localToWorldMatrix;
    const vec3 normal = normalize((vec4(inNormal, 0.0) * uniforms.localToWorldMatrix).xyz);

    vec3 direction;
    if (shadow.directional != 0u)
        direction = -shadow.lightDirection.xyz;
    else
        direction = normalize(shadow.lightPosition.xyz - position.xyz);

    // Clamped because a diffuse term above 1 is legal but would over-darken.
    const vec3 diffuse =
        clamp(shadow.opacity * vec3(max(0.0, dot(normal, direction))) * shadow.power, 0.0, 1.0);
    outColor = vec4(diffuse, 1.0);

    const vec4 camPosition = position * uniforms.worldToCameraMatrix;
    outFogColor = calcFog(camPosition);

    // Shadow casting does not use normals, but sampleLocation takes one.
    const vec4 cameraSpaceNormal =
        normalize((vec4(inNormal, 0.0) * uniforms.localToWorldMatrix) * uniforms.worldToCameraMatrix);

    for (int layer = 0; layer < numLayers; layer++)
        outTexCoord[layer] = sampleLocation(layer, cameraSpaceNormal, camPosition);

    gl_Position = camPosition * uniforms.projectionMatrix;
}
