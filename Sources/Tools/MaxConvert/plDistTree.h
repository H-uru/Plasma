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

#ifndef plDistTree_inc
#define plDistTree_inc

#include "hsTemplates.h"

class plDistNode
{
public:
    enum {
        kIsLeaf = 0x1
    };
    uint32_t      fFlags;

    int32_t       fChildren[8];
    Box3        fBox;
    Box3        fFade;

    union
    {
        void*       fPData;
        uint32_t      fIData;
    };

    const Box3& GetBox() const { return fBox; }

    BOOL        IsLeaf() const { return 0 != (fFlags & kIsLeaf); }
    void        SetIsLeaf(BOOL on) { if(on)fFlags |= kIsLeaf; else fFlags &= ~kIsLeaf; }
};

class plDistTree
{
protected:
    
    int32_t                           fRoot;

    hsTArray<plDistNode>            fNodes;

    int32_t   IAddNodeRecur(int32_t iNode, const Box3& box, const Box3& fade, uint32_t iData);

    int32_t   IMergeNodes(int32_t iNode, const Box3& box, const Box3& fade, uint32_t iData);
    int32_t   INextNode(const Box3& box, const Box3& fade, uint32_t iData);

    int32_t   IGetChild(const Box3& parent, const Box3& child) const;

    inline BOOL IBoxesClear(const Box3& box0, const Box3& box1) const;
    inline BOOL IFadesClear(const Box3& fade0, const Box3& fade1) const;

    BOOL    IBox0ContainsBox1(const Box3& box0, const Box3& box1, const Box3& fade0, const Box3& fade1) const;

    BOOL    IBoxClearRecur(int32_t iNode, const Box3& box, const Box3& fade) const;
    BOOL    IPointClearRecur(int32_t iNode, const Point3& pt, const Box3& fade) const;

    void    IHarvestBoxRecur(int32_t iNode, const Box3& box, Tab<int32_t>& out) const;

public:
    plDistTree();
    virtual ~plDistTree();

    void Reset();

    void AddBoxPData(const Box3& box, const Box3& fade, void* pData=nullptr) { AddBoxIData(box, fade, uint32_t(pData)); }
    void AddBoxIData(const Box3& box, const Box3& fade, uint32_t iData=0);
    void AddBox(const Box3& box, const Box3& fade=NonFade()) { AddBoxIData(box, fade, 0); }

    BOOL BoxClear(const Box3& box, const Box3& fade) const;
    BOOL PointClear(const Point3& pt, const Box3& fade) const;

    BOOL IsEmpty() const { return fRoot < 0; }

    static Box3 NonFade() { return Box3(Point3(0,0,0), Point3(0,0,0)); }

    void HarvestBox(const Box3& box, Tab<int32_t>& out) const;

    const plDistNode& GetBox(int32_t i) const { return fNodes[i]; }
};

#endif // plDistTree_inc
