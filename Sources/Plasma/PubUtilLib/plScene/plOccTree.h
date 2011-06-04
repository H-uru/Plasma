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

#ifndef plOccTree_inc
#define plOccTree_inc

#include "hsTemplates.h"
#include "hsGeometry3.h"

class plBoundsHierarchy;

class plOccPlane
{
public:
	hsVector3			fNormal;
	hsScalar			fDist;

};

class plOccPoly
{
public:
	enum {
		kEdgeClipped		= 0x1
	};

	plOccPlane					fPlane;

	hsTArray<hsPoint3>			fVerts;
	hsTArray<UInt8>				fEdgeFlags; // flag[i] => edge(fVerts[i], fVerts[(i+1)%n])
};

class plOccNode
{
protected:
	enum {
		kNone			= 0x0,
		kIsLeaf			= 0x1
	};
	enum {
		kAllIn			= 0x0,
		kAllOut			= 0x1,
		kSplit			= 0x2
	};

	UInt32				fFlags;

	plOccPlane			fPolyPlane; // Plane of the poly we came from
	plOccPlane			fViewPlane; // Plane perp to view dir. 
	// For an interior node, ViewPlane is for the nearest (to view) point
	// on the poly. A bound closer than that will not be occluded by this
	// node or any nodes deeper in the tree.
	// For a leaf it's the farthest point on the poly. A bound inside this
	// plane OR the PolyPlane is occluded.

	plOccNode*		fInChild;
	
	plOccNode*		fOutChild;
};

class plOccTree
{
protected:

	enum {
		kNone			= 0x0,
		kNeedsBuild		= 0x1
	};

	UInt8					fFlags;

	// Temp pools for building our trees each frame.
	hsTArray<plOccPoly>		fPolyPool;
	hsTArray<plOccPoly>		fBasePolys;

	// The BSP used to add our polys front to back. This BSP is constant.
	plOccNode*			fBSP;

	// This current frame's view pos and occluder tree.
	plOccNode*			fRoot;
	hsPoint3			fViewPos;


	plOccNode*			IAddPolyRecur(plOccNode* n, plOccPoly* poly);

	void				ITrimPoly(plOccPlane& plane, plOccPoly* polyIn, plOccPoly*& polyIn, plOccPoly*& polyOut);

	plOccNode*			IBuildOccTree();

public:

	plOccTree() : fFlags(kNone), fBSP(nil), fRoot(nil) {}
	~plOccTree() {}

	// We'll take in the view position (for sorting and building).
	// The view direction isn't necessary, but may be useful for
	// selecting a subset of occluders (like don't bother with ones parallel to the view dir).
	// What we really want is to pass in the viewport walls, or all the clip planes to initialize
	// the occtree, then occluders out of view are automatically pruned, and the single test
	// does the full view/portal/occluder test.
	void SetView(const hsPoint3& pos, const hsVector3& dir);


	// The algorithm is:
	//	if bnd is totally inside this node's plane
	//		recur bnd on inside child/leaf
	//  else if bnd is totaly outside this node's plane
	//		recur bnd on outside child
	//	else
	//		recur bnd's children on this node
	// 
	// There's two ways to output the visibility info
	//		1) Set a visible/invisible bit for each of the bnd leaves
	//		2) output a list of visible bnds.
	// The second is preferable, since leaves determined invisible by interior
	// node tests never get traversed. But if the rendering pipeline has needs
	// to traverse the drawable data in some other order (for depth or material
	// sorting for example), then the list of visible bnds needs to be translated
	// into the first option anyway.
	//
	// Notes on the vague algorithm:
	//	When recurring on the inside child, hitting a leaf checks against the source
	//		occluder poly, with the usual inside=hidden, outside=visible, split recurs
	//		the bnd's children on this leaf.
	//	Hitting a nil outside child == visible
	//	It's a double recursion, recurring first on the bnd hierarchy, and second on the occluder tree.
	//	Recursion stops when:
	//		1) A bnd is totally in or totally out of a leaf of the occluder tree
	//		2) A bnd is a leaf of the bnd hierarchy.
	// 
	void TestHeirarchy(plBoundsHierarchy* bnd);

	virtual void Read(hsStream* s, hsResMgr* mgr);
	virtual void Write(hsStream* s, hsResMgr* mgr);

	// Export only
	void				AddPoly(plOccPoly* poly);
	void				BuildBSP();
};

#endif // plOccTree_inc
