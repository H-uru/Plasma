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
#include "HeadSpin.h"
#include <algorithm>

#include "plComponentBase.h"
#include "MaxMain/plMaxNode.h"
#include "resource.h"

#include "plPickNodeBase.h"
#include "MaxMain/plMaxAccelerators.h"

plPickNodeBase::plPickNodeBase(IParamBlock2* pb, int nodeParamID) : fPB(pb), fNodeParamID(nodeParamID)
{
}


bool plPickNodeBase::DoPick()
{
    plMaxAccelerators::Disable();

    // Create Dlg
    INT_PTR ret = DialogBoxParam(hInstance,
                            MAKEINTRESOURCE(IDD_PICK_NODE),
                            GetCOREInterface()->GetMAXHWnd(),
                            IDlgProc,
                            (LPARAM)this);

    plMaxAccelerators::Enable();

    return (ret != 0);
}

void plPickNodeBase::IInitDlg(HWND hDlg)
{
    HWND hList = GetDlgItem(hDlg, IDC_NODE_LIST);

//  int idx = ListBox_AddString(hList, kUserTypeNone);
//  ListBox_SetCurSel(hList, idx);

    IAddUserType(hList);

//  LONG style = GetWindowLong(hList, GWL_STYLE);
//  SetWindowLong(hList, GWL_STYLE, style | LBS_MULTIPLESEL);

    plMaxNode* curSelNode = nullptr;
    ParamType2 type = fPB->GetParameterType(fNodeParamID);
    if (type == TYPE_REFTARG)
        curSelNode = (plMaxNode*)fPB->GetReferenceTarget(fNodeParamID);
    else if (type == TYPE_INODE)
        curSelNode = (plMaxNode*)fPB->GetINode(fNodeParamID);

    IGetNodesRecur((plMaxNode*)GetCOREInterface()->GetRootNode(), hList, curSelNode);
}

INT_PTR plPickNodeBase::IDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static plPickNodeBase* pthis = nullptr;

    switch (msg)
    {
    case WM_INITDIALOG:
        pthis = (plPickNodeBase*)lParam;
        pthis->IInitDlg(hDlg);
        return TRUE;

    case WM_COMMAND:
        // For 'OK' or double-click, get the selected node and exit
        if ((HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDOK) ||
            (HIWORD(wParam) == LBN_DBLCLK && LOWORD(wParam) == IDC_NODE_LIST))
        {
            pthis->IGetSelNode(GetDlgItem(hDlg, IDC_NODE_LIST));
            EndDialog(hDlg, 1);
            return TRUE;
        }
        else if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, 0);
            return TRUE;
        }
        break;
    }

    return FALSE;
}

void plPickNodeBase::IGetNodesRecur(plMaxNode* node, HWND hList, plMaxNode* curSelNode)
{
    if (node)
    {
        if (ICheckNode(node))
        {
            int idx = ListBox_AddString(hList, node->GetName());
            ListBox_SetItemData(hList, idx, node);

            if (node == curSelNode)
                ListBox_SetCurSel(hList, idx);
        }

        int numChildren = node->NumberOfChildren();
        for (int i = 0; i < numChildren; i++)
        {
            IGetNodesRecur((plMaxNode*)node->GetChildNode(i), hList, curSelNode);
        }
    }
}

void plPickNodeBase::ISetNodeValue(plMaxNode* node)
{
    ParamType2 type = fPB->GetParameterType(fNodeParamID);
    if (type == TYPE_REFTARG)
    {
        fPB->SetValue(fNodeParamID, 0, (ReferenceTarget*)node);
    }
//  else if (type == TYPE_REFTARG_TAB)
//  {
//  }
    else if (type == TYPE_INODE)
    {
        fPB->SetValue(fNodeParamID, 0, (INode*)node);
    }
//  else if (type == TYPE_INODE_TAB)
//  {
//  }
}

void plPickNodeBase::IGetSelNode(HWND hList)
{
    int sel = ListBox_GetCurSel(hList);
    if (sel == LB_ERR)
        return;

    plMaxNode* node = (plMaxNode*)ListBox_GetItemData(hList, sel);

    if (node)
    {
        ISetNodeValue(node);
        ISetUserType(node, nullptr);
    }
    else
    {
        int len = ListBox_GetTextLen(hList, sel);
        TCHAR* buf = new TCHAR[len+1];
        ListBox_GetText(hList, sel, buf);

/*      if (_tcsncmp(buf, std::size(buf), kUserTypeNone) == 0)
        {
            ISetNodeValue(nullptr);
            ISetUserType(nullptr, nullptr);
        }
        else
*/          ISetUserType(nullptr, buf);

        delete [] buf;
    }
}

/////////////////////////////////////////////////////////////////

plPickNode::plPickNode(IParamBlock2* pb, int nodeParamID) :
    plPickNodeBase(pb, nodeParamID)
{
}

bool plPickNode::ICanConvertToType(Object *obj)
{
    for (int i = 0; i < fCIDs.size(); i++)
    {
        if (obj->CanConvertToType(fCIDs[i]))
            return true;
    }

    return false;
}

bool plPickNode::ICheckNode(plMaxNode* node)
{
    if (node->GetObjectRef())
    {
        // Not filtering by ClassID and node is hidden or frozen
        if (fCIDs.size() == 0 && (node->IsHidden() || node->IsFrozen()))
            return false;

        // We aren't filtering by ClassID or we are and we found a match
        if (fCIDs.size() == 0 ||
            (fCanConvertToType && ICanConvertToType(node->GetObjectRef()) ||
            std::find(fCIDs.begin(), fCIDs.end(), node->GetObjectRef()->ClassID()) != fCIDs.end()))
        {
            // Don't allow a ref to ourselves (a cyclical reference)
            if (fPB->GetOwner() == node)
                return false;

            // Approved and not in the list, add it
            return true;
        }
    }

    return false;
}

///////////////////////////////////////////////////////////////

plPickMtlNode::plPickMtlNode(IParamBlock2* pb, int nodeParamID, Mtl* mtl) :
    plPickNodeBase(pb, nodeParamID), fMtl(mtl)
{
}

bool plPickMtlNode::ICheckNode(plMaxNode* node)
{
    // Filtering by nodes with a specific material on them
    if (fMtl && node->GetMtl() == fMtl)
        return true;
    return false;
}

/////////////////////////////////////////////////////

plPickCompNode::plPickCompNode(IParamBlock2* pb, int nodeParamID, plComponentBase* comp) :
    plPickNodeBase(pb, nodeParamID), fComp(comp)
{
}

bool plPickCompNode::ICheckNode(plMaxNode* node)
{
    // Filtering by nodes a component is attached to
    if (fComp && fComp->IsTarget((plMaxNodeBase*)node))
        return true;

    return false;
}
