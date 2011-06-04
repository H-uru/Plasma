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
#ifndef plMaxMeshExtractor_h_inc
#define plMaxMeshExtractor_h_inc

#include "hsTypes.h"
#include "hsMatrix44.h"

class plMaxNode;

class plMaxMeshExtractor
{
public:
	struct NeutralMesh
	{
		int fNumVerts;
		hsPoint3* fVerts;
		int fNumFaces;
		UInt16* fFaces;

		hsMatrix44 fL2W;
	};

	// Converts a max node into a position, rotation, and arrays of verts and faces
	// relative to those.
	static bool Extract(NeutralMesh& mesh, plMaxNode* pNode, bool makeAABB = false, plMaxNode* sOwningNode = nil);
};

#endif // plMaxMeshExtractor_h_inc