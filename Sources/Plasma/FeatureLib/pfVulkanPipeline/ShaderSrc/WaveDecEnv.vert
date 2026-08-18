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

// Water with an environment-mapped decal layer. Used in Ahnonay, around the
// edges of the island.
//
// Ported from vs_WaveDecEnv_7 (WaveDecEnv.metal:99-365).
//
// The displacement is the same four-wave Gerstner sum as WaveDec1Lay_7. What this
// adds is a per-vertex tangent basis derived analytically from the wave
// derivatives, so the fragment stage can turn a normal-map sample into a world
// reflection vector. Writing out the sums the original derives:
//
//      T = sum(A sin())                     the height
//      S = sum(k dirX A cos())              horizontal displacement, x
//      R = sum(k dirY A cos())              horizontal displacement, y
//      W = sum(k w dirX^2 A sin())
//      V = sum(k w dirX dirY A sin())
//      U = sum(k w dirY^2 A sin())
//      P = sum(w dirX A cos())
//      N = sum(w dirY A cos())
//      Q = sum(k w A cos())
//
// giving the basis
//      Bin = (1 - W,   -V,      P)
//      Tan = (-V,      1 - U,   N)
//      Nor = (-P,      -N,      1 - Q)
//
// which is surface-to-world. Texture-to-surface comes from the authored partials
// dPos/dU and dPos/dV in UV channels 1 and 2, plus their cross product. The two
// are composed here and handed to the fragment stage as three rows, with the
// eye-ray vector packed into their w components.

#version 450
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_scalar_block_layout : require

#include "plVulkanShaderTypes.h"
#include "WaveConsts.glsl"

layout(location = kVtxAttrPosition) in vec3 inPosition;
layout(location = kVtxAttrColor)    in vec4 inColor;
layout(location = kVtxAttrTexcoord) in vec3 inTexCoord[kMaxLayers];

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outTexCoord0;
layout(location = 2) out vec4 outTexCoord1;
layout(location = 3) out vec4 outTexCoord2;
layout(location = 4) out vec4 outTexCoord3;
layout(location = 5) out float outFog;

void main()
{
    vec4 worldPosition = vec4(vec4(inPosition, 1.0) * L2W, 1.0);

    // inColor is filter data, not a color, and deliberately unswizzled. See
    // WaveRip.vert.
    vec4 phases = DirectionX * worldPosition.xxxx;
    phases += DirectionY * worldPosition.yyyy;
    phases = (phases * Frequency) + Phase;

    const vec4 sinPhasesRaw = sin(phases);
    const vec4 cosPhasesRaw = cos(phases);

    vec4 depth = WaterLevel - worldPosition.zzzz;
    depth *= DepthFalloff;
    depth += MinAtten;
    depth = clamp(depth, 0.0, 1.0);

    vec4 waveFilter = inColor.wwww * Lengths;
    waveFilter = max(waveFilter, NumericConsts.xxxx);
    waveFilter = min(waveFilter, NumericConsts.zzzz);

    const vec4 sinPhases = sinPhasesRaw * waveFilter * Amplitude;

    vec4 accumPos = vec4(0.0);
    accumPos.x = dot(sinPhases, NumericConsts.zzzz);
    accumPos.y = accumPos.x * depth.z;
    accumPos.z = accumPos.y + WaterLevel.w;
    worldPosition.z = max(worldPosition.z, accumPos.z);

    vec4 cosPhases = cosPhasesRaw * waveFilter;
    worldPosition.xy += vec2(dot(cosPhases, QADirX), dot(cosPhases, QADirY));

    worldPosition.z += Bias.x;

    gl_Position = worldPosition * WorldToNDC;
    outFog = (gl_Position.w + FogSet.x) * FogSet.y;

    outTexCoord0 = vec4(vec4(inTexCoord[0], 1.0) * Tex0, 0.0, 0.0);

    //
    // Texture-to-surface: the authored partials and their cross product.
    //
    const vec3 dPosDu = inTexCoord[1];
    const vec3 dPosDv = inTexCoord[2];
    const vec3 basisZ = cross(dPosDu, dPosDv);

    // Everything below wants cosine times amplitude.
    cosPhases *= Amplitude;

    // Rows of surface-to-world, as laid out in the comment above.
    vec3 rowX;
    rowX.x = dot(sinPhases, -DirXSqKW) + NumericConsts.z;
    rowX.y = dot(sinPhases, -DirXDirYKW);
    rowX.z = dot(cosPhases, -DirXW);

    vec3 rowY;
    rowY.x = dot(sinPhases, -DirXDirYKW);
    rowY.y = dot(sinPhases, -DirYSqKW) + NumericConsts.z;
    rowY.z = dot(cosPhases, -DirYW);

    vec3 rowZ;
    rowZ.x = -rowX.z;
    rowZ.y = -rowY.z;
    rowZ.z = dot(cosPhases, -WK) + NumericConsts.z;

    // Compose the two, giving texture-to-world.
    vec4 texToWorldX = vec4(dot(rowX, dPosDu), dot(rowX, dPosDv), dot(rowX, basisZ), 0.0);
    vec4 texToWorldY = vec4(dot(rowY, dPosDu), dot(rowY, dPosDv), dot(rowY, basisZ), 0.0);
    vec4 texToWorldZ = vec4(dot(rowZ, dPosDu), dot(rowZ, dPosDv), dot(rowZ, basisZ), 0.0);

    //
    // The eye ray, adjusted for the environment map's centre not being at the
    // camera. Derivation is in vs_WaveFixedFin6.inl.
    //
    vec4 eye = worldPosition - CameraPos;
    eye *= inversesqrt(dot(eye.xyz, eye.xyz));

    const float alongAdjust = dot(eye.xyz, EnvAdjust.xyz);
    const float discriminant = (alongAdjust * alongAdjust) - EnvAdjust.w;
    const float scale = (discriminant * inversesqrt(discriminant)) + alongAdjust;

    eye.xyz = (eye.xyz * scale) - EnvAdjust.xyz;

    // Renormalized: the ATI 9000 could not cope with the vector as computed.
    eye.xyz = normalize(eye.xyz);

    texToWorldX.w = -eye.x;
    texToWorldY.w = -eye.y;
    texToWorldZ.w = -eye.z;

    // Normalized and handed over. Note Y and Z are swapped on the way out: the
    // environment map is flipped relative to D3D, and to expectation.
    outTexCoord1 = texToWorldX * vec4(vec3(inversesqrt(dot(texToWorldX.xyz, texToWorldX.xyz))), 1.0);
    outTexCoord2 = texToWorldZ * vec4(vec3(inversesqrt(dot(texToWorldZ.xyz, texToWorldZ.xyz))), 1.0);
    outTexCoord3 = texToWorldY * vec4(vec3(inversesqrt(dot(texToWorldY.xyz, texToWorldY.xyz))), 1.0);

    outColor = clamp(inColor.yyyz * MatColor, 0.0, 1.0);
}
