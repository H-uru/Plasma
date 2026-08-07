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

// The plWaveSetShaderConsts register bank, shared by the water vertex shaders.
//
// Every one of them is handed the whole bank; which registers matter depends on
// the variant, and the ones a variant ignores still have to occupy their slots.
// Include this after `#version`, the scalar-layout extension, and
// plVulkanShaderTypes.h.

layout(scalar, set = kDescSetUniforms, binding = kBindingVertexShaderConsts)
uniform WaveConstsBlock {
    mat4 WorldToNDC;
    vec4 Frequency;
    vec4 Phase;
    vec4 Amplitude;
    vec4 DirectionX;
    vec4 DirectionY;
    vec4 Scrunch;           // unused
    vec4 SinConsts;
    vec4 CosConsts;
    vec4 PiConsts;
    vec4 NumericConsts;     // (0, 0.5, 1, 2)
    mat2x4 Tex0;
    vec4 Tex1_Row0;
    vec4 Tex1_Row1;
    mat3x4 L2W;
    vec4 Lengths;
    vec4 WaterLevel;        // per-channel water table plus offset; .w is the table
    vec4 DepthFalloff;
    vec4 MinAtten;
    vec4 Bias;              // only .x is used
    vec4 MatColor;
    vec4 CameraPos;         // DecalEnv and Fixed only
    vec4 EnvAdjust;         // DecalEnv and Fixed only
    vec4 FogSet;
    vec4 QADirX;
    vec4 QADirY;

    vec4 DirXW;             // the six below are DecalEnv and Fixed only
    vec4 DirYW;
    vec4 WK;
    vec4 DirXSqKW;
    vec4 DirXDirYKW;
    vec4 DirYSqKW;
};
