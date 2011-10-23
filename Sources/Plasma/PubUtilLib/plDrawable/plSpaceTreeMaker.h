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

#ifndef plSpaceTreeMaker_inc
#define plSpaceTreeMaker_inc

#include "hsBounds.h"
#include "hsTemplates.h"
#include "hsBitVector.h"

class hsRadixSortElem;
class plSpaceTree;
 
class plSpacePrepNode
{
public:
    hsBounds3Ext        fWorldBounds;

    Int16               fIndex;
    Int16               fDataIndex;

    plSpacePrepNode*    fChildren[2];
};

class plSpaceTreeMaker
{
protected:
    hsTArray<plSpacePrepNode*>      fLeaves; // input

    hsRadixSortElem*                fSortScratch;

    hsBitVector                     fDisabled;

    plSpacePrepNode*                fPrepTree;
    Int16                           fTreeSize;

    plSpacePrepNode*                INewSubRoot(const hsBounds3Ext& bnd);
    void                            IFindBigList(hsTArray<plSpacePrepNode*>& nodes, hsScalar length, const hsVector3& axis, hsTArray<plSpacePrepNode*>& giants, hsTArray<plSpacePrepNode*>& strimp);
    void                            ISortList(hsTArray<plSpacePrepNode*>& nodes, const hsVector3& axis);
    void                            ISplitList(hsTArray<plSpacePrepNode*>& nodes, const hsVector3& axis, hsTArray<plSpacePrepNode*>& lower, hsTArray<plSpacePrepNode*>& upper);
    hsBounds3Ext                    IFindDistToCenterAxis(hsTArray<plSpacePrepNode*>& nodes, hsScalar& length, hsVector3& axis);
    plSpacePrepNode*                IMakeFatTreeRecur(hsTArray<plSpacePrepNode*>& nodes);
    hsBounds3Ext                    IFindSplitAxis(hsTArray<plSpacePrepNode*>& nodes, hsScalar& length, hsVector3& axis);
    plSpacePrepNode*                IMakeTreeRecur(hsTArray<plSpacePrepNode*>& nodes);

    void                            IMakeTree();

    plSpaceTree*                    IMakeEmptyTree();
    plSpaceTree*                    IMakeDegenerateTree();
    void                            IGatherLeavesRecur(plSpacePrepNode* sub, plSpaceTree* tree);
    void                            IMakeSpaceTreeRecur(plSpacePrepNode* sub, plSpaceTree* tree, const int targetLevel, int currLevel);
    plSpaceTree*                    IMakeSpaceTree();
    int                             ITreeDepth(plSpacePrepNode* subRoot);
    
    void                            IDeleteTreeRecur(plSpacePrepNode* node);

public:

    void                            Cleanup();

    void                            Reset();
    Int32                           AddLeaf(const hsBounds3Ext& worldBnd, hsBool disable=false);
    plSpaceTree*                    MakeTree();

    void                            TestTree(); // development only - NUKE ME mf horse
};

#endif // plSpaceTreeMaker_inc
