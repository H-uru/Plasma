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

// Renders a shadow caster into a shadow map.
//
// Ported from shadowVertexShader (FixedPipelineShaders.metal:613-627). The map
// stores distance-from-light in alpha: uvTransforms[0] is set up by
// plVulkanPipeline::IPushShadowCastState to project camera-space position onto the
// light's axis, and the x component of the result is that distance.

#version 450
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_scalar_block_layout : require

#include "plVulkanShaderTypes.h"

layout(scalar, set = kDescSetUniforms, binding = kBindingVertexUniforms)
uniform VertexUniformsBlock { VertexUniforms uniforms; };

layout(location = kVtxAttrPosition) in vec3 inPosition;

layout(location = 0) out vec3 outTexCoord;

void main()
{
    const vec4 camPosition =
        (vec4(inPosition, 1.0) * uniforms.localToWorldMatrix) * uniforms.worldToCameraMatrix;

    outTexCoord = (camPosition * uniforms.uvTransforms[0].transform).xyz;
    gl_Position = camPosition * uniforms.projectionMatrix;
}
