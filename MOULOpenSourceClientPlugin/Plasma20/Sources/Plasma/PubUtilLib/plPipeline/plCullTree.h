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

#ifndef plCullTree_inc
#define plCullTree_inc

#include "hsBounds.h"
#include "hsGeometry3.h"
#include "hsBitVector.h"
#include "plCuller.h"
#include "../plScene/plCullPoly.h"

#ifdef HS_DEBUGGING
#define DEBUG_POINTERS
#endif // HS_DEBUGGING

class plCullTree;
class plCullNode;

// for vis
struct hsPoint3;
struct hsVector3;
struct hsColorRGBA;

class plCullTree : public plCuller
{
protected:

	// Visualization stuff, to be nuked from production version.
	mutable hsBool							fCapturePolys;
	mutable hsTArray<hsPoint3>				fVisVerts;
	mutable hsTArray<hsVector3>				fVisNorms;
	mutable hsTArray<hsColorRGBA>			fVisColors;
	mutable hsTArray<UInt16>				fVisTris;
	mutable hsScalar						fVisYon;

	mutable hsTArray<plCullPoly>		fScratchPolys;
	mutable hsLargeArray<Int16>		fScratchClear;
	mutable hsLargeArray<Int16>		fScratchSplit;
	mutable hsLargeArray<Int16>		fScratchCulled;
	mutable hsBitVector				fScratchBitVec;
	mutable hsBitVector				fScratchTotVec;

	void		IVisPolyShape(const plCullPoly& poly, hsBool dark) const;
	void		IVisPolyEdge(const hsPoint3& p0, const hsPoint3& p1, hsBool dark) const;
	void		IVisPoly(const plCullPoly& poly, hsBool dark) const;


	hsPoint3						fViewPos;

	Int16							fRoot;
	mutable hsTArray<plCullNode>	fNodeList; // Scratch list we make the tree from.
	plCullNode*						IGetRoot() const { return IGetNode(fRoot); }
	plCullNode*						IGetNode(Int16 i) const { return i >= 0 ? &fNodeList[i] : nil; }

	void				ITestNode(const plSpaceTree* space, Int16 who, hsTArray<Int16>& outList) const; // Appends to outlist
	void				ITestList(const plSpaceTree* space, const hsTArray<Int16>& inList, hsTArray<Int16>& outList) const;

	Int16				IAddPolyRecur(const plCullPoly& poly, Int16 iNode);
	Int16				IMakeHoleSubTree(const plCullPoly& poly) const;
	Int16				IMakePolySubTree(const plCullPoly& poly) const;
	Int16				IMakePolyNode(const plCullPoly& poly, int i0, int i1) const;

	// Some scratch areas for the nodes use when building the tree etc.
	hsTArray<plCullPoly>&			ScratchPolys() const { return fScratchPolys; }
	hsLargeArray<Int16>&			ScratchClear() const { return fScratchClear; }
	hsLargeArray<Int16>&			ScratchSplit() const { return fScratchSplit; }
	hsLargeArray<Int16>&			ScratchCulled() const { return fScratchCulled; }
	hsBitVector&					ScratchBitVec() const { return fScratchBitVec; }
	hsBitVector&					ScratchTotVec() const { return fScratchTotVec; }

	void							ISetupScratch(UInt16 nNodes);

	friend class plCullNode;

public:
	plCullTree();
	~plCullTree();

	void					Reset(); // Called before starting to add polys for this frame.
	void					InitFrustum(const hsMatrix44& world2NDC);
	void					SetViewPos(const hsPoint3& pos);
	void					AddPoly(const plCullPoly& poly);

	UInt32					GetNumNodes() const { return fNodeList.GetCount(); }

	virtual void			Harvest(const plSpaceTree* space, hsTArray<Int16>& outList) const;
	virtual hsBool			BoundsVisible(const hsBounds3Ext& bnd) const;
	virtual hsBool			SphereVisible(const hsPoint3& center, hsScalar rad) const;

	// Visualization stuff. Only to be called by the pipeline (or some other vis manager).
	void					SetVisualizationYon(hsScalar y) const { fVisYon = y; }
	void					BeginCapturePolys() const { fCapturePolys = true; }
	void					EndCapturePolys() const { fCapturePolys = false; }
	hsTArray<hsPoint3>&		GetCaptureVerts() const { return fVisVerts; }
	hsTArray<hsVector3>&	GetCaptureNorms() const { return fVisNorms; }
	hsTArray<hsColorRGBA>&	GetCaptureColors() const { return fVisColors; }
	hsTArray<UInt16>&		GetCaptureTris() const { return fVisTris; }
	void					ReleaseCapture() const;
};

class plCullNode
{
public:
enum plCullStatus
{
	kClear,
	kCulled,
	kSplit,
	kPureSplit
};
protected:
	hsVector3			fNorm;
	hsScalar			fDist;

	hsBool				fIsFace;

	Int16				fInnerChild;
	Int16				fOuterChild;

	const plCullTree*			fTree;

	plCullNode*					IGetNode(Int16 i) const;

#ifdef DEBUG_POINTERS
	mutable plCullNode*			fInnerPtr;
	mutable plCullNode*			fOuterPtr;

	void						ISetPointersRecur() const;
#else // DEBUG_POINTERS
	void						ISetPointersRecur() const {}
#endif // DEBUG_POINTERS

	// Bounds only version
	plCullNode::plCullStatus	ITestBoundsRecur(const hsBounds3Ext& bnd) const;
	plCullNode::plCullStatus	ITestSphereRecur(const hsPoint3& center, hsScalar rad) const;

	// Using the nodes
	plCullNode::plCullStatus	ITestNode(const plSpaceTree* space, Int16 who, hsLargeArray<Int16>& clear, hsLargeArray<Int16>& split, hsLargeArray<Int16>& culled) const;
	void						ITestNode(const plSpaceTree* space, Int16 who, hsBitVector& totList, hsBitVector& outList) const;
	void						IHarvest(const plSpaceTree* space, hsTArray<Int16>& outList) const;

	// Constructing the tree
	hsScalar					IInterpVert(const hsPoint3& p0, const hsPoint3& p1, hsPoint3& out) const;
	plCullNode::plCullStatus	ISplitPoly(const plCullPoly& poly, plCullPoly*& innerPoly, plCullPoly*& outerPoly) const;
	void						IMarkClipped(const plCullPoly& poly, const hsBitVector& onVerts) const;
	void						ITakeHalfPoly(const plCullPoly& scrPoly, 
								   const hsTArray<int>& vtxIdx, 
								   const hsBitVector& onVerts, 
								   plCullPoly& outPoly) const;
	void						IBreakPoly(const plCullPoly& poly, const hsTArray<hsScalar>& depths,
									hsBitVector& inVerts,
									hsBitVector& outVerts,
									hsBitVector& onVerts,
									plCullPoly& srcPoly) const;

	hsTArray<plCullPoly>&			ScratchPolys() const { return fTree->ScratchPolys(); }
	hsLargeArray<Int16>&			ScratchClear() const { return fTree->ScratchClear(); }
	hsLargeArray<Int16>&			ScratchSplit() const { return fTree->ScratchSplit(); }
	hsLargeArray<Int16>&			ScratchCulled() const { return fTree->ScratchCulled(); }
	hsBitVector&					ScratchBitVec() const { return fTree->ScratchBitVec(); }
	hsBitVector&					ScratchTotVec() const { return fTree->ScratchTotVec(); }

	friend class plCullTree;
public:

	void	Init(const plCullTree* t, const hsVector3& n, hsScalar d) { fIsFace = false; fTree = t; fInnerChild = fOuterChild = -1; SetPlane(n, d); }
	void	Init(const plCullTree* t, const plCullPoly& poly) { Init(t, poly.fNorm, poly.fDist); }

	void	SetPlane(const hsVector3& n, hsScalar d) { fNorm = n; fDist = d; }
	
	const hsVector3& GetNormal() const { return fNorm; }
	const hsScalar GetDist() const { return fDist; }

	plCullStatus	TestBounds(const hsBounds3Ext& bnd) const;
	plCullStatus	TestSphere(const hsPoint3& center, hsScalar rad) const;
};

inline plCullNode* plCullNode::IGetNode(Int16 i) const
{ 
	return fTree->IGetNode(i); 
}

#endif // plCullTree_inc
