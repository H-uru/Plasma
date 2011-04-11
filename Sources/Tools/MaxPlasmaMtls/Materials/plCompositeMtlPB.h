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

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/
#ifndef PL_COMPOSITE_MTL_PB_H
#define PL_COMPOSITE_MTL_PB_H

enum
{
	kCompPasses,
	kCompOn,
	kCompBlend,
	kCompUVChannels,
	kCompLayerCounts
};

// Make sure to pair up each blend mode with an inverse after it
// (This way we check for an inverse blend by doing an odd/even check.)
enum BlendMethod // These should match up in order with the blend strings
{
	kCompBlendVertexAlpha,
	kCompBlendInverseVtxAlpha,
	kCompBlendVertexIllumRed,
	kCompBlendInverseVtxIllumRed,
	kCompBlendVertexIllumGreen,
	kCompBlendInverseVtxIllumGreen,
	kCompBlendVertexIllumBlue,
	kCompBlendInverseVtxIllumBlue,
	kCompNumBlendMethods
};	

static char *BlendStrings[] = // Make sure these match up in order with the Blend enum
{
	"Vertex Alpha",
	"Inverse Vtx Alpha",
	"Vertex Illum Red",
	"Inv. Vtx Illum Red",
	"Vertex Illum Green",
	"Inv. Vtx Illum Green",
	"Vertex Illum Blue",
	"Inv. Vtx Illum Blue"
};

#endif //PL_COMPOSITE_MTL_PB_H