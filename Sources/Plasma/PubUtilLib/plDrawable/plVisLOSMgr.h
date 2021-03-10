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

#ifndef plVisLOSMgr_inc
#define plVisLOSMgr_inc

#include <vector>

#include "hsGeometry3.h"

class plSpaceTreeNode;
class plSpaceTree;
class plDrawableSpans;
class plDrawable;
class plSceneNode;
class plPageTreeMgr;
class plPipeline;
class hsBounds3Ext;

struct plVisHit
{
    hsPoint3 fPos;
};

struct plSpaceHit
{
    plSpaceHit(int idx, float closest) : fIdx(idx), fClosest(closest) { }

    int     fIdx;
    float   fClosest;
};

class plVisLOSMgr
{
protected:
    plPageTreeMgr*  fPageMgr;
    plPipeline*     fPipe;

    float        fMaxDist;

    hsPoint3        fCurrFrom;
    hsPoint3        fCurrTarg;

    bool ISetup(const hsPoint3& pStart, const hsPoint3& pEnd);
    bool ICheckBound(const hsBounds3Ext& bnd, float& closest);
    bool ICheckSpaceTreeRecur(plSpaceTree* space, int which, std::vector<plSpaceHit>& hits);
    bool ICheckSpaceTree(plSpaceTree* space, std::vector<plSpaceHit>& hits);
    bool ICheckSceneNode(plSceneNode* node, plVisHit& hit);
    bool ICheckDrawable(plDrawable* d, plVisHit& hit);
    bool ICheckSpan(plDrawableSpans* dr, uint32_t spanIdx, plVisHit& hit);
    
public:
    bool Check(const hsPoint3& pStart, const hsPoint3& pEnd, plVisHit& hit);
    bool CursorCheck(plVisHit& hit);

    static plVisLOSMgr* Instance();

    static void Init(plPipeline* pipe, plPageTreeMgr* mgr) { Instance()->fPipe = pipe; Instance()->fPageMgr = mgr; }
    static void DeInit() { Instance()->fPipe = nullptr; Instance()->fPageMgr = nullptr; }
};

#endif // plVisLOSMgr_inc
