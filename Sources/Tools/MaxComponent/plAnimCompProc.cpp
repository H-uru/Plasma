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

#include "plAnimComponent.h"
#include "plComponentBase.h"
#include "MaxMain/plMaxNode.h"

#include "MaxMain/MaxAPI.h"

#include <set>

#include "plAnimCompProc.h"

#include "plPickNode.h"
#include "plPickNodeBase.h"
#include "plNotetrackAnim.h"
#include "plInterp/plAnimEaseTypes.h"

#include "plPickMaterialMap.h"
#include "MaxMain/plMtlCollector.h"

plAnimCompProc::plAnimCompProc() :
    fCompButtonID(0),
    fCompParamID(0),
    fNodeButtonID(0),
    fNodeParamID(0)
{
}

INT_PTR plAnimCompProc::DlgProc(TimeValue t, IParamMap2* pm, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_INITDIALOG:
        {
            IParamBlock2* pb = pm->GetParamBlock();

            IUpdateNodeButton(hWnd, pb);
            IUpdateCompButton(hWnd, pb);

            ILoadUser(hWnd, pb);
        }
        return TRUE;

    case WM_COMMAND:
        {
            int cmd = HIWORD(wParam);
            int resID = LOWORD(wParam);

            if (cmd == BN_CLICKED && resID == fCompButtonID)
            {
                ICompButtonPress(hWnd, pm->GetParamBlock());
                return TRUE;
            }
            else if (cmd == BN_CLICKED && resID == fNodeButtonID)
            {
                INodeButtonPress(hWnd, pm->GetParamBlock());
                return TRUE;
            }
            else if (IUserCommand(hWnd, pm->GetParamBlock(), cmd, resID))
                return TRUE;

        }
        break;
    }
    return FALSE;
}

void plAnimCompProc::ICompButtonPress(HWND hWnd, IParamBlock2* pb)
{
    IPickComponent(pb);
    
    IUpdateCompButton(hWnd, pb);
    IUpdateNodeButton(hWnd, pb);

    ILoadUser(hWnd, pb);
}

void plAnimCompProc::IPickNode(IParamBlock2* pb, plComponentBase* comp)
{
    plPick::CompTargets(pb, fNodeParamID, comp);
}

void plAnimCompProc::INodeButtonPress(HWND hWnd, IParamBlock2* pb)
{
    plComponentBase* comp = IGetComp(pb);
    if (comp)
        IPickNode(pb, comp);

    IUpdateNodeButton(hWnd, pb);
    ILoadUser(hWnd, pb);
}

void plAnimCompProc::IUpdateNodeButton(HWND hWnd, IParamBlock2* pb)
{
    HWND hButton = GetDlgItem(hWnd, fNodeButtonID);

    plComponentBase* comp = IGetComp(pb);
    if (!comp)
    {
        SetWindowText(hButton, _T("(none)"));
        EnableWindow(hButton, FALSE);
        return;
    }

    // If this is an anim grouped component you can't pick a target
    if (comp->ClassID() == ANIM_GROUP_COMP_CID)
    {
        IClearNode(pb);
        SetWindowText(hButton, _T("(none)"));
        EnableWindow(hButton, FALSE);
        return;
    }

    EnableWindow(hButton, TRUE);

    // Make sure the node is actually in the components target list
    plMaxNode* node = IGetNode(pb);
    if (comp->IsTarget((plMaxNodeBase*)node))
        SetWindowText(hButton, node->GetName());
    else
        SetWindowText(hButton, _T("(none)"));
}

void plAnimCompProc::IUpdateCompButton(HWND hWnd, IParamBlock2* pb)
{
    HWND hAnim = GetDlgItem(hWnd, fCompButtonID);

    plComponentBase* comp = IGetComp(pb);
    if (comp)
        SetWindowText(hAnim, comp->GetINode()->GetName());
    else
        SetWindowText(hAnim, _T("(none)"));
}

plComponentBase* plAnimCompProc::IGetComp(IParamBlock2* pb)
{
    plMaxNode* node = nullptr;
    if (pb->GetParameterType(fCompParamID) == TYPE_REFTARG)
        node = (plMaxNode*)pb->GetReferenceTarget(fCompParamID);
    else
        node = (plMaxNode*)pb->GetINode(fCompParamID);

    if (node)
        return node->ConvertToComponent();

    return nullptr;
}

plMaxNode* plAnimCompProc::IGetNode(IParamBlock2* pb)
{
    if (pb->GetParameterType(fNodeParamID) == TYPE_REFTARG)
        return (plMaxNode*)pb->GetReferenceTarget(fNodeParamID);
    else
        return (plMaxNode*)pb->GetINode(fNodeParamID);
}

void plAnimCompProc::IClearNode(IParamBlock2* pb)
{
    if (pb->GetParameterType(fNodeParamID) == TYPE_REFTARG)
        pb->SetValue(fNodeParamID, 0, (ReferenceTarget*)nullptr);
    else
        pb->SetValue(fNodeParamID, 0, (INode*)nullptr);
}

bool plAnimCompProc::GetCompAndNode(IParamBlock2* pb, plComponentBase*& comp, plMaxNode*& node)
{
    comp = IGetComp(pb);
    if (comp)
    {
        node = IGetNode(pb);

        // If it's an anim group component (don't need a node), or we have a node
        // and the component is attached to it, we're ok.
        if (comp->ClassID() == ANIM_GROUP_COMP_CID ||
            (node && comp->IsTarget((plMaxNodeBase*)node)))
            return true;
    }

    return false;
}


///////////////////////////////////////////////////////////////////////////////////////////

plMtlAnimProc::plMtlAnimProc() :
    fMtlButtonID(0),
    fMtlParamID(0),
    fNodeButtonID(0),
    fNodeParamID(0),
    fAnimComboID(0),
    fAnimParamID(0)
{
}

INT_PTR plMtlAnimProc::DlgProc(TimeValue t, IParamMap2* pm, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_INITDIALOG:
        {
            IParamBlock2* pb = pm->GetParamBlock();

            IOnInitDlg(hWnd, pb);

            IUpdateMtlButton(hWnd, pb);
        }
        return TRUE;

    case WM_COMMAND:
        {
            int cmd = HIWORD(wParam);
            int resID = LOWORD(wParam);

            IParamBlock2* pb = pm->GetParamBlock();

            if (cmd == BN_CLICKED && resID == fMtlButtonID)
            {
                IMtlButtonPress(hWnd, pb);
                return TRUE;
            }
            else if (cmd == BN_CLICKED && resID == fNodeButtonID)
            {
                INodeButtonPress(hWnd, pb);
                return TRUE;
            }
            else if (cmd == CBN_SELCHANGE && resID == fAnimComboID)
            {
                IAnimComboChanged(hWnd, pb);
                return TRUE;
            }
            else if (IUserCommand(hWnd, pb, cmd, resID))
                return TRUE;
        }
        break;
    }

    return FALSE;
}

void plMtlAnimProc::IUpdateMtlButton(HWND hWnd, IParamBlock2* pb)
{
    HWND hMtl = GetDlgItem(hWnd, fMtlButtonID);

    // Get the saved material
    Mtl *savedMtl = IGetMtl(pb);

    if (savedMtl)
        SetWindowText(hMtl, savedMtl->GetName());
    else
        SetWindowText(hMtl, _T("(none)"));

    // Enable the node button if a material is selected
    EnableWindow(GetDlgItem(hWnd, fNodeButtonID), (savedMtl != nullptr));

    // Update the dependencies of this
    IUpdateNodeButton(hWnd, pb);
}

void plMtlAnimProc::IUpdateNodeButton(HWND hWnd, IParamBlock2* pb)
{
    ISetNodeButtonText(hWnd, pb);

    // Update the dependencies of this
    ILoadAnimCombo(hWnd, pb);
}

void plMtlAnimProc::ILoadAnimCombo(HWND hWnd, IParamBlock2* pb)
{
    HWND hAnim = GetDlgItem(hWnd, fAnimComboID);

    ComboBox_ResetContent(hAnim);
    int sel = ComboBox_AddString(hAnim, _T(ENTIRE_ANIMATION_NAME));
    ComboBox_SetCurSel(hAnim, sel);
    
    auto savedName = pb->GetStr(fAnimParamID);
    if (!savedName)
        savedName = _T("");

    Mtl* mtl = IGetMtl(pb);
    if (mtl)
    {
        plNotetrackAnim anim(mtl, nullptr);
        ST::string animName;
        while (!(animName = anim.GetNextAnimName()).empty())
        {
            int idx = ComboBox_AddString(hAnim, ST2T(animName));
            ComboBox_SetItemData(hAnim, idx, 1);
            if (!animName.compare(savedName))
                ComboBox_SetCurSel(hAnim, idx);
        }

        EnableWindow(hAnim, TRUE);
    }
    else
        EnableWindow(hAnim, FALSE);

    // Update the dependencies of this
    ILoadUser(hWnd, pb);
}

void plMtlAnimProc::IMtlButtonPress(HWND hWnd, IParamBlock2* pb)
{
    // Let the user pick a new material
    Mtl* pickedMtl = plPickMaterialMap::PickMaterial(plMtlCollector::kUsedOnly |
                                                    plMtlCollector::kPlasmaOnly);

    // Save the mtl in the pb and update the interface
    if (pickedMtl != nullptr)
    {
        if (pb->GetParameterType(fMtlParamID) == TYPE_REFTARG)
            pb->SetValue(fMtlParamID, 0, (ReferenceTarget*)pickedMtl);
        else
            pb->SetValue(fMtlParamID, 0, pickedMtl);
    }


    // Make sure the current node has the selected material on it (clear it otherwise)
    INode* node = pb->GetINode(fNodeParamID);
    if (!pickedMtl || !node || node->GetMtl() != pickedMtl)
        pb->SetValue(fNodeParamID, 0, (INode*)nullptr);

    IUpdateMtlButton(hWnd, pb);
}

void plMtlAnimProc::INodeButtonPress(HWND hWnd, IParamBlock2* pb)
{
    IPickNode(pb);

    IUpdateNodeButton(hWnd, pb);
}

void plMtlAnimProc::IAnimComboChanged(HWND hWnd, IParamBlock2* pb)
{
    HWND hCombo = GetDlgItem(hWnd, fAnimComboID);
    int idx = ComboBox_GetCurSel(hCombo);

    if (idx != CB_ERR)
    {
        if (ComboBox_GetItemData(hCombo, idx) == 0)
            pb->SetValue(fAnimParamID, 0, _M(""));
        else
        {
            // Get the name of the animation and save it
            TCHAR buf[256];
            ComboBox_GetText(hCombo, buf, std::size(buf));
            pb->SetValue(fAnimParamID, 0, buf);
        }
    }

    // Update the dependencies of this
    ILoadUser(hWnd, pb);
}

Mtl* plMtlAnimProc::IGetMtl(IParamBlock2* pb)
{
    if (pb->GetParameterType(fMtlParamID) == TYPE_REFTARG)
        return (Mtl*)pb->GetReferenceTarget(fMtlParamID);
    else
        return pb->GetMtl(fMtlParamID);
}

static const TCHAR* kUserTypeAll = _T("(All)");

class plPickAllMtlNode : public plPickMtlNode
{
protected:
    void IAddUserType(HWND hList) override
    {
        int idx = ListBox_AddString(hList, kUserTypeAll);
        if (!fPB->GetINode(fNodeParamID))
            ListBox_SetCurSel(hList, idx);
    }

    void ISetUserType(plMaxNode* node, const TCHAR* userType) override
    {
        if (userType && _tcscmp(userType, kUserTypeAll) == 0)
            ISetNodeValue(nullptr);
    }

public:
    plPickAllMtlNode(IParamBlock2* pb, int nodeParamID, Mtl* mtl) :
      plPickMtlNode(pb, nodeParamID, mtl)
    {
    }
};

void plMtlAnimProc::IPickNode(IParamBlock2* pb)
{
    plPickAllMtlNode pick(pb, fNodeParamID, IGetMtl(pb));
    pick.DoPick();
}

void plMtlAnimProc::ISetNodeButtonText(HWND hWnd, IParamBlock2* pb)
{
    HWND hNode = GetDlgItem(hWnd, fNodeButtonID);

    INode* node = pb->GetINode(fNodeParamID);
    if (node)
        SetWindowText(hNode, node->GetName());
    else
        SetWindowText(hNode, kUserTypeAll);
}

