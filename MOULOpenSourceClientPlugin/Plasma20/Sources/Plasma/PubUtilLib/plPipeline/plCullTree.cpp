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

#include "hsTypes.h"
#include "plCullTree.h"
#include "../plDrawable/plSpaceTree.h"
#include "hsFastMath.h"
#include "hsColorRGBA.h"
#include "plProfile.h"

#include "plTweak.h"

#define MF_DEBUG_NORM
#ifdef MF_DEBUG_NORM

#define IDEBUG_NORMALIZE( a, b ) { hsScalar len = hsFastMath::InvSqrtAppr((a).MagnitudeSquared()); a *= len; b *= len; }

#else // MF_DEBUG_NORM
#define IDEBUG_NORMALIZE( a, b )
#endif // MF_DEBUG_NORM

//#define CULL_SMALL_TOLERANCE
#ifdef CULL_SMALL_TOLERANCE
//static const hsScalar kTolerance = 1.e-5f;
static const hsScalar kTolerance = 1.e-3f;
#else //CULL_SMALL_TOLERANCE
static const hsScalar kTolerance = 1.e-1f;
#endif // CULL_SMALL_TOLERANCE

plProfile_CreateCounter("Harvest Nodes", "Draw", HarvestNodes);

//////////////////////////////////////////////////////////////////////
// Harvest culling section.
// These are the functions used on a built tree
//////////////////////////////////////////////////////////////////////
plCullNode::plCullStatus plCullNode::ITestBoundsRecur(const hsBounds3Ext& bnd) const
{
	plCullNode::plCullStatus retVal = TestBounds(bnd);

	// No Children, what we say goes.
	if( (fOuterChild < 0) && (fInnerChild < 0) )
		return retVal;

	// No innerchild. If we cull, it's culled, else we
	// hope our outerchild culls it.
	if( fInnerChild < 0 )
	{
		if( retVal == kCulled )
			return kCulled;

		return IGetNode(fOuterChild)->ITestBoundsRecur(bnd);
	}

	// No outerchild. If we say it's clear, it's clear (or split), but if
	// it's culled, we have to pass it to innerchild, who may pronounce it clear
	if( fOuterChild < 0 )
	{
		if( retVal == kClear )
			return kClear;
		if( retVal == kSplit )
			return kSplit;
		return IGetNode(fInnerChild)->ITestBoundsRecur(bnd);
	}

	// We've got both children to feed.
	// We pass the clear ones to the inner child, culled to outer, 
	// and split to both. Remember, a both children have to agree to cull a split.
	if( retVal == kClear )
		return IGetNode(fOuterChild)->ITestBoundsRecur(bnd);

	if( retVal == kCulled )
		return IGetNode(fInnerChild)->ITestBoundsRecur(bnd);

	// Here's the split, to be culled, both children have to
	// say its culled.
	if( kCulled != IGetNode(fOuterChild)->ITestBoundsRecur(bnd) )
		return kSplit;

	if( kCulled != IGetNode(fInnerChild)->ITestBoundsRecur(bnd) )
		return kSplit;

	return kCulled;
}

plCullNode::plCullStatus plCullNode::TestBounds(const hsBounds3Ext& bnd) const
{
	// Not sure if doing a sphere test will pay off or not. Some circumstantial evidence
	// from TrueTime suggests it could very well, but I really need to do some side by
	// side timings to be sure. Still looking for some reasonably constructed real data sets. mf
#define MF_TEST_SPHERE_FIRST
#ifdef MF_TEST_SPHERE_FIRST
	hsScalar dist = fNorm.InnerProduct(bnd.GetCenter()) + fDist;
	hsScalar rad = bnd.GetRadius();
	if( dist < -rad )
		return kCulled;
	if( dist > rad )
		return kClear;
#endif // MF_TEST_SPHERE_FIRST

	hsPoint2 depth;
	bnd.TestPlane(fNorm, depth);

	const hsScalar kSafetyDist = -0.1f;
	if( depth.fY + fDist < kSafetyDist )
		return kCulled;

	if( depth.fX + fDist >= 0 )
		return kClear;

	return kSplit;
}

plCullNode::plCullStatus plCullNode::ITestSphereRecur(const hsPoint3& center, hsScalar rad) const
{
	plCullNode::plCullStatus retVal = TestSphere(center, rad);

	// No Children, what we say goes.
	if( (fOuterChild < 0) && (fInnerChild < 0) )
		return retVal;

	// No innerchild. If we cull, it's culled, else we
	// hope our outerchild culls it.
	if( fInnerChild < 0 )
	{
		if( retVal == kCulled )
			return kCulled;

		return IGetNode(fOuterChild)->ITestSphereRecur(center, rad);
	}

	// No outerchild. If we say it's clear, it's clear (or split), but if
	// it's culled, we have to pass it to innerchild, who may pronounce it clear
	if( fOuterChild < 0 )
	{
		if( retVal == kClear )
			return kClear;
		if( retVal == kSplit )
			return kSplit;
		return IGetNode(fInnerChild)->ITestSphereRecur(center, rad);
	}

	// We've got both children to feed.
	// We pass the clear ones to the inner child, culled to outer, 
	// and split to both. Remember, a both children have to agree to cull a split.
	if( retVal == kClear )
		return IGetNode(fOuterChild)->ITestSphereRecur(center, rad);

	if( retVal == kCulled )
		return IGetNode(fInnerChild)->ITestSphereRecur(center, rad);

	// Here's the split, to be culled, both children have to
	// say its culled.
	if( kCulled != IGetNode(fOuterChild)->ITestSphereRecur(center, rad) )
		return kSplit;

	if( kCulled != IGetNode(fInnerChild)->ITestSphereRecur(center, rad) )
		return kSplit;

	return kCulled;
}

plCullNode::plCullStatus plCullNode::TestSphere(const hsPoint3& center, hsScalar rad) const
{
	hsScalar dist = fNorm.InnerProduct(center) + fDist;
	if( dist < -rad )
		return kCulled;
	if( dist > rad )
		return kClear;

	return kSplit;
}

// For this Cull Node, recur down the space hierarchy pruning out who to test for the next Cull Node.
plCullNode::plCullStatus plCullNode::ITestNode(const plSpaceTree* space, Int16 who, hsLargeArray<Int16>& clear, hsLargeArray<Int16>& split, hsLargeArray<Int16>& culled) const
{
	if( space->IsDisabled(who) || (space->GetNode(who).fWorldBounds.GetType() != kBoundsNormal) )
	{
		culled.Append(who);
		return kCulled;
	}

	plCullStatus retVal = kClear;
	plCullStatus stat = TestBounds(space->GetNode(who).fWorldBounds);

	switch( stat )
	{
	case kClear:
		clear.Append(who);
		retVal = kClear;
		break;
	case kCulled:
		culled.Append(who);
		retVal = kCulled;
		break;
	case kSplit:
		if( space->GetNode(who).fFlags & plSpaceTreeNode::kIsLeaf )
		{
//			split.Append(who);
			retVal = kPureSplit;
		}
		else
		{
			plCullStatus child0 = ITestNode(space, space->GetNode(who).GetChild(0), clear, split, culled);
			plCullStatus child1 = ITestNode(space, space->GetNode(who).GetChild(1), clear, split, culled);

			if( child0 != child1 )
			{
				if( child0 == kPureSplit )
					split.Append(space->GetNode(who).GetChild(0));
				else if( child1 == kPureSplit )
					split.Append(space->GetNode(who).GetChild(1));
				retVal = kSplit;
			}
			else if( child0 == kPureSplit )
			{
				retVal = kPureSplit;
			}
		}
	}
	return retVal;
}

// Cycle through the Cull Nodes, paring down the list of who to test (through ITestNode above).
// We reclaim the scratch indices in clear and split when we're done (SetCount(0)), but we can't
// reclaim the culled, because our caller may be looking at who all we culled. See below in split.
// If a node is disabled, we can just ignore we ever got called.
void plCullNode::ITestNode(const plSpaceTree* space, Int16 who, hsBitVector& totList, hsBitVector& outList) const
{
	if( space->IsDisabled(who) )
		return;

	UInt32 myClearStart = ScratchClear().GetCount();
	UInt32 mySplitStart = ScratchSplit().GetCount();
	UInt32 myCullStart = ScratchCulled().GetCount();

	if( kPureSplit == ITestNode(space, who, ScratchClear(), ScratchSplit(), ScratchCulled()) )
		ScratchSplit().Append(who);

	UInt32 myClearEnd = ScratchClear().GetCount();
	UInt32 mySplitEnd = ScratchSplit().GetCount();
	UInt32 myCullEnd = ScratchCulled().GetCount();

	int i;
	// If there's no OuterChild, everything in clear and split is visible. Everything in culled
	// goes to innerchild (if any).
	if( fOuterChild < 0 )
	{
		plProfile_IncCount(HarvestNodes, myClearEnd - myClearStart + mySplitEnd - mySplitStart);
		// Replace these with a memcopy or something!!!!
		for( i = myClearStart; i < myClearEnd; i++ )
		{
			space->HarvestLeaves(ScratchClear()[i], totList, outList);
		}
		for( i = mySplitStart; i < mySplitEnd; i++ )
		{
			space->HarvestLeaves(ScratchSplit()[i], totList, outList);
		}

		if( fInnerChild >= 0 )
		{
			for( i = myCullStart; i < myCullEnd; i++ )
			{
				IGetNode(fInnerChild)->ITestNode(space, ScratchCulled()[i], totList, outList);
			}
		}
		ScratchClear().SetCount(myClearStart);
		ScratchSplit().SetCount(mySplitStart);
		ScratchCulled().SetCount(myCullStart);

		return;
	}

	// There is an OuterChild, so whether there's an InnerChild or not,
	// everything in ClearList is visible soley on the discretion of OuterChild.
	for( i = myClearStart; i < myClearEnd; i++ )
	{
		IGetNode(fOuterChild)->ITestNode(space, ScratchClear()[i], totList, outList);
	}

	// If there's no InnerChild, then the SplitList is also visible soley
	// on the discretion of OuterChild.
	if( fInnerChild < 0 )
	{
		for( i = mySplitStart; i < mySplitEnd; i++ )
		{
			IGetNode(fOuterChild)->ITestNode(space, ScratchSplit()[i], totList, outList);
		}

		ScratchClear().SetCount(myClearStart);
		ScratchSplit().SetCount(mySplitStart);
		ScratchCulled().SetCount(myCullStart);

		return;
	}

	// There is an inner child. Everything in culled list is visible
	// soley on its discretion.
	for( i = myCullStart; i < myCullEnd; i++ )
	{
		IGetNode(fInnerChild)->ITestNode(space, ScratchCulled()[i], totList, outList);
	}

	// Okay, here's the rub.
	// Everyone in the split list needs to be tested against InnerChild and OuterChild.
	// If either child says it's okay (puts it in OutList), then it's okay.
	// The problem is that if both children say it's okay, it will wind up in outList twice.
	// This is complicated by the fact that outList is still subTrees at this point,
	// so InnerChild adding a subTree and OuterChild adding a child of that subTree isn't
	// even appending the same value to the list.
	// Sooooo.
	// What we do is keep track of every node (interior and leaf) that gets harvested.
	// When we go to harvest a subtree, we check in totList for its bit being set. Bits
	// set in totList are ENTIRE SUBTREE IS HARVESTED. SpaceTree understands this too in
	// its HarvestLeaves. Seems obvious now, but I didn't hear you suggest it.

	for( i = mySplitStart; i < mySplitEnd; i++ )
	{
		IGetNode(fOuterChild)->ITestNode(space, ScratchSplit()[i], totList, outList);
	}

	for( i = mySplitStart; i < mySplitEnd; i++ )
	{
		if( !totList.IsBitSet(ScratchSplit()[i]) )
			IGetNode(fInnerChild)->ITestNode(space, ScratchSplit()[i], totList, outList);
	}

	ScratchClear().SetCount(myClearStart);
	ScratchSplit().SetCount(mySplitStart);
	ScratchCulled().SetCount(myCullStart);
}

void plCullNode::IHarvest(const plSpaceTree* space, hsTArray<Int16>& outList) const
{
	ITestNode(space, space->GetRoot(), ScratchTotVec(), ScratchBitVec());
	space->BitVectorToList(outList, ScratchBitVec());
	ScratchBitVec().Clear();
	ScratchTotVec().Clear();

	ScratchClear().SetCount(0);
	ScratchSplit().SetCount(0);
	ScratchCulled().SetCount(0);
}

//////////////////////////////////////////////////////////////////////
// This section builds the tree from the input cullpoly's
//////////////////////////////////////////////////////////////////////

void plCullNode::IBreakPoly(const plCullPoly& poly, const hsTArray<hsScalar>& depths,
							hsBitVector& inVerts,
							hsBitVector& outVerts,
							hsBitVector& onVerts,
							plCullPoly& outPoly) const
{
	inVerts.Clear();
	outVerts.Clear();
	onVerts.Clear();

	outPoly.Init(poly);

	if( depths[0] < -kTolerance )
		inVerts.SetBit(0);
	else if( depths[0] > kTolerance )
		outVerts.SetBit(0);
	else
		onVerts.SetBit(0);
	
	if( poly.fClipped.IsBitSet(0) )
		outPoly.fClipped.SetBit(0);
	outPoly.fVerts.Append(poly.fVerts[0]);

	int i;
	for( i = 1; i < poly.fVerts.GetCount(); i++ )
	{
		if( depths[i] < -kTolerance )
		{
			if( outVerts.IsBitSet(outPoly.fVerts.GetCount()-1) )
			{
				hsPoint3 interp;
				hsScalar t = IInterpVert(poly.fVerts[i-1], poly.fVerts[i], interp);
				// add interp
				onVerts.SetBit(outPoly.fVerts.GetCount());
				if( poly.fClipped.IsBitSet(i-1) )
					outPoly.fClipped.SetBit(outPoly.fVerts.GetCount());
				outPoly.fVerts.Append(interp);
			}
			inVerts.SetBit(outPoly.fVerts.GetCount());
		}
		else if( depths[i] > kTolerance )
		{
			if( inVerts.IsBitSet(outPoly.fVerts.GetCount()-1) )
			{
				hsPoint3 interp;
				hsScalar t = IInterpVert(poly.fVerts[i-1], poly.fVerts[i], interp);
				// add interp
				onVerts.SetBit(outPoly.fVerts.GetCount());
				if( poly.fClipped.IsBitSet(i-1) )
					outPoly.fClipped.SetBit(outPoly.fVerts.GetCount());
				outPoly.fVerts.Append(interp);
			}
			outVerts.SetBit(outPoly.fVerts.GetCount());
		}
		else
		{
			onVerts.SetBit(outPoly.fVerts.GetCount());
		}

		if( poly.fClipped.IsBitSet(i) )
			outPoly.fClipped.SetBit(outPoly.fVerts.GetCount());
		outPoly.fVerts.Append(poly.fVerts[i]);
	}
	if( (inVerts.IsBitSet(outPoly.fVerts.GetCount()-1) && outVerts.IsBitSet(0))
		||(outVerts.IsBitSet(outPoly.fVerts.GetCount()-1) && inVerts.IsBitSet(0)) )
	{
		hsPoint3 interp;
		hsScalar t = IInterpVert(poly.fVerts[poly.fVerts.GetCount()-1], poly.fVerts[0], interp);
		onVerts.SetBit(outPoly.fVerts.GetCount());
		if( poly.fClipped.IsBitSet(poly.fVerts.GetCount()-1) )
			outPoly.fClipped.SetBit(outPoly.fVerts.GetCount());
		outPoly.fVerts.Append(interp);
	}
}

void plCullNode::ITakeHalfPoly(const plCullPoly& srcPoly, 
							   const hsTArray<int>& vtxIdx, 
							   const hsBitVector& onVerts, 
							   plCullPoly& outPoly) const
{
	if( vtxIdx.GetCount() > 2 )
	{
		int i;
		for( i = 0; i < vtxIdx.GetCount(); i++ )
		{
			int next = i < vtxIdx.GetCount()-1 ? i+1 : 0;
			int last = i ? i-1 : vtxIdx.GetCount()-1;

			// If these 3 verts are all on the plane, we may have created a collinear vertex (the middle one)
			// which we now want to skip.
			if( onVerts.IsBitSet(vtxIdx[i]) && onVerts.IsBitSet(vtxIdx[last]) && onVerts.IsBitSet(vtxIdx[next]) )
			{
#if 0 // FISH
				hsScalar dot = hsVector3(&srcPoly.fVerts[vtxIdx[last]], &srcPoly.fVerts[vtxIdx[i]]).InnerProduct(hsVector3(&srcPoly.fVerts[vtxIdx[next]], &srcPoly.fVerts[vtxIdx[i]]));
				if( dot <= 0 )
#endif // FISH
					continue;
			}
			if( srcPoly.fClipped.IsBitSet(vtxIdx[i])
				||(onVerts.IsBitSet(vtxIdx[i]) && onVerts.IsBitSet(vtxIdx[next])) )
					outPoly.fClipped.SetBit(outPoly.fVerts.GetCount());
			outPoly.fVerts.Append(srcPoly.fVerts[vtxIdx[i]]);
		}
	}
	else
	{
		// Just need a break point
		hsStatusMessage("Under 2"); // FISH
	}
}

void plCullNode::IMarkClipped(const plCullPoly& poly, const hsBitVector& onVerts) const
{
	int i;
	for( i = 1; i < poly.fVerts.GetCount(); i++ )
	{
		int last = i-1;
		if( onVerts[i] && onVerts[last] )
			poly.fClipped.SetBit(last);
	}
	if( onVerts[i] && onVerts[0] )
		poly.fClipped.SetBit(0);
}

plCullNode::plCullStatus plCullNode::ISplitPoly(const plCullPoly& poly, 
												plCullPoly*& innerPoly, 
												plCullPoly*& outerPoly) const
{
	static hsTArray<hsScalar> depths;
	depths.SetCount(poly.fVerts.GetCount());

	static hsBitVector onVerts;
	onVerts.Clear();

	hsBool someInner = false;
	hsBool someOuter = false;
	hsBool someOn = false;
	int i;
	for( i = 0; i < poly.fVerts.GetCount(); i++ )
	{
		depths[i] = fNorm.InnerProduct(poly.fVerts[i]) + fDist;
		if( depths[i] < -kTolerance )
			someInner = true;
		else if( depths[i] > kTolerance )
			someOuter = true;
		else 
		{
			someOn = true;
			onVerts.SetBit(i);
		}
	}
	if( !(someInner || someOuter) )
	{
		(innerPoly = ScratchPolys().Push())->Init(poly);
		(outerPoly = ScratchPolys().Push())->Init(poly);
		return kSplit;
	}
	else if( !someInner )
	{
		IMarkClipped(poly, onVerts);
		return kClear;
	}
	else if( !someOuter )
	{
		IMarkClipped(poly, onVerts);
		return kCulled;
	}


	// Okay, it's split, now break it into the two polys
	(innerPoly = ScratchPolys().Push())->Init(poly);
	(outerPoly = ScratchPolys().Push())->Init(poly);

	static plCullPoly scrPoly;

	static hsBitVector inVerts;
	static hsBitVector outVerts;

	IBreakPoly(poly, depths,
		inVerts,
		outVerts,
		onVerts,
		scrPoly);

	static hsTArray<int> inPolyIdx;
	inPolyIdx.SetCount(0);
	static hsTArray<int> outPolyIdx;
	outPolyIdx.SetCount(0);

	for( i = 0; i < scrPoly.fVerts.GetCount(); i++ )
	{
		if( inVerts.IsBitSet(i) )
		{
			inPolyIdx.Append(i);
		}
		else if( outVerts.IsBitSet(i) )
		{
			outPolyIdx.Append(i);
		}
		else
		{
			inPolyIdx.Append(i);
			outPolyIdx.Append(i);
		}
	}

	ITakeHalfPoly(scrPoly, inPolyIdx, onVerts, *innerPoly);

	ITakeHalfPoly(scrPoly, outPolyIdx, onVerts, *outerPoly);

	return kSplit;
}

hsScalar plCullNode::IInterpVert(const hsPoint3& p0, const hsPoint3& p1, hsPoint3& out) const
{
	hsVector3 oneToOh;
	oneToOh.Set(&p0, &p1);

	hsScalar t = -(fNorm.InnerProduct(p1) + fDist) / fNorm.InnerProduct(oneToOh);
	if( t >= 1.f )
	{
		out = p0;
		return 1.f;
	}
	if( t <= 0 )
	{
		out = p1;
		return 0;
	}

	out = p1;

	out += oneToOh * t;

	return t;
}

// We use indices so our tree can actually be an array, which may be
// resized at any time, which would invalidate any pointers we have.
// But debugging a large tree is hard enough when stepping through pointers,
// it's pretty much impossible with indices. So when debugging we can 
// setup these pointers for stepping through the tree. We just need to
// reset them every time we add a poly, because that's when the tree
// may have been resized invalidating the old pointers.
#ifdef DEBUG_POINTERS
void plCullNode::ISetPointersRecur() const
{
	if( fInnerPtr = IGetNode(fInnerChild) )
		fInnerPtr->ISetPointersRecur();
	if( fOuterPtr = IGetNode(fOuterChild) )
		fOuterPtr->ISetPointersRecur();
}
#endif // DEBUG_POINTERS

//////////////////////////////////////////////////////////////////////
// Now the tree proper
//////////////////////////////////////////////////////////////////////
// Build the tree
plCullTree::plCullTree()
:	fRoot(-1),
	fCapturePolys(false)
{
}

plCullTree::~plCullTree()
{
}

void plCullTree::AddPoly(const plCullPoly& poly)
{
	const plCullPoly* usePoly = &poly;

	hsVector3 cenToEye(&fViewPos, &poly.fCenter);
	hsFastMath::NormalizeAppr(cenToEye);
	hsScalar camDist = cenToEye.InnerProduct(poly.fNorm);
	plConst(hsScalar) kTol(0.1f);
	hsBool backFace = camDist < -kTol;
	if( !backFace && (camDist < kTol) )
		return;

	plCullPoly scratchPoly;
	if( poly.IsHole() )
	{
		if( !backFace )
			return;
	}
	else
	if( backFace )
	{
		plConst(hsBool) kAllowTwoSided(true);
		if( !kAllowTwoSided || !poly.IsTwoSided() )
			return;

		scratchPoly.Flip(poly);
		usePoly = &scratchPoly;
	}

	if( !SphereVisible(usePoly->GetCenter(), usePoly->GetRadius()) )
		return;

	usePoly->fClipped.Clear();

	usePoly->Validate();

	// Make sure we have enough scratch polys. Each node
	// can potentially split this poly, so...
	ISetupScratch(fNodeList.GetCount());

#if 1
	if( IGetRoot() && IGetNode(IGetRoot()->fOuterChild) )
	{
		IAddPolyRecur(*usePoly, IGetRoot()->fOuterChild);
	}
	else
#endif
	{
		fRoot = IAddPolyRecur(*usePoly, fRoot);
	}

#ifdef DEBUG_POINTERS
	if( IGetRoot() )
		IGetRoot()->ISetPointersRecur();
#endif // DEBUG_POINTERS
}

Int16 plCullTree::IAddPolyRecur(const plCullPoly& poly, Int16 iNode)
{
	if( poly.fVerts.GetCount() < 3 )
		return iNode;

	if( iNode < 0 )
		return IMakePolySubTree(poly);

	hsBool addInner = (IGetNode(iNode)->fInnerChild >= 0)
						|| ((iNode > 5) && poly.IsHole());
	hsBool addOuter = !poly.IsHole() || (IGetNode(iNode)->fOuterChild >= 0);

	plCullPoly* innerPoly = nil;
	plCullPoly* outerPoly = nil;

	plCullNode::plCullStatus test = IGetNode(iNode)->ISplitPoly(poly, innerPoly, outerPoly);

	switch( test )
	{
	case plCullNode::kClear:
		if( addOuter )
		{
			int child = IAddPolyRecur(poly, IGetNode(iNode)->fOuterChild);
			IGetNode(iNode)->fOuterChild = child;
		}
		break;
	case plCullNode::kCulled:
		if( addInner )
		{
			int child = IAddPolyRecur(poly, IGetNode(iNode)->fInnerChild);
			IGetNode(iNode)->fInnerChild = child;
		}
		break;
	case plCullNode::kSplit:
		hsAssert(innerPoly && outerPoly, "Poly should have been split into inner and outer in SplitPoly");
		if( addOuter )
		{
			int child = IAddPolyRecur(*outerPoly, IGetNode(iNode)->fOuterChild);
			IGetNode(iNode)->fOuterChild = child;
		}
		if( addInner )
		{
			int child = IAddPolyRecur(*innerPoly, IGetNode(iNode)->fInnerChild);
			IGetNode(iNode)->fInnerChild = child;
		}
		break;
	}
	return iNode;
}

Int16 plCullTree::IMakePolyNode(const plCullPoly& poly, int i0, int i1) const
{
	Int16 retINode = fNodeList.GetCount();
	plCullNode* nextNode = fNodeList.Push();
	hsVector3 a;
	hsVector3 b;
	a.Set(&poly.fVerts[i0], &fViewPos);
	b.Set(&poly.fVerts[i1], &fViewPos);
	hsVector3 n = a % b;
	hsScalar d = -n.InnerProduct(fViewPos);

	IDEBUG_NORMALIZE(n, d);

	nextNode->Init(this, n, d);

	return retINode;
}

Int16 plCullTree::IMakeHoleSubTree(const plCullPoly& poly) const
{
	if( fCapturePolys )
		IVisPoly(poly, true);

	int firstNode = fNodeList.GetCount();

	Int16 iNode = -1;

	int i;
	for( i = 0; i < poly.fVerts.GetCount()-1; i++ )
	{
		if( !poly.fClipped.IsBitSet(i) )
		{
			Int16 child = IMakePolyNode(poly, i, i+1);
			if( iNode >= 0 )
				IGetNode(iNode)->fOuterChild = child;
			iNode = child;
		}
	}
	if( !poly.fClipped.IsBitSet(i) )
	{
		Int16 child = IMakePolyNode(poly, i, 0);
		if( iNode >= 0 )
			IGetNode(iNode)->fOuterChild = child;
		iNode = child;
	}

	plCullNode* child = fNodeList.Push();
	child->Init(this, poly.fNorm, poly.fDist);
	if( iNode >= 0 )
		IGetNode(iNode)->fOuterChild = fNodeList.GetCount()-1;

	return firstNode;
}

Int16 plCullTree::IMakePolySubTree(const plCullPoly& poly) const
{
	poly.Validate();

	if( poly.IsHole() )
		return IMakeHoleSubTree(poly);

	if( fCapturePolys )
		IVisPoly(poly, false);

	int firstNode = fNodeList.GetCount();

	Int16 iNode = -1;

	int i;
	for( i = 0; i < poly.fVerts.GetCount()-1; i++ )
	{
		if( !poly.fClipped.IsBitSet(i) )
		{
			Int16 child = IMakePolyNode(poly, i, i+1);
			if( iNode >= 0 )
				IGetNode(iNode)->fInnerChild = child;
			iNode = child;
		}
	}
	if( !poly.fClipped.IsBitSet(i) )
	{
		Int16 child = IMakePolyNode(poly, i, 0);
		if( iNode >= 0 )
			IGetNode(iNode)->fInnerChild = child;
		iNode = child;
	}

	plCullNode* child = fNodeList.Push();
	child->Init(this, poly.fNorm, poly.fDist);
	child->fIsFace = true;
	if( iNode >= 0 )
		IGetNode(iNode)->fInnerChild = fNodeList.GetCount()-1;

	return firstNode;
}

///////////////////////////////////////////////////////////////////
// Begin visualization section of the program
///////////////////////////////////////////////////////////////////
void plCullTree::IVisPolyShape(const plCullPoly& poly, hsBool dark) const
{
	int i;

	int vertStart = fVisVerts.GetCount();
	
	hsColorRGBA color;
	if( dark )
		color.Set(0.2f, 0.2f, 0.2f, 1.f);
	else
		color.Set(1.f, 1.f, 1.f, 1.f);

	hsVector3 norm = dark ? -poly.fNorm : poly.fNorm;

	for( i = 0; i < poly.fVerts.GetCount(); i++ )
	{
		fVisVerts.Append(poly.fVerts[i]);
		fVisNorms.Append(poly.fNorm);
		fVisColors.Append(color);
	}
	if( !dark )
	{
		for( i = 2; i < poly.fVerts.GetCount(); i++ )
		{
			fVisTris.Append(vertStart);
			fVisTris.Append(vertStart + i-1);
			fVisTris.Append(vertStart + i);
		}
	}
	else
	{
		for( i = 2; i < poly.fVerts.GetCount(); i++ )
		{
			fVisTris.Append(vertStart);
			fVisTris.Append(vertStart + i);
			fVisTris.Append(vertStart + i-1);
		}
	}
}

void plCullTree::IVisPolyEdge(const hsPoint3& p0, const hsPoint3& p1, hsBool dark) const
{
	hsColorRGBA color;
	if( dark )
		color.Set(0.2f, 0.2f, 0.2f, 1.f);
	else
		color.Set(1.f, 1.f, 1.f, 1.f);

	int vertStart = fVisVerts.GetCount();

	hsVector3 dir0(&p0, &fViewPos);
	hsFastMath::NormalizeAppr(dir0);
	dir0 *= fVisYon;
	hsVector3 dir1(&p1, &fViewPos);
	hsFastMath::NormalizeAppr(dir1);
	dir1 *= fVisYon;

	hsPoint3 p3 = fViewPos;
	p3 += dir0;
	hsPoint3 p2 = fViewPos;
	p2 += dir1;

	hsVector3 norm = hsVector3(&p0, &fViewPos) % hsVector3(&p1, &fViewPos);
	hsFastMath::NormalizeAppr(norm);

	fVisVerts.Append(p0);
	fVisNorms.Append(norm);
	fVisColors.Append(color);
	fVisVerts.Append(p1);
	fVisNorms.Append(norm);
	fVisColors.Append(color);
	fVisVerts.Append(p2);
	fVisNorms.Append(norm);
	fVisColors.Append(color);
	fVisVerts.Append(p3);
	fVisNorms.Append(norm);
	fVisColors.Append(color);

	fVisTris.Append(vertStart + 0);
	fVisTris.Append(vertStart + 2);
	fVisTris.Append(vertStart + 1);

	fVisTris.Append(vertStart + 0);
	fVisTris.Append(vertStart + 3);
	fVisTris.Append(vertStart + 2);
}

void plCullTree::IVisPoly(const plCullPoly& poly, hsBool dark) const
{
	IVisPolyShape(poly, dark);

	int i;
	for( i = 0; i < poly.fVerts.GetCount()-1; i++ )
	{
		if( !poly.fClipped.IsBitSet(i) )
			IVisPolyEdge(poly.fVerts[i], poly.fVerts[i+1], dark);
	}
	if( !poly.fClipped.IsBitSet(i) )
		IVisPolyEdge(poly.fVerts[i], poly.fVerts[0], dark);
}

void plCullTree::ReleaseCapture() const
{
	fVisVerts.Reset();
	fVisNorms.Reset();
	fVisColors.Reset();
	fVisTris.Reset();
}

///////////////////////////////////////////////////////////////////
// End visualization section of the program
///////////////////////////////////////////////////////////////////

void plCullTree::ISetupScratch(UInt16 nNodes)
{
	ScratchPolys().SetCount(nNodes << 1);
	ScratchPolys().SetCount(0);
}

void plCullTree::Reset()
{
	// Using NodeList as scratch will only work if we use indices,
	// because a push invalidates any pointers we've stored away.
	fNodeList.SetCount(0);

	fRoot = -1;

	ScratchPolys().SetCount(0);
}


void plCullTree::InitFrustum(const hsMatrix44& world2NDC)
{
	Reset();

	fNodeList.SetCount(6);
	fNodeList.SetCount(0);

	Int16		lastIdx = -1;

	plCullNode* node;
	hsVector3 norm;
	hsScalar dist;

	int i;
	for( i = 0; i < 2; i++ )
	{
		
		norm.Set(world2NDC.fMap[3][0] - world2NDC.fMap[i][0], world2NDC.fMap[3][1] - world2NDC.fMap[i][1], world2NDC.fMap[3][2] - world2NDC.fMap[i][2]);
		dist = world2NDC.fMap[3][3] - world2NDC.fMap[i][3];

		IDEBUG_NORMALIZE( norm, dist );

		node = fNodeList.Push();
		node->Init(this, norm, dist);
		node->fOuterChild = lastIdx;
		lastIdx = fNodeList.GetCount()-1;

		norm.Set(world2NDC.fMap[3][0] + world2NDC.fMap[i][0], world2NDC.fMap[3][1] + world2NDC.fMap[i][1], world2NDC.fMap[3][2] + world2NDC.fMap[i][2]);
		dist = world2NDC.fMap[3][3] + world2NDC.fMap[i][3];

		IDEBUG_NORMALIZE( norm, dist );

		node = fNodeList.Push();
		node->Init(this, norm, dist);
		node->fOuterChild = lastIdx;
		lastIdx = fNodeList.GetCount()-1;
	}
	norm.Set(world2NDC.fMap[3][0] - world2NDC.fMap[2][0], world2NDC.fMap[3][1] - world2NDC.fMap[2][1], world2NDC.fMap[3][2] - world2NDC.fMap[2][2]);
	dist = world2NDC.fMap[3][3] - world2NDC.fMap[2][3];

	IDEBUG_NORMALIZE( norm, dist );

	node = fNodeList.Push();
	node->Init(this, norm, dist);
	node->fOuterChild = lastIdx;
	lastIdx = fNodeList.GetCount()-1;

#ifdef SYMMET
	norm.Set(world2NDC.fMap[3][0] + world2NDC.fMap[2][0], world2NDC.fMap[3][1] + world2NDC.fMap[2][1], world2NDC.fMap[3][2] + world2NDC.fMap[2][2]);
	dist = world2NDC.fMap[3][3] + world2NDC.fMap[2][3];
#else // SYMMET
	norm.Set(world2NDC.fMap[2][0], world2NDC.fMap[2][1], world2NDC.fMap[2][2]);
	dist = world2NDC.fMap[2][3];
#endif // SYMMET

	IDEBUG_NORMALIZE( norm, dist );

	node = fNodeList.Push();
	node->Init(this, norm, dist);
	node->fOuterChild = lastIdx;
	lastIdx = fNodeList.GetCount()-1;

	fRoot = fNodeList.GetCount()-1;

#ifdef DEBUG_POINTERS
	if( IGetRoot() )
		IGetRoot()->ISetPointersRecur();
#endif // DEBUG_POINTERS
}

void plCullTree::SetViewPos(const hsPoint3& p)
{
	fViewPos = p;
}

//////////////////////////////////////////////////////////////////////
// Use the tree
//////////////////////////////////////////////////////////////////////
void plCullTree::Harvest(const plSpaceTree* space, hsTArray<Int16>& outList) const
{
	outList.SetCount(0);
	if (!space->IsEmpty())
		IGetRoot()->IHarvest(space, outList);

}

hsBool plCullTree::BoundsVisible(const hsBounds3Ext& bnd) const
{
	return plCullNode::kCulled != IGetRoot()->ITestBoundsRecur(bnd);
}

hsBool plCullTree::SphereVisible(const hsPoint3& center, hsScalar rad) const
{
	return plCullNode::kCulled != IGetRoot()->ITestSphereRecur(center, rad);
}

