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
#include "plSpaceTree.h"
#include "hsStream.h"
#include "hsBitVector.h"
#include "plProfile.h"

#include "../plIntersect/plVolumeIsect.h"
#include "../plMath/hsRadixSort.h"

static hsBitVector scratchTotVec;
static hsBitVector scratchBitVec;
static hsTArray<Int16> scratchList;
static hsTArray<hsRadixSort::Elem> scratchSort;

plProfile_CreateCounter("Harvest Leaves", "Draw", HarvestLeaves);

void plSpaceTreeNode::Read(hsStream* s)
{
	fWorldBounds.Read(s);

	fFlags = s->ReadSwap16();
	fParent = s->ReadSwap16();
	fChildren[0] = s->ReadSwap16();
	fChildren[1] = s->ReadSwap16();

}

void plSpaceTreeNode::Write(hsStream* s)
{
	fWorldBounds.Write(s);

	s->WriteSwap16(fFlags);
	s->WriteSwap16(fParent);
	s->WriteSwap16(fChildren[0]);
	if( fFlags & kIsLeaf )
		// Temp for now to play nice with binary patches
		s->WriteSwap16( 0 );
	else
		s->WriteSwap16(fChildren[1]);
}

plSpaceTree::plSpaceTree()
:	fCullFunc(nil),
	fNumLeaves(0),
	fCache(nil)
{
}

plSpaceTree::~plSpaceTree()
{
}

void plSpaceTree::IRefreshRecur(Int16 which)
{
	plSpaceTreeNode& sub = fTree[which];

	if( sub.fFlags & plSpaceTreeNode::kIsLeaf )
	{
		sub.fFlags &= ~plSpaceTreeNode::kDirty;
		return;
	}
	
	if( sub.fFlags & plSpaceTreeNode::kDirty )
	{
		IRefreshRecur(sub.fChildren[0]);
		IRefreshRecur(sub.fChildren[1]);

		sub.fWorldBounds.MakeEmpty();
		if( !(fTree[sub.fChildren[0]].fFlags & plSpaceTreeNode::kDisabled) )
			sub.fWorldBounds.Union(&fTree[sub.fChildren[0]].fWorldBounds);
		if( !(fTree[sub.fChildren[1]].fFlags & plSpaceTreeNode::kDisabled) )
			sub.fWorldBounds.Union(&fTree[sub.fChildren[1]].fWorldBounds);

		sub.fFlags &= ~plSpaceTreeNode::kDirty;
	}
}

void plSpaceTree::Refresh()
{
	if( !IsEmpty() )
		IRefreshRecur(fRoot);
}

void plSpaceTree::SetTreeFlag(UInt16 f, hsBool on)
{
	if( IsEmpty() )
		return;

	if( !on )
	{
		ClearTreeFlag(f);
		return;
	}

	int i;
	for( i = 0; i < fTree.GetCount(); i++ )
		fTree[i].fFlags |= f;
}

void plSpaceTree::ClearTreeFlag(UInt16 f)
{
	if( IsEmpty() )
		return;

	int i;
	for( i = 0; i < fTree.GetCount(); i++ )
		fTree[i].fFlags &= ~f;
}

void plSpaceTree::SetLeafFlag(Int16 idx, UInt16 f, hsBool on)
{
	if( IsEmpty() )
		return;

	hsAssert(idx == fTree[idx].fLeafIndex, "Some scrambling of indices");

	if( !on )
	{
		ClearLeafFlag(idx, f);
		return;
	}

	fTree[idx].fFlags |= f;

	idx = fTree[idx].fParent;

	while( idx != kRootParent )
	{
		if( (fTree[idx].fFlags & f)
			|| !(fTree[fTree[idx].fChildren[0]].fFlags & fTree[fTree[idx].fChildren[1]].fFlags & f) )
		{
			idx = kRootParent;
		}
		else
		{
			fTree[idx].fFlags |= f;
			idx = fTree[idx].fParent;
		}
	}
}

void plSpaceTree::ClearLeafFlag(Int16 idx, UInt16 f)
{
	hsAssert(idx == fTree[idx].fLeafIndex, "Some scrambling of indices");

	while( idx != kRootParent )
	{
		if( !(fTree[idx].fFlags & f) )
		{
			return;
		}
		else
		{
			fTree[idx].fFlags &= ~f;
			idx = fTree[idx].fParent;
		}
	}

}

inline void plSpaceTree::IEnableLeaf(Int16 idx, hsBitVector& cache) const
{

	cache.SetBit(idx);

	idx = fTree[idx].fParent;

	while( idx != kRootParent )
	{
		if( cache.IsBitSet(idx) )
		{
			return;
		}
		else
		{
			cache.SetBit(idx);
			idx = fTree[idx].fParent;
		}
	}
}

void plSpaceTree::EnableLeaf(Int16 idx, hsBitVector& cache) const
{
	IEnableLeaf(idx, cache);
}

void plSpaceTree::EnableLeaves(const hsTArray<Int16>& list, hsBitVector& cache) const
{
	if( IsEmpty() )
		return;
	int i;
	for( i = 0; i < list.GetCount(); i++ )
	{
		IEnableLeaf(list[i], cache);
	}
}

void plSpaceTree::IHarvestAndCullEnabledLeaves(Int16 subIdx, const hsBitVector& cache, hsTArray<Int16>& list) const
{
	if( !cache.IsBitSet(subIdx) )
		return;

	const plSpaceTreeNode& subRoot = fTree[subIdx];

	plVolumeCullResult res = fCullFunc->Test(subRoot.fWorldBounds);
	if( res == kVolumeCulled )
		return;

	if( subRoot.fFlags & plSpaceTreeNode::kIsLeaf )
	{
		list.Append(subIdx);
	}
	else
	{
		if( res == kVolumeClear )
		{
			IHarvestEnabledLeaves(subRoot.fChildren[0], cache, list);
			IHarvestEnabledLeaves(subRoot.fChildren[1], cache, list);
		}
		else
		{
			IHarvestAndCullEnabledLeaves(subRoot.fChildren[0], cache, list);
			IHarvestAndCullEnabledLeaves(subRoot.fChildren[1], cache, list);
		}
	}
}

void plSpaceTree::IHarvestEnabledLeaves(Int16 subIdx, const hsBitVector& cache, hsTArray<Int16>& list) const
{
	if( !cache.IsBitSet(subIdx) )
		return;

	const plSpaceTreeNode& subRoot = fTree[subIdx];

	if( subRoot.fFlags & plSpaceTreeNode::kIsLeaf )
	{
		plProfile_Inc(HarvestLeaves);
		list.Append(subIdx);
	}
	else
	{
		IHarvestEnabledLeaves(subRoot.fChildren[0], cache, list);
		IHarvestEnabledLeaves(subRoot.fChildren[1], cache, list);
	}
}

void plSpaceTree::HarvestEnabledLeaves(plVolumeIsect* cull, const hsBitVector& cache, hsTArray<Int16>& list) const
{
	if( IsEmpty() )
		return;

	if( fCullFunc = cull )
		IHarvestAndCullEnabledLeaves(fRoot, cache, list);
	else
		IHarvestEnabledLeaves(fRoot, cache, list);
}

void plSpaceTree::IHarvestEnabledLeaves(Int16 subIdx, const hsBitVector& cache, hsBitVector& totList, hsBitVector& list) const
{
	if( IsDisabled(subIdx) )
		return;

	if( totList.IsBitSet(subIdx) )
		return;

	totList.SetBit(subIdx);

	const plSpaceTreeNode& subRoot = fTree[subIdx];
	if( subRoot.fFlags & plSpaceTreeNode::kIsLeaf )
	{
		plProfile_Inc(HarvestLeaves);
		list.SetBit(subIdx);
	}
	else
	{
		IHarvestEnabledLeaves(subRoot.fChildren[0], cache, totList, list);
		IHarvestEnabledLeaves(subRoot.fChildren[1], cache, totList, list);
	}
}

void plSpaceTree::MoveLeaf(Int16 idx, const hsBounds3Ext& bnd)
{
	hsAssert(idx == fTree[idx].fLeafIndex, "Some scrambling of indices");

	fTree[idx].fWorldBounds = bnd;

	while( idx != kRootParent )
	{
		if( fTree[idx].fFlags & plSpaceTreeNode::kDirty )
		{
			idx = kRootParent;
		}
		else
		{
			fTree[idx].fFlags |= plSpaceTreeNode::kDirty;
			idx = fTree[idx].fParent;
		}
	}
}

void plSpaceTree::HarvestLeaves(Int16 subRoot, hsBitVector& totList, hsBitVector& list) const
{
	if( !IsEmpty() )
	{
		if( fCache )
		{
			IHarvestEnabledLeaves(subRoot, *fCache, totList, list);
		}
		else
		{
			IHarvestLeaves(fTree[subRoot], totList, list);
		}
	}
}

void plSpaceTree::HarvestLeaves(hsBitVector& totList, hsBitVector& list) const
{
	if( !IsEmpty() )
		IHarvestLeaves(fTree[fRoot], totList, list);
}

void plSpaceTree::HarvestLeaves(hsBitVector& list) const
{
	if( !IsEmpty() )
		IHarvestLeaves(fTree[fRoot], scratchTotVec, list);
	scratchTotVec.Clear();
}

void plSpaceTree::HarvestLeaves(plVolumeIsect* cull, hsBitVector& list) const
{
	if( !IsEmpty() )
	{
		if( fCullFunc = cull )
			IHarvestAndCullLeaves(fTree[fRoot], scratchTotVec, list);
		else
			IHarvestLeaves(fTree[fRoot], scratchTotVec, list);
	}
	scratchTotVec.Clear();
}

void plSpaceTree::HarvestLeaves(Int16 subRoot, hsBitVector& list) const 
{ 
	IHarvestLeaves(GetNode(subRoot), scratchTotVec, list);
	scratchTotVec.Clear();
}

void plSpaceTree::HarvestLeaves(plVolumeIsect* cull, hsTArray<Int16>& list) const
{
	if( !IsEmpty() )
	{
		scratchBitVec.Clear();
		HarvestLeaves(cull, scratchBitVec);

		BitVectorToList(list, scratchBitVec);
	}
}

void plSpaceTree::HarvestLeaves(hsTArray<Int16>& list) const 
{
	if( !IsEmpty() )
	{
		scratchBitVec.Clear();
		HarvestLeaves(scratchBitVec);

		BitVectorToList(list, scratchBitVec);
	}
}

void plSpaceTree::HarvestLeaves(Int16 subRoot, hsTArray<Int16>& list) const 
{ 
	if( !IsEmpty() )
	{
		scratchBitVec.Clear();

		HarvestLeaves(subRoot, scratchBitVec);

		BitVectorToList(list, scratchBitVec);
	}
}

void plSpaceTree::BitVectorToList(hsTArray<Int16>& list, const hsBitVector& bitVec) const
{
#if 0 // added func to bitvector
	int i;
	for( i = 0; i < fNumLeaves; i++ )
	{
		if( bitVec.IsBitSet(i) )
			list.Append(i);
	}
#else  // added func to bitvector
	bitVec.Enumerate(list);
#endif  // added func to bitvector
}

void plSpaceTree::IHarvestAndCullLeaves(const plSpaceTreeNode& subRoot, hsBitVector& totList, hsBitVector& list) const
{
	if( subRoot.fFlags & plSpaceTreeNode::kDisabled )
		return;

	int idx = &subRoot - &fTree[0];
	if( totList.IsBitSet(idx) )
		return;

	hsAssert(fCullFunc, "Oops");
	plVolumeCullResult res = fCullFunc->Test(subRoot.fWorldBounds);
	if( res == kVolumeCulled )
		return;

	if( subRoot.fFlags & plSpaceTreeNode::kIsLeaf )
	{
		totList.SetBit(idx);

		plProfile_Inc(HarvestLeaves);
		list.SetBit(subRoot.fLeafIndex);
	}
	else
	{
		if( res == kVolumeClear )
		{
			totList.SetBit(idx);

			IHarvestLeaves(fTree[subRoot.fChildren[0]], totList, list);
			IHarvestLeaves(fTree[subRoot.fChildren[1]], totList, list);
		}
		else
		{
			IHarvestAndCullLeaves(fTree[subRoot.fChildren[0]], totList, list);
			IHarvestAndCullLeaves(fTree[subRoot.fChildren[1]], totList, list);
		}
	}
}

void plSpaceTree::IHarvestAndCullLeaves(const plSpaceTreeNode& subRoot, hsTArray<Int16>& list) const
{
	if( subRoot.fFlags & plSpaceTreeNode::kDisabled )
		return;

	hsAssert(fCullFunc, "Oops");
	plVolumeCullResult res = fCullFunc->Test(subRoot.fWorldBounds);
	if( res == kVolumeCulled )
		return;

	if( subRoot.fFlags & plSpaceTreeNode::kIsLeaf )
	{
		plProfile_Inc(HarvestLeaves);
		list.Append(subRoot.fLeafIndex);
	}
	else
	{
		if( res == kVolumeClear )
		{
			IHarvestLeaves(fTree[subRoot.fChildren[0]], list);
			IHarvestLeaves(fTree[subRoot.fChildren[1]], list);
		}
		else
		{
			IHarvestAndCullLeaves(fTree[subRoot.fChildren[0]], list);
			IHarvestAndCullLeaves(fTree[subRoot.fChildren[1]], list);
		}
	}
}

void plSpaceTree::IHarvestLeaves(const plSpaceTreeNode& subRoot, hsBitVector& totList, hsBitVector& list) const
{
	if( subRoot.fFlags & plSpaceTreeNode::kDisabled )
		return;

	int idx = &subRoot - &fTree[0];
	if( totList.IsBitSet(idx) )
		return;

	totList.SetBit(idx);

	if( subRoot.fFlags & plSpaceTreeNode::kIsLeaf )
	{
		plProfile_Inc(HarvestLeaves);
		list.SetBit(subRoot.fLeafIndex);
	}
	else
	{
		IHarvestLeaves(fTree[subRoot.fChildren[0]], totList, list);
		IHarvestLeaves(fTree[subRoot.fChildren[1]], totList, list);
	}
}

void plSpaceTree::IHarvestLeaves(const plSpaceTreeNode& subRoot, hsTArray<Int16>& list) const
{
	if( subRoot.fFlags & plSpaceTreeNode::kDisabled )
		return;
	if( subRoot.fFlags & plSpaceTreeNode::kIsLeaf )
	{
		plProfile_Inc(HarvestLeaves);
		list.Append(subRoot.fLeafIndex);
	}
	else
	{
		IHarvestLeaves(fTree[subRoot.fChildren[0]], list);
		IHarvestLeaves(fTree[subRoot.fChildren[1]], list);
	}
}

void plSpaceTree::Read(hsStream* s, hsResMgr* mgr)
{
	plCreatable::Read(s, mgr);

	fRoot = s->ReadSwap16();

	fNumLeaves = UInt16(s->ReadSwap32());

	UInt32 n = s->ReadSwap32();
	fTree.SetCount(n);
	int i;
	for( i = 0; i < n; i++ )
		fTree[i].Read(s);
}

void plSpaceTree::Write(hsStream* s, hsResMgr* mgr)
{
	plCreatable::Write(s, mgr);

	s->WriteSwap16(fRoot);

	s->WriteSwap32(fNumLeaves);

	s->WriteSwap32(fTree.GetCount());
	int i;
	for( i = 0; i < fTree.GetCount(); i++ )
	{
		fTree[i].Write(s);
	}
}

// Some debug only stuff

void plSpaceTree::HarvestLevel(int level, hsTArray<Int16>& list) const
{
	if( !IsEmpty() )
	{
		IHarvestLevel(fRoot, level, 0, list);
	}
}

void plSpaceTree::IHarvestLevel(Int16 subRoot, int level, int currLevel, hsTArray<Int16>& list) const
{
	if( level == currLevel )
	{
		list.Append(subRoot);
		return;
	}
	if( IsLeaf(subRoot) )
		return;

	IHarvestLevel(GetNode(subRoot).GetChild(0), level, currLevel+1, list);
	IHarvestLevel(GetNode(subRoot).GetChild(1), level, currLevel+1, list);
}

