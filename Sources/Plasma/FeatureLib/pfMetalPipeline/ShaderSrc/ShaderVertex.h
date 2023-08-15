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
#include "ShaderTypes.h"

constant ushort num_uvs        [[ function_constant(FunctionConstantNumUVs) ]];
constant bool hasTexture1 = num_uvs > 0;
constant bool hasTexture2 = num_uvs > 1;
constant bool hasTexture3 = num_uvs > 2;
constant bool hasTexture4 = num_uvs > 3;
constant bool hasTexture5 = num_uvs > 4;
constant bool hasTexture6 = num_uvs > 5;
constant bool hasTexture7 = num_uvs > 6;
constant bool hasTexture8 = num_uvs > 7;

constant ushort num_layers        [[ function_constant(FunctionConstantNumLayers) ]];
constant bool hasLayer1 = num_layers > 0;
constant bool hasLayer2 = num_layers > 1;
constant bool hasLayer3 = num_layers > 2;
constant bool hasLayer4 = num_layers > 3;
constant bool hasLayer5 = num_layers > 4;
constant bool hasLayer6 = num_layers > 5;
constant bool hasLayer7 = num_layers > 6;
constant bool hasLayer8 = num_layers > 7;

typedef struct
{
    float3 position [[attribute(VertexAttributePosition)]];
    float3 normal [[attribute(VertexAttributeNormal)]];
    float3 texCoord1 [[attribute(VertexAttributeTexcoord), function_constant(hasTexture1)]];
    float3 texCoord2 [[attribute(VertexAttributeTexcoord+1), function_constant(hasTexture2)]];
    float3 texCoord3 [[attribute(VertexAttributeTexcoord+2), function_constant(hasTexture3)]];
    float3 texCoord4 [[attribute(VertexAttributeTexcoord+3), function_constant(hasTexture4)]];
    float3 texCoord5 [[attribute(VertexAttributeTexcoord+4), function_constant(hasTexture5)]];
    float3 texCoord6 [[attribute(VertexAttributeTexcoord+5), function_constant(hasTexture6)]];
    float3 texCoord7 [[attribute(VertexAttributeTexcoord+6), function_constant(hasTexture7)]];
    float3 texCoord8 [[attribute(VertexAttributeTexcoord+7), function_constant(hasTexture8)]];
    uchar4 color [[attribute(VertexAttributeColor)]];
} Vertex;
