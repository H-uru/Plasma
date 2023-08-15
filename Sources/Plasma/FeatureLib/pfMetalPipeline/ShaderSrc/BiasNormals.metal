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

#include <metal_stdlib>
using namespace metal;

#include "ShaderVertex.h"

typedef struct  {
    float4 TexU0;
    float4 TexV0;

    float4 TexU1;
    float4 TexV1;

    float4 Numbers;

    float4 ScaleBias;
} vs_BiasNormalsUniforms;
    
typedef struct {
    float4 position [[position]];
    float4 texCoord0;
    float4 texCoord1;
    //not actually colors, just emulating the registers
    float4 color1;
    float4 color2;
} vs_BiasNormalsOut;

vertex vs_BiasNormalsOut vs_BiasNormals(Vertex in [[stage_in]],
                               constant vs_BiasNormalsUniforms & uniforms [[ buffer(BufferIndexUniforms) ]]) {
    vs_BiasNormalsOut out;
    
    out.position = float4(in.position, 1.0);
    
    out.texCoord0 = float4(
                           dot(float4(in.texCoord1, 1.0), uniforms.TexU0),
                           dot(float4(in.texCoord1, 1.0), uniforms.TexV0),
                           0,
                           1
                           );
    
    out.texCoord1 = float4(
                           dot(float4(in.texCoord1, 1.0), uniforms.TexU1),
                           dot(float4(in.texCoord1, 1.0), uniforms.TexV1),
                           0,
                           1
                           );
    
    out.color1 = uniforms.ScaleBias.xxzz;
    out.color2 = uniforms.ScaleBias.yyzz;
    
    return out;
}

fragment float4 ps_BiasNormals(vs_BiasNormalsOut in [[stage_in]],
                             texture2d<float> t0 [[ texture(0) ]],
                             texture2d<float> t1 [[ texture(1) ]]) {
    // Composite the cosines together.
    // Input map is cosine(pix) for each of
    // the 4 waves.
    //
    // The constants are set up so:
    //      Nx = -freq * amp * dirX * cos(pix);
    //      Ny = -freq * amp * dirY * cos(pix);
    //  So c[i].x = -freq[i] * amp[i] * dirX[i]
    //  etc.
    // All textures are:
    //      (r,g,b,a) = (cos(), cos(), 1, 1)
    //
    // So c[0].z = 1, but all other c[i].z = 0
    // Note also the c4 used for biasing back at the end.
    
    constexpr sampler colorSampler = sampler(mip_filter::linear,
                              mag_filter::linear,
                              min_filter::linear,
                              address::repeat);
    float4 sample1 = t0.sample(colorSampler, in.texCoord0.xy);
    float4 sample2 = t1.sample(colorSampler, in.texCoord0.xy);
    float4 out = float4(sample1.rgb - 0.5 + sample2.rgb - 0.5,
                        sample1.a + sample2.a);
    out.rgb = (out.rgb * in.color1.rgb) + in.color2.rgb;
    return out;
}
