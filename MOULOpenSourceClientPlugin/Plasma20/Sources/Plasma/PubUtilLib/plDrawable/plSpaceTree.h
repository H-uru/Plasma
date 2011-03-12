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

#ifndef plSpaceTree_inc
#define plSpaceTree_inc

#include "hsTemplates.h"
#include "hsBounds.h"
#include "../pnFactory/plCreatable.h"
#include "hsBitVector.h"

class hsStream;
class hsResMgr;
class plVolumeIsect;

class plSpaceTreeNode 
{
public:
	enum {
		kNone			= 0x0,
		kIsLeaf			= 0x1,
		kDirty			= 0x2,
		kDisabled		= 0x4,
		kEmpty			= 0x8,
		kEnabledTemp	= 0x10
	};

	hsBounds3Ext		fWorldBounds;

	UInt16				fFlags; // mostly for alignment;
	Int16				fParent;
	union {
		Int16				fChildren[2]; // Children are actually in same array as parent
		Int16				fLeafIndex;
	};

	Int16				GetChild(int w) const { hsAssert(!(fFlags & kIsLeaf), "Getting Child of leaf node"); return fChildren[w]; }
	Int16				GetParent() const { return fParent; }
	Int16				GetLeaf() const { hsAssert(fFlags & kIsLeaf, "Getting leaf idx off interior node"); return fLeafIndex; }
	hsBool				IsLeaf() const { return 0 != (fFlags & kIsLeaf); }
	const hsBounds3Ext&	GetWorldBounds() const { return fWorldBounds; }

	// Kind of hate this. Would like to blast the whole thing in, but
	// the bounds are a real class.
	void				Read(hsStream* s);
	void				Write(hsStream* s);
};


class plSpaceTree : public plCreatable
{
public:
	enum plHarvestFlags {
		kNone					= 0x0,
		kSortBackToFront		= 0x1,
		kSortFrontToBack		= 0x2
	};
	enum {
		kRootParent = -1
	};
private:
	hsTArray<plSpaceTreeNode>		fTree;
	const hsBitVector*				fCache;

	Int16							fRoot;
	Int16							fNumLeaves;

	UInt16							fHarvestFlags;

	mutable plVolumeIsect*			fCullFunc;

	hsPoint3						fViewPos;

	void		IRefreshRecur(Int16 which);
	
	void		IHarvestAndCullLeaves(const plSpaceTreeNode& subRoot, hsTArray<Int16>& list) const;
	void		IHarvestLeaves(const plSpaceTreeNode& subRoot, hsTArray<Int16>& list) const;
	
	void		IHarvestAndCullLeaves(const plSpaceTreeNode& subRoot, hsBitVector& totList, hsBitVector& list) const;
	void		IHarvestLeaves(const plSpaceTreeNode& subRoot, hsBitVector& totList, hsBitVector& list) const;

	void		IHarvestLevel(Int16 subRoot, int level, int currLevel, hsTArray<Int16>& list) const;

	void		IHarvestAndCullEnabledLeaves(Int16 subRoot, const hsBitVector& cache, hsTArray<Int16>& list) const;
	void		IHarvestEnabledLeaves(Int16 subRoot, const hsBitVector& cache, hsTArray<Int16>& list) const;
	void		IHarvestEnabledLeaves(Int16 subIdx, const hsBitVector& cache, hsBitVector& totList, hsBitVector& list) const;

	void		IEnableLeaf(Int16 idx, hsBitVector& cache) const;

public:
	plSpaceTree();
	virtual ~plSpaceTree();

	CLASSNAME_REGISTER( plSpaceTree );
	GETINTERFACE_ANY( plSpaceTree, plCreatable );

	void SetViewPos(const hsPoint3& p) { fViewPos = p; }
	const hsPoint3& GetViewPos() const { return fViewPos; }

	const plSpaceTreeNode&	GetNode(Int16 w) const { return fTree[w]; }
	Int16					GetRoot() const { return fRoot; }
	hsBool					IsRoot(Int16 w) const { return fRoot == w; }
	hsBool					IsLeaf(Int16 w) const { return GetNode(w).IsLeaf(); }

	void HarvestLeaves(hsBitVector& totList, hsBitVector& list) const;
	void HarvestLeaves(hsBitVector& list) const;
	void HarvestLeaves(plVolumeIsect* cullFunc, hsBitVector& list) const;
	void HarvestLeaves(Int16 subRoot, hsBitVector& list) const;
	void HarvestLeaves(Int16 subRoot, hsBitVector& totList, hsBitVector& list) const;

	void HarvestLeaves(hsTArray<Int16>& list) const;
	void HarvestLeaves(plVolumeIsect* cullFunc, hsTArray<Int16>& list) const;
	void HarvestLeaves(Int16 subRoot, hsTArray<Int16>& list) const;

	void EnableLeaf(Int16 idx, hsBitVector& cache) const;
	void EnableLeaves(const hsTArray<Int16>& list, hsBitVector& cache) const;
	void HarvestEnabledLeaves(plVolumeIsect* cullFunc, const hsBitVector& cache, hsTArray<Int16>& list) const;
	void SetCache(const hsBitVector* cache) { fCache = cache; }

	void BitVectorToList(hsTArray<Int16>& list, const hsBitVector& bitVec) const;

	void SetHarvestFlags(plHarvestFlags f) { fHarvestFlags = f; }
	UInt16 GetHarvestFlags() const { return fHarvestFlags; }

	UInt16 HasLeafFlag(Int16 w, UInt16 f) const { return GetNode(w).fFlags & f; }
	void SetLeafFlag(Int16 w, UInt16 f, hsBool on=true);
	void ClearLeafFlag(Int16 w, UInt16 f);
	void ClearTreeFlag(UInt16 f);
	void SetTreeFlag(UInt16 f, hsBool on=true);

	hsBool IsDisabled(UInt16 w) const { return (GetNode(w).fFlags & plSpaceTreeNode::kDisabled) || (fCache && !fCache->IsBitSet(w)); }

	// Should GetWorldBounds check and refresh if needed?
	const hsBounds3Ext& GetWorldBounds() const { return GetNode(GetRoot()).fWorldBounds; }

	void MoveLeaf(Int16 idx, const hsBounds3Ext& newWorldBnd);
	void Refresh();
	hsBool IsEmpty() const { return 0 != (GetNode(GetRoot()).fFlags & plSpaceTreeNode::kEmpty); }
	hsBool IsDirty() const { return 0 != (GetNode(GetRoot()).fFlags & plSpaceTreeNode::kDirty); }
	void MakeDirty() { fTree[GetRoot()].fFlags |= plSpaceTreeNode::kDirty; }

	Int16 GetNumLeaves() const { return fNumLeaves; }

	virtual void Read(hsStream* s, hsResMgr* mgr);
	virtual void Write(hsStream* s, hsResMgr* mgr);

	void HarvestLevel(int level, hsTArray<Int16>& list) const;

	friend class plSpaceTreeMaker;
};

#endif // plSpaceTree_inc
