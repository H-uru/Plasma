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

// The main water surface. Ported from vs_WaveFixedFin7 (WaveSet7.metal:94-306).
//
// Same four-wave Gerstner displacement and analytic tangent basis as
// WaveDecEnv.vert, with two additions: a distance-based specular attenuation
// folded into the basis rows, and a Fresnel-ish term that decides how much of the
// environment reflection survives versus the flat water tint.
//
// Note this shader has its own constant bank, not plWaveSetShaderConsts: it adds
// WaterTint, UVScale, SpecAtten and EnvTint, and calls the transform
// LocalToWorld rather than L2W.

#version 450
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_scalar_block_layout : require

#include "plVulkanShaderTypes.h"

layout(scalar, set = kDescSetUniforms, binding = kBindingVertexShaderConsts)
uniform WaveFixedBlock {
    mat4 WorldToNDC;
    vec4 WaterTint;
    vec4 Frequency;
    vec4 Phase;
    vec4 Amplitude;
    vec4 DirectionX;
    vec4 DirectionY;
    vec4 UVScale;
    vec4 SpecAtten;
    vec4 Scrunch;
    vec4 SinConsts;
    vec4 CosConsts;
    vec4 PiConsts;
    vec4 NumericConsts;     // (0, 0.5, 1, 2)
    vec4 CameraPos;
    vec4 WindRot;
    vec4 EnvAdjust;
    vec4 EnvTint;
    mat3x4 LocalToWorld;
    vec4 Lengths;
    vec4 WaterLevel;
    vec4 DepthFalloff;
    vec4 MinAtten;
    vec4 FogSet;
    vec4 DirXK;
    vec4 DirYK;
    vec4 DirXW;
    vec4 DirYW;
    vec4 WK;
    vec4 DirXSqKW;
    vec4 DirXDirYKW;
    vec4 DirYSqKW;
};

layout(location = kVtxAttrPosition) in vec3 inPosition;
layout(location = kVtxAttrColor)    in vec4 inColor;

layout(location = 0) out vec4 outColor1;
layout(location = 1) out vec4 outColor2;
layout(location = 2) out vec4 outTexCoord0;
layout(location = 3) out vec4 outTexCoord1;
layout(location = 4) out vec4 outTexCoord2;
layout(location = 5) out vec4 outTexCoord3;
layout(location = 6) out float outFog;

void main()
{
    vec4 worldPosition = vec4(vec4(inPosition, 1.0) * LocalToWorld, 1.0);

    // inColor is filter data, not a color, and deliberately unswizzled. See
    // WaveRip.vert.
    vec4 phases = DirectionX * worldPosition.xxxx;
    phases = (DirectionY * worldPosition.yyyy) + phases;
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

    const vec4 cosPhases = cosPhasesRaw * Amplitude * waveFilter;
    worldPosition.x += dot(cosPhases, DirXK);
    worldPosition.y += dot(cosPhases, DirYK);

    //
    // Surface-to-world, from the wave derivatives. Same construction as
    // WaveDecEnv.vert, but note the z column's signs differ: here rowX.z and
    // rowY.z take -DirXW / -DirYW while rowZ takes +DirXW / +DirYW.
    //
    vec4 rowX = vec4(0.0);
    vec4 rowY = vec4(0.0);
    vec4 rowZ = vec4(0.0);

    rowX.x = dot(sinPhases, -DirXSqKW) + NumericConsts.z;
    rowX.y = dot(sinPhases, -DirXDirYKW);
    rowX.z = dot(cosPhases, -DirXW);

    rowY.x = dot(sinPhases, -DirXDirYKW);
    rowY.y = dot(sinPhases, -DirYSqKW) + NumericConsts.z;
    rowY.z = dot(cosPhases, -DirYW);

    rowZ.x = dot(cosPhases, DirXW);
    rowZ.y = dot(cosPhases, DirYW);
    rowZ.z = dot(sinPhases, -WK) + NumericConsts.z;

    //
    // Eye ray, and the specular attenuation that rides on its length.
    //
    vec4 eye = worldPosition - CameraPos;
    const float invLength = inversesqrt(dot(eye.xyz, eye.xyz));
    eye = eye * vec4(invLength);

    float specAtten = 1.0 / invLength;     // the distance
    specAtten += SpecAtten.x;
    specAtten *= SpecAtten.y;
    specAtten = clamp(specAtten, NumericConsts.x, NumericConsts.z);
    specAtten *= specAtten;                // squared, for perspective
    specAtten *= SpecAtten.z;

    // The environment-map centre offset, as in WaveDecEnv.vert. The abs() calls
    // are Metal's; the discriminant can go slightly negative on a grazing ray.
    vec3 envRay;
    {
        const vec3 D = eye.xyz;
        const vec3 F = EnvAdjust.xyz;
        const float G = EnvAdjust.w;
        const float DdotF = dot(D, F);
        const float t = DdotF + sqrt(abs(pow(abs(DdotF), 2.0) - G));
        envRay = (D * t) - F;
    }
    envRay = normalize(envRay);

    rowX.w = -envRay.x;
    rowY.w = -envRay.y;
    rowZ.w = -envRay.z;

    //
    // Normalize each row, scaling x and y by the specular attenuation but z by the
    // normalization alone -- so the reflection dims with distance while the term
    // collected in `normalZ` stays a true normal.
    //
    vec3 normalZ;

    float invLen = inversesqrt(dot(rowX.xyz, rowX.xyz));
    outTexCoord1 = rowX * vec4(invLen * specAtten, invLen * specAtten, invLen, 1.0);
    normalZ.x = rowX.z * invLen;

    invLen = inversesqrt(dot(rowY.xyz, rowY.xyz));
    outTexCoord3 = rowY * vec4(invLen * specAtten, invLen * specAtten, invLen, 1.0);
    normalZ.y = rowY.z * invLen;

    invLen = inversesqrt(dot(rowZ.xyz, rowZ.xyz));
    outTexCoord2 = rowZ * vec4(invLen * specAtten, invLen * specAtten, invLen, 1.0);
    normalZ.z = rowZ.z * invLen;

    gl_Position = worldPosition * WorldToNDC;
    outFog = (gl_Position.w + FogSet.x) * FogSet.y;

    // The normal map tiles over object space, not over a UV channel.
    outTexCoord0 = vec4(inPosition.xy * UVScale.x, 0.0, 1.0);

    //
    // How much reflection survives. The dot of the incident ray against the
    // surface normal is the Fresnel-ish term; one minus it goes to the color, and
    // a half of two minus it goes to alpha.
    //
    // Metal indexes inColor.z here, which on this unswizzled attribute is the red
    // channel -- overall transparency. The original D3D shader's v5.z would have
    // been blue, the wave-scaling channel. Reproduced as Metal has it rather than
    // as the register naming suggests, because Metal is what this is a port of.
    //
    float fresnel = dot(-eye.xyz, normalZ) * inColor.z;

    vec4 tint = vec4(NumericConsts.z - fresnel);
    tint.w += NumericConsts.z;
    tint.w *= NumericConsts.y;
    tint *= depth.yyyx;
    tint.w *= inColor.y;
    tint.w *= WaterTint.w;

    outColor1 = clamp(tint * EnvTint, 0.0, 1.0);
    outColor2 = WaterTint;
}
