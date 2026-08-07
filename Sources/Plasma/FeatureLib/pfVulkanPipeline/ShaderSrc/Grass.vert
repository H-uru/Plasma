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

// Grass. Ported from ShaderSrc/Grass.metal.
//
// Sways the top of each blade by summing four sine waves. The sine is evaluated
// as an odd-power Taylor series rather than with sin(), because the original was
// a D3D vertex shader with no transcendentals; the series coefficients arrive in
// the constant bank, so this reproduces the authored motion exactly.

#version 450
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_scalar_block_layout : require

#include "plVulkanShaderTypes.h"

// The plShader constant bank, named by register the way the shader author wrote
// it. Registers past waveSpeed exist but are unused.
layout(scalar, set = kDescSetUniforms, binding = kBindingVertexShaderConsts)
uniform GrassBlock {
    mat4 Local2NDC;
    vec4 intConstants;
    vec4 time;
    vec4 piConstants;
    vec4 sinConstants;
    vec4 waveDistortX;
    vec4 waveDistortY;
    vec4 waveDistortZ;
    vec4 waveDirX;
    vec4 waveDirY;
    vec4 waveSpeed;
};

layout(location = kVtxAttrPosition) in vec3 inPosition;
layout(location = kVtxAttrColor)    in vec4 inColor;
layout(location = kVtxAttrTexcoord) in vec3 inTexCoord[kMaxLayers];

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec3 outTexCoord;

void main()
{
    // Note this uses waveDirX for both terms, as the original does. waveDirY is
    // set by the material and never read.
    vec4 r0 = (inPosition.x * waveDirX) + (inPosition.y * waveDirX);

    r0 += time.x * waveSpeed;
    r0 = fract(r0);

    // Map 0..1 onto -pi..pi, which is where the series is accurate.
    r0 = (r0 - 0.5) * 3.14159265 * 2.0;

    const vec4 pow2 = r0 * r0;
    const vec4 pow3 = pow2 * r0;
    const vec4 pow5 = pow2 * pow3;
    const vec4 pow7 = pow2 * pow5;
    const vec4 pow9 = pow2 * pow7;

    r0 += pow3 * sinConstants.x;
    r0 += pow5 * sinConstants.y;
    r0 += pow7 * sinConstants.z;
    r0 += pow9 * sinConstants.w;

    vec3 offset = vec3(dot(r0, waveDistortX),
                       dot(r0, waveDistortY),
                       dot(r0, waveDistortZ));

    // Scaled by the V coordinate, so only the top of the blade moves.
    offset *= 2.0 * (1.0 - inTexCoord[0].y);

    outColor = inColor.bgra;
    outTexCoord = inTexCoord[0];

    gl_Position = vec4(inPosition + offset, 1.0) * Local2NDC;
}
