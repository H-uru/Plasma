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

// Swim ripples. Used in Kemo when the player is in the water.
//
// Ported from vs_WaveRip7 (WaveRip.metal:96-263). The original is a D3D vs.1.1
// shader and the comments in the Metal source trace its register use; the
// substance is a four-wave Gerstner displacement, depth-attenuated, plus the
// dynamic-decal age fade that makes a ripple appear and die.

#version 450
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_scalar_block_layout : require

#include "plVulkanShaderTypes.h"

// One register per field, in the order plWaveSetShaderConsts declares them.
// L2WUnused exists only to keep the registers after it in the right place:
// ripple is the one shader where kLocalToWorld and kL2WRow0 have different
// indices, which looks like a slip when the constants were numbered.
layout(scalar, set = kDescSetUniforms, binding = kBindingVertexShaderConsts)
uniform WaveRipBlock {
    mat4 WorldToNDC;
    vec4 FogSet;
    vec4 Frequency;
    vec4 Phase;
    vec4 Amplitude;
    vec4 DirectionX;
    vec4 DirectionY;
    vec4 QADirX;
    vec4 QADirY;
    vec4 Scrunch;
    vec4 SinConsts;
    vec4 CosConsts;
    vec4 PiConsts;
    vec4 NumericConsts;
    vec4 CameraPos;
    vec4 WindRot;
    vec4 Tex0_Row0;
    vec4 Tex0_Row1;
    vec4 Tex0_Row2;
    vec4 Tex1_Row0;
    vec4 Tex1_Row1;
    vec4 Tex1_Row2;
    mat3x4 L2W;
    vec4 L2WUnused;
    vec4 Lengths;
    vec4 WaterLevel;
    vec4 DepthFalloff;
    vec4 MinAtten;
    vec4 TexConsts;
    vec4 LifeConsts;
    vec4 RampBias;
};

layout(location = kVtxAttrPosition) in vec3 inPosition;
layout(location = kVtxAttrColor)    in vec4 inColor;
layout(location = kVtxAttrTexcoord) in vec3 inTexCoord[kMaxLayers];

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec2 outTexCoord;
layout(location = 2) out float outFog;

void main()
{
    vec4 worldPosition = vec4(vec4(inPosition, 1.0) * L2W, 1.0);

    // The vertex color is not a color here. It carries the per-vertex filtering
    // the exporter computed:
    //      r = overall transparency
    //      g = illumination
    //      b = overall wave scaling
    //      a = 1 / (2 * edge length)
    // so a wave starts dying out at four times the sampling frequency and is
    // gone at twice it.
    //
    // Note this attribute is NOT swizzled the way a real vertex color is: Plasma
    // stores ARGB little-endian, so inColor arrives as (b, g, r, a) and these
    // shaders index it in that order, exactly as the Metal originals do.

    vec4 phases = DirectionX * worldPosition.xxxx;
    phases += DirectionY * worldPosition.yyyy;
    phases = (phases * Frequency) + Phase;

    const vec4 sinPhasesRaw = sin(phases);
    const vec4 cosPhasesRaw = cos(phases);

    // Depth attenuation. WaterLevel already has each channel's offset folded in,
    // so this is: (waterLevel + offset - z) / depthFalloff + minAtten, per
    // channel, which fades transparency, reflection and wave scale out
    // independently as the water shallows.
    vec4 depth = WaterLevel - worldPosition.zzzz;
    depth *= DepthFalloff;
    depth += MinAtten;
    depth = clamp(depth, 0.0, 1.0);

    vec4 waveFilter = clamp(inColor.wwww * Lengths, 0.0, 1.0);

    vec4 sinPhases = sinPhasesRaw * waveFilter * Amplitude;

    // Height is the sum of the amplitudes, damped by depth and lifted to the
    // water table, then clamped so the surface never sinks below the ground.
    vec4 accumPos = vec4(0.0);
    accumPos.x = dot(sinPhases, NumericConsts.zzzz);
    accumPos.y = accumPos.x * depth.z;
    accumPos.z = accumPos.y + WaterLevel.w;
    worldPosition.z = max(worldPosition.z, accumPos.z);

    // The horizontal part of the Gerstner displacement.
    const vec4 cosPhases = cosPhasesRaw * waveFilter;
    worldPosition.xy += vec2(dot(cosPhases, QADirX), dot(cosPhases, QADirY));

    // Nudged up to cover precision loss: the filter coefficients arrive as
    // interpolated bytes, so there is real slop in them.
    worldPosition.z += RampBias.z;

    gl_Position = worldPosition * WorldToNDC;
    outFog = (gl_Position.w + FogSet.x) * FogSet.y;

    // The decal's own fade. Birth time rides in the third UV component.
    const float age = LifeConsts.y - inTexCoord[0].z;
    const float atten = clamp(age * RampBias.y, 0.0, 1.0) *
                        clamp((LifeConsts.z - age) * LifeConsts.w, 0.0, 1.0);

    outColor = (depth.y * LifeConsts.x) * vec4(atten, atten, atten, 1.0);

    // The ripple spreads as it ages: UV = (UV - 0.5) * scale + 0.5, with
    // scale = C1 / (age * C2 + 1).
    vec2 scale = age * TexConsts.yw;
    scale += 1.0;
    scale = 1.0 / scale;
    scale *= TexConsts.xz;

    outTexCoord = (inTexCoord[0].xy - 0.5) * scale + 0.5;
}
