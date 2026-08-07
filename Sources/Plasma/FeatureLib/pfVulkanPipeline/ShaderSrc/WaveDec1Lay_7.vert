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

// Water with one decal layer. Used in Ahnonay, at the edge of the sphere.
//
// Ported from vs_WaveDec1Lay_7 (WaveDec1Lay_7.metal:93-231). The wave
// displacement is the same four-wave Gerstner sum as WaveRip; what differs is
// that the horizontal term is scaled by amplitude as well as by the filter, and
// that the output color comes from the vertex channels rather than a decal fade.

#version 450
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_scalar_block_layout : require

#include "plVulkanShaderTypes.h"
#include "WaveConsts.glsl"

layout(location = kVtxAttrPosition) in vec3 inPosition;
layout(location = kVtxAttrColor)    in vec4 inColor;
layout(location = kVtxAttrTexcoord) in vec3 inTexCoord[kMaxLayers];

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec2 outTexCoord;
layout(location = 2) out float outFog;

void main()
{
    vec4 worldPosition = vec4(vec4(inPosition, 1.0) * L2W, 1.0);

    // inColor is filter data, not a color, and is deliberately unswizzled: the
    // attribute arrives as (b, g, r, a) and the channels are indexed in that
    // order. See WaveRip.vert for what each one carries.
    vec4 phases = DirectionX * worldPosition.xxxx;
    phases += DirectionY * worldPosition.yyyy;
    phases = (phases * Frequency) + Phase;

    const vec4 sinPhasesRaw = sin(phases);
    const vec4 cosPhasesRaw = cos(phases);

    vec4 depth = WaterLevel - worldPosition.zzzz;
    depth *= DepthFalloff;
    depth += MinAtten;
    depth = clamp(depth, 0.0, 1.0);

    // NumericConsts.x is 0 and .z is 1; the original had no clamp instruction.
    vec4 waveFilter = inColor.wwww * Lengths;
    waveFilter = max(waveFilter, NumericConsts.xxxx);
    waveFilter = min(waveFilter, NumericConsts.zzzz);

    const vec4 sinPhases = sinPhasesRaw * waveFilter * Amplitude;

    vec4 accumPos = vec4(0.0);
    accumPos.x = dot(sinPhases, NumericConsts.zzzz);
    accumPos.y = accumPos.x * depth.z;
    accumPos.z = accumPos.y + WaterLevel.w;
    worldPosition.z = max(worldPosition.z, accumPos.z);

    // Unlike WaveRip, the horizontal term picks up Amplitude here as well; QADirX
    // and QADirY carry only k * dir for this shader.
    const vec4 cosPhases = cosPhasesRaw * Amplitude * waveFilter;
    worldPosition.xy += vec2(dot(cosPhases, QADirX), dot(cosPhases, QADirY));

    worldPosition.z += Bias.x;

    gl_Position = worldPosition * WorldToNDC;
    outFog = (gl_Position.w + FogSet.x) * FogSet.y;

    // Color is the green channel, alpha is red -- vertex alpha is spoken for by
    // the wave filtering. The whole thing takes the material's color and opacity.
    outColor = inColor.yyyz * MatColor;

    outTexCoord = vec4(inTexCoord[0], 1.0) * Tex0;
}
