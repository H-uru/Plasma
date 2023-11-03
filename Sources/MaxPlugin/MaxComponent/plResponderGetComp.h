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

#ifndef plResponderGetComp_inc
#define plResponderGetComp_inc

#include <set>
#include <vector>

class Class_ID;
class plComponentBase;
class plMaxNodeBase;
class IParamBlock2;

class plResponderGetComp
{
public:
    typedef std::vector<Class_ID> ClassIDs;

    static plResponderGetComp& Instance();

    // Get a comp and node it is applied to from the user and store it in the PB
    //
    // pb - Your param block
    // nodeID - ParamID of a reftarg for the node to be put in
    // compID - ParamID of a reftarg for the comp to be put in
    // classIDs - Optional list of component Class_ID's to allow (nil for all)
    bool GetComp(IParamBlock2 *pb, int nodeID, int compID, ClassIDs *classIDs);

    // Get the comp stored in your PB
    // Set convertTime to true to verify that the node can convert (ie, only use it after PreConvert)
    plComponentBase *GetSavedComp(IParamBlock2 *pb, int nodeID, int compID, bool convertTime=false);

protected:
    IParamBlock2 *fPB;
    int fNodeID;
    int fCompID;
    ClassIDs *fClassIDs;

    typedef std::set<plMaxNodeBase*> NodeSet;

#if 0
    ULONG fLastAnimComp;
    ULONG fLastAnimObj;
    ULONG fLastSoundComp;
    ULONG fLastSoundObj;
#endif

    void IFindCompsRecur(plMaxNodeBase *node, NodeSet& nodes);
    void ILoadNodes(plMaxNodeBase *compNode, HWND hDlg);

    static INT_PTR CALLBACK ForwardDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
    INT_PTR DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
};

class plResponderCompNode
{
public:
    typedef std::vector<Class_ID> ClassIDs;

protected:
    IParamBlock2 *fPB;
    int fNodeID;
    int fCompID;
    int fCompResID;
    int fNodeResID;
    ClassIDs fCompCIDs;

//  typedef std::set<plMaxNodeBase*> NodeSet;
    void IUpdateCompButton(HWND hWnd);
    void IUpdateNodeButton(HWND hWnd);

    bool IValidate();

public:

    // Get a comp and node it is applied to from the user and store it in the PB
    //
    // pb - Your param block
    // nodeID - ParamID of a reftarg for the node to be put in
    // compID - ParamID of a reftarg for the comp to be put in
    // classIDs - Optional list of component Class_ID's to allow (nil for all)
    void Init(IParamBlock2 *pb, int compID, int nodeID, int compResID, int nodeResID, ClassIDs *compCIDs);
    void InitDlg(HWND hWnd);
    
    void CompButtonPress(HWND hWnd);
    void NodeButtonPress(HWND hWnd);

    bool GetCompAndNode(plComponentBase*& comp, plMaxNodeBase*& node);
};

#endif
