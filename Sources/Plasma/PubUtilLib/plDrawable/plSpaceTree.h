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

#ifndef plSpaceTree_inc
#define plSpaceTree_inc

#include "hsTemplates.h"
#include "hsBounds.h"
#include "pnFactory/plCreatable.h"
#include "hsBitVector.h"

class hsStream;
class hsResMgr;
class plVolumeIsect;

class plSpaceTreeNode 
{
public:
    enum {
        kNone           = 0x0,
        kIsLeaf         = 0x1,
        kDirty          = 0x2,
        kDisabled       = 0x4,
        kEmpty          = 0x8,
        kEnabledTemp    = 0x10
    };

    hsBounds3Ext        fWorldBounds;

    uint16_t              fFlags; // mostly for alignment;
    int16_t               fParent;
    union {
        int16_t               fChildren[2]; // Children are actually in same array as parent
        int16_t               fLeafIndex;
    };

    int16_t               GetChild(int w) const { hsAssert(!(fFlags & kIsLeaf), "Getting Child of leaf node"); return fChildren[w]; }
    int16_t               GetParent() const { return fParent; }
    int16_t               GetLeaf() const { hsAssert(fFlags & kIsLeaf, "Getting leaf idx off interior node"); return fLeafIndex; }
    bool                IsLeaf() const { return 0 != (fFlags & kIsLeaf); }
    const hsBounds3Ext& GetWorldBounds() const { return fWorldBounds; }

    // Kind of hate this. Would like to blast the whole thing in, but
    // the bounds are a real class.
    void                Read(hsStream* s);
    void                Write(hsStream* s);
};


class plSpaceTree : public plCreatable
{
public:
    enum plHarvestFlags {
        kNone                   = 0x0,
        kSortBackToFront        = 0x1,
        kSortFrontToBack        = 0x2
    };
    enum {
        kRootParent = -1
    };
private:
    hsTArray<plSpaceTreeNode>       fTree;
    const hsBitVector*              fCache;

    int32_t                           fNumLeaves;
    int16_t                           fRoot;

    uint16_t                          fHarvestFlags;

    mutable plVolumeIsect*          fCullFunc;

    hsPoint3                        fViewPos;

    void        IRefreshRecur(int16_t which);
    
    void        IHarvestAndCullLeaves(const plSpaceTreeNode& subRoot, hsTArray<int16_t>& list) const;
    void        IHarvestLeaves(const plSpaceTreeNode& subRoot, hsTArray<int16_t>& list) const;
    
    void        IHarvestAndCullLeaves(const plSpaceTreeNode& subRoot, hsBitVector& totList, hsBitVector& list) const;
    void        IHarvestLeaves(const plSpaceTreeNode& subRoot, hsBitVector& totList, hsBitVector& list) const;

    void        IHarvestLevel(int16_t subRoot, int level, int currLevel, hsTArray<int16_t>& list) const;

    void        IHarvestAndCullEnabledLeaves(int16_t subRoot, const hsBitVector& cache, std::vector<int16_t>& list) const;
    void        IHarvestEnabledLeaves(int16_t subRoot, const hsBitVector& cache, std::vector<int16_t>& list) const;
    void        IHarvestEnabledLeaves(int16_t subIdx, const hsBitVector& cache, hsBitVector& totList, hsBitVector& list) const;

    void        IEnableLeaf(int16_t idx, hsBitVector& cache) const;

public:
    plSpaceTree();
    virtual ~plSpaceTree();

    CLASSNAME_REGISTER( plSpaceTree );
    GETINTERFACE_ANY( plSpaceTree, plCreatable );

    void SetViewPos(const hsPoint3& p) { fViewPos = p; }
    const hsPoint3& GetViewPos() const { return fViewPos; }

    const plSpaceTreeNode&  GetNode(int16_t w) const { return fTree[w]; }
    int16_t                   GetRoot() const { return fRoot; }
    bool                    IsRoot(int16_t w) const { return fRoot == w; }
    bool                    IsLeaf(int16_t w) const { return GetNode(w).IsLeaf(); }

    void HarvestLeaves(hsBitVector& totList, hsBitVector& list) const;
    void HarvestLeaves(hsBitVector& list) const;
    void HarvestLeaves(plVolumeIsect* cullFunc, hsBitVector& list) const;
    void HarvestLeaves(int16_t subRoot, hsBitVector& list) const;
    void HarvestLeaves(int16_t subRoot, hsBitVector& totList, hsBitVector& list) const;

    void HarvestLeaves(std::vector<int16_t>& list) const;
    void HarvestLeaves(plVolumeIsect* cullFunc, std::vector<int16_t>& list) const;
    void HarvestLeaves(int16_t subRoot, std::vector<int16_t>& list) const;

    void EnableLeaf(int16_t idx, hsBitVector& cache) const;
    void EnableLeaves(const std::vector<int16_t>& list, hsBitVector& cache) const;
    void HarvestEnabledLeaves(plVolumeIsect* cullFunc, const hsBitVector& cache, std::vector<int16_t>& list) const;
    void SetCache(const hsBitVector* cache) { fCache = cache; }

    void SetHarvestFlags(plHarvestFlags f) { fHarvestFlags = f; }
    uint16_t GetHarvestFlags() const { return fHarvestFlags; }

    uint16_t HasLeafFlag(int16_t w, uint16_t f) const { return GetNode(w).fFlags & f; }
    void SetLeafFlag(int16_t w, uint16_t f, bool on=true);
    void ClearLeafFlag(int16_t w, uint16_t f);
    void ClearTreeFlag(uint16_t f);
    void SetTreeFlag(uint16_t f, bool on=true);

    bool IsDisabled(uint16_t w) const { return (GetNode(w).fFlags & plSpaceTreeNode::kDisabled) || (fCache && !fCache->IsBitSet(w)); }

    // Should GetWorldBounds check and refresh if needed?
    const hsBounds3Ext& GetWorldBounds() const { return GetNode(GetRoot()).fWorldBounds; }

    void MoveLeaf(int16_t idx, const hsBounds3Ext& newWorldBnd);
    void Refresh();
    bool IsEmpty() const { return 0 != (GetNode(GetRoot()).fFlags & plSpaceTreeNode::kEmpty); }
    bool IsDirty() const { return 0 != (GetNode(GetRoot()).fFlags & plSpaceTreeNode::kDirty); }
    void MakeDirty() { fTree[GetRoot()].fFlags |= plSpaceTreeNode::kDirty; }

    int32_t GetNumLeaves() const { return fNumLeaves; }

    void Read(hsStream* s, hsResMgr* mgr) override;
    void Write(hsStream* s, hsResMgr* mgr) override;

    void HarvestLevel(int level, hsTArray<int16_t>& list) const;

    friend class plSpaceTreeMaker;
};

#endif // plSpaceTree_inc
