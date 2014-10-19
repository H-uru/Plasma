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

#ifndef plPageTreeMgr_inc
#define plPageTreeMgr_inc

#include "hsTemplates.h"
#include <vector>

class plSceneNode;
class plSpaceTree;
class plPipeline;
class plCullPoly;
class plOccluder;
class plDrawable;
class plDrawVisList;
class plVolumeIsect;
class plVisMgr;

class plDrawSpanPair
{
public:
    plDrawSpanPair() {}
    plDrawSpanPair(uint16_t d, uint16_t s) : fDrawable(d), fSpan(s) {}
    uint16_t      fDrawable;
    uint16_t      fSpan;
};

class plDrawVisList
{
public:
    plDrawVisList() : fDrawable(nil) {}
    virtual ~plDrawVisList() {}

    plDrawable*         fDrawable;
    hsTArray<int16_t>     fVisList;

    plDrawVisList& operator=(const plDrawVisList& v) { fDrawable = v.fDrawable; fVisList = v.fVisList; return *this; }
};

class plPageTreeMgr
{
protected:
    std::vector<plSceneNode*>      fNodes;

    plSpaceTree*                fSpaceTree;
    plVisMgr*                   fVisMgr;

    static bool                 fDisableVisMgr;

    hsTArray<const plOccluder*> fOccluders;
    hsTArray<const plCullPoly*> fCullPolys;
    hsTArray<const plCullPoly*> fSortedCullPolys;

    void                        ITrashSpaceTree();
    bool                        IBuildSpaceTree();
    bool                        IRefreshTree(plPipeline* pipe);
    void                        ISortCullPolys(plPipeline* pipe);
    bool                        IGetOcclusion(plPipeline* pipe, hsTArray<int16_t>& list);
    bool                        IGetCullPolys(plPipeline* pipe);
    void                        IResetOcclusion(plPipeline* pipe);
    void                        IAddCullPolyList(const hsTArray<plCullPoly>& polyList);

    bool                        ISortByLevel(plPipeline* pipe, hsTArray<plDrawVisList>& drawList, hsTArray<plDrawVisList>& sortedDrawList);
    int                         IPrepForRenderSortingSpans(plPipeline* pipe, hsTArray<plDrawVisList>& drawVis, int& iDrawStart);
    bool                        IRenderSortingSpans(plPipeline* pipe, hsTArray<plDrawVisList*>& drawList, hsTArray<plDrawSpanPair>& pairs);
    int                         IRenderVisList(plPipeline* pipe, hsTArray<plDrawVisList>& visList);

public:
    plPageTreeMgr();
    virtual ~plPageTreeMgr();

    const std::vector<plSceneNode*>& GetNodes() const { return fNodes; }

    void            AddNode(plSceneNode* node);
    void            RemoveNode(plSceneNode* node);
    virtual void    Reset(); // remove all nodes, nuke the space tree
    virtual bool    Empty() const { return fNodes.empty(); }

    virtual int     Render(plPipeline* pipe);

    bool            Harvest(plVolumeIsect* isect, hsTArray<plDrawVisList>& levList);

    void            AddOccluderList(const hsTArray<plOccluder*> occList);

    plSpaceTree*    GetSpaceTree() { if( !fSpaceTree ) IBuildSpaceTree(); return fSpaceTree; }

    void            SetVisMgr(plVisMgr* visMgr) { fVisMgr = visMgr; }
    plVisMgr*       GetVisMgr() const { return fVisMgr; }

    static void     EnableVisMgr(bool on) { fDisableVisMgr = !on; }
    static bool     VisMgrEnabled() { return !fDisableVisMgr; }
};

#endif // plPageTreeMgr_inc
