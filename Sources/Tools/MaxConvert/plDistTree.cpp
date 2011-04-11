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
#include "Max.h"


#include "plDistTree.h"

plDistTree::plDistTree()
:	fRoot(-1)
{
}

plDistTree::~plDistTree()
{
}

void plDistTree::Reset()
{
	fRoot = -1;
	fNodes.Reset();
}

void plDistTree::AddBoxIData(const Box3& box, const Box3& fade, UInt32 iData)
{
	fRoot = IAddNodeRecur(fRoot, box, fade, iData);
}

BOOL plDistTree::BoxClear(const Box3& box, const Box3& fade) const
{
	return IBoxClearRecur(fRoot, box, fade);
}

BOOL plDistTree::PointClear(const Point3& pt, const Box3& fade) const
{
	return IPointClearRecur(fRoot, pt, fade);
}

BOOL plDistTree::IFadesClear(const Box3& fade0, const Box3& fade1) const
{
	// Only two ways fade can come out non-overlapping.
	// Either fade0 fades out before fade1 fades in, or v.v.

	// First case, does fade0 fade out?
	if( fade0.Max()[2] > 0 )
	{
		// does fade1 fade in?
		if( fade1.Min()[2] < 0 )
		{
			// Okay they do, does fade0 fade out before fade1 fades in?
			if( fade0.Max()[0] <= fade1.Min()[0] )
				return true;
		}
	}
	// Second case, same thing but reversed order
	if( fade1.Max()[2] > 0 )
	{
		// does fade0 fade in?
		if( fade0.Min()[2] < 0 )
		{
			// Okay they do, does fade1 fade out before fade0 fades in?
			if( fade1.Max()[0] <= fade0.Min()[0] )
				return true;
		}
	}
	return false;
}

BOOL plDistTree::IBox0ContainsBox1(const Box3& box0, const Box3& box1, const Box3& fade0, const Box3& fade1) const
{
#ifdef MAX_CONTAINS_WORKS
	if( !box0.Contains(box1) )
		return false;
#else MAX_CONTAINS_WORKS
	if( (box0.Min()[0] > box1.Min()[0])
		||(box0.Min()[1] > box1.Min()[1])
		||(box0.Min()[2] > box1.Min()[2])
		||(box0.Max()[0] < box1.Max()[0])
		||(box0.Max()[1] < box1.Max()[1])
		||(box0.Max()[2] < box1.Max()[2]) )
		return false;
#endif // MAX_CONTAINS_WORKS

	if( IFadesClear(fade0, fade1) )
		return false;

	return true;
}

BOOL plDistTree::IBoxesClear(const Box3& box0, const Box3& box1) const
{
	return (box0.Min()[0] > box1.Max()[0])
		||(box0.Max()[0] < box1.Min()[0])

		||(box0.Min()[1] > box1.Max()[1])
		||(box0.Max()[1] < box1.Min()[1])

		||(box0.Min()[2] > box1.Max()[2])
		||(box0.Max()[2] < box1.Min()[2]);
}

BOOL plDistTree::IBoxClearRecur(Int32 iNode, const Box3& box, const Box3& fade) const
{
	if( iNode < 0 )
		return true;

	if( IBoxesClear(fNodes[iNode].fBox, box) )
		return true;

	if( IFadesClear(fNodes[iNode].fFade, fade) )
		return true;

	if( fNodes[iNode].IsLeaf() )
		return false;

	int i;
	for( i = 0; i < 8; i++ )
	{
		if( !IBoxClearRecur(fNodes[iNode].fChildren[i], box, fade) )
			return false;
	}
	return true;
}

BOOL plDistTree::IPointClearRecur(Int32 iNode, const Point3& pt, const Box3& fade) const
{
	if( iNode < 0 )
		return true;

	if( !fNodes[iNode].fBox.Contains(pt) )
		return true;

	if( IFadesClear(fNodes[iNode].fFade, fade) )
		return true;

	if( fNodes[iNode].IsLeaf() )
		return false;

	int i;
	for( i = 0; i < 8; i++ )
	{
		if( !IPointClearRecur(fNodes[iNode].fChildren[i], pt, fade) )
			return false;
	}
	return true;
}


Int32 plDistTree::IAddNodeRecur(Int32 iNode, const Box3& box, const Box3& fade, UInt32 iData)
{
	// if iNode < 0, make a node for box and return that.
	if( iNode < 0 )
	{
		return INextNode(box, fade, iData);
	}

	// if the box is contained
	//		if this node is a leaf, pitch the box
	//
	//		else
	//			recur on one of 8 children, based on
	//			box's center relative to node center.

	// if the box doesn't intercect this node,
	//		replace this node with a node of combined boxes.
	//		this node becomes sibling of box.

	// if the box does intercect, but isn't contained
	//		same thing.


#if 0
	if( IBox0ContainsBox1(fNodes[iNode].fBox, box, fNodes[iNode].fFade, fade) )
	{
		if( !fNodes[iNode].IsLeaf() )
#else
	if( !fNodes[iNode].IsLeaf() && IBox0ContainsBox1(fNodes[iNode].fBox, box, fNodes[iNode].fFade, fade) )
	{
#endif
		{
			Int32 iChild = IGetChild(fNodes[iNode].fBox, box);
		
			Int32 iChildNode = IAddNodeRecur(fNodes[iNode].fChildren[iChild], box, fade, iData);
			fNodes[iNode].fChildren[iChild] = iChildNode;

			fNodes[iNode].fBox += fNodes[fNodes[iNode].fChildren[iChild]].fBox;
		}
		return iNode;
	}
	else
	{
		return IMergeNodes(iNode, box, fade, iData);
	}
}

Int32 plDistTree::IMergeNodes(Int32 iNode, const Box3& box, const Box3& fade, UInt32 iData)
{
	Box3 parBox = box;
	parBox += fNodes[iNode].fBox;

	Int32 pNode = INextNode(parBox, NonFade(), UInt32(-1));
	Int32 iChild = IGetChild(parBox, box);

	Int32 cNode = INextNode(box, fade, iData);

	fNodes[pNode].fChildren[iChild] = cNode;

	// Put the original node in the opposite quadrant from the child.
	// This handles the case where one of the bounds completely contains
	// the other. The octant structure of the tree isn't relied on, it
	// only helps balance the tree. So being wrong here won't hurt anything.
	iChild = iChild ^ 0x7;

	fNodes[pNode].fChildren[iChild] = iNode;

	fNodes[pNode].SetIsLeaf(false);

	return pNode;
}

Int32 plDistTree::IGetChild(const Box3& parent, const Box3& child) const
{
	Point3 parCenter = parent.Center();
	Point3 chiCenter = child.Center();

	Int32 idx = ((parCenter[0] < chiCenter[0]) << 0)
		| ((parCenter[1] < chiCenter[1]) << 1)
		| ((parCenter[2] < chiCenter[2]) << 2);
	return idx;
}

Int32 plDistTree::INextNode(const Box3& box, const Box3& fade, UInt32 iData)
{
	Int32 iNode = fNodes.GetCount();

	fNodes.Push();

	fNodes[iNode].fFlags = plDistNode::kIsLeaf;
	fNodes[iNode].fBox = box;
	fNodes[iNode].fFade = fade;
	fNodes[iNode].fIData = iData;
	fNodes[iNode].fChildren[0]
		= fNodes[iNode].fChildren[1]
		= fNodes[iNode].fChildren[2]
		= fNodes[iNode].fChildren[3]
		= fNodes[iNode].fChildren[4]
		= fNodes[iNode].fChildren[5]
		= fNodes[iNode].fChildren[6]
		= fNodes[iNode].fChildren[7] = -1;

	return iNode;
}

void plDistTree::HarvestBox(const Box3& box, Tab<Int32>& out) const
{
	IHarvestBoxRecur(fRoot, box, out);
}

void plDistTree::IHarvestBoxRecur(Int32 iNode, const Box3& box, Tab<Int32>& out) const
{
	if( iNode < 0 )
		return;

	if( IBoxesClear(fNodes[iNode].fBox, box) )
		return;

	if( fNodes[iNode].IsLeaf() )
	{
		out.Append(1, &iNode);
	}
	else
	{
		int i;
		for( i = 0; i < 8; i++ )
			IHarvestBoxRecur(fNodes[iNode].fChildren[i], box, out);
	}
}