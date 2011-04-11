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
#include "HeadSpin.h"
#include "plAnimCompProc.h"

#include "../MaxMain/plMaxNode.h"
#include "plComponentBase.h"

#include "plPickNode.h"
#include "plAnimComponent.h"
#include "../../PubUtilLib/plInterp/plAnimEaseTypes.h"

plAnimCompProc::plAnimCompProc() :
	fCompButtonID(0),
	fCompParamID(0),
	fNodeButtonID(0),
	fNodeParamID(0)
{
}

BOOL plAnimCompProc::DlgProc(TimeValue t, IParamMap2* pm, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
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
		SetWindowText(hButton, "(none)");
		EnableWindow(hButton, FALSE);
		return;
	}

	// If this is an anim grouped component you can't pick a target
	if (comp->ClassID() == ANIM_GROUP_COMP_CID)
	{
		IClearNode(pb);
		SetWindowText(hButton, "(none)");
		EnableWindow(hButton, FALSE);
		return;
	}

	EnableWindow(hButton, TRUE);

	// Make sure the node is actually in the components target list
	plMaxNode* node = IGetNode(pb);
	if (comp->IsTarget((plMaxNodeBase*)node))
		SetWindowText(hButton, node->GetName());
	else
		SetWindowText(hButton, "(none)");
}

void plAnimCompProc::IUpdateCompButton(HWND hWnd, IParamBlock2* pb)
{
	HWND hAnim = GetDlgItem(hWnd, fCompButtonID);

	plComponentBase* comp = IGetComp(pb);
	if (comp)
		SetWindowText(hAnim, comp->GetINode()->GetName());
	else
		SetWindowText(hAnim, "(none)");
}

plComponentBase* plAnimCompProc::IGetComp(IParamBlock2* pb)
{
	plMaxNode* node = nil;
	if (pb->GetParameterType(fCompParamID) == TYPE_REFTARG)
		node = (plMaxNode*)pb->GetReferenceTarget(fCompParamID);
	else
		node = (plMaxNode*)pb->GetINode(fCompParamID);

	if (node)
		return node->ConvertToComponent();

	return nil;
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
		pb->SetValue(fNodeParamID, 0, (ReferenceTarget*)nil);
	else
		pb->SetValue(fNodeParamID, 0, (INode*)nil);
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

BOOL plMtlAnimProc::DlgProc(TimeValue t, IParamMap2* pm, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
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
		SetWindowText(hMtl, "(none)");

	// Enable the node button if a material is selected
	EnableWindow(GetDlgItem(hWnd, fNodeButtonID), (savedMtl != nil));

	// Update the dependencies of this
	IUpdateNodeButton(hWnd, pb);
}

void plMtlAnimProc::IUpdateNodeButton(HWND hWnd, IParamBlock2* pb)
{
	ISetNodeButtonText(hWnd, pb);

	// Update the dependencies of this
	ILoadAnimCombo(hWnd, pb);
}

#include "plNotetrackAnim.h"

void plMtlAnimProc::ILoadAnimCombo(HWND hWnd, IParamBlock2* pb)
{
	HWND hAnim = GetDlgItem(hWnd, fAnimComboID);

	ComboBox_ResetContent(hAnim);
	int sel = ComboBox_AddString(hAnim, ENTIRE_ANIMATION_NAME);
	ComboBox_SetCurSel(hAnim, sel);
	
	const char* savedName = pb->GetStr(fAnimParamID);
	if (!savedName)
		savedName = "";

	Mtl* mtl = IGetMtl(pb);
	if (mtl)
	{
		plNotetrackAnim anim(mtl, nil);
		while (const char* animName = anim.GetNextAnimName())
		{
			int idx = ComboBox_AddString(hAnim, animName);
			ComboBox_SetItemData(hAnim, idx, 1);
			if (!strcmp(animName, savedName))
				ComboBox_SetCurSel(hAnim, idx);
		}

		EnableWindow(hAnim, TRUE);
	}
	else
		EnableWindow(hAnim, FALSE);

	// Update the dependencies of this
	ILoadUser(hWnd, pb);
}

#include "plPickMaterialMap.h"
#include "../MaxMain/plMtlCollector.h"

void plMtlAnimProc::IMtlButtonPress(HWND hWnd, IParamBlock2* pb)
{
	// Let the user pick a new material
	Mtl* pickedMtl = plPickMaterialMap::PickMaterial(plMtlCollector::kUsedOnly |
													plMtlCollector::kPlasmaOnly);

	// Save the mtl in the pb and update the interface
	if (pickedMtl != nil)
	{
		if (pb->GetParameterType(fMtlParamID) == TYPE_REFTARG)
			pb->SetValue(fMtlParamID, 0, (ReferenceTarget*)pickedMtl);
		else
			pb->SetValue(fMtlParamID, 0, pickedMtl);
	}


	// Make sure the current node has the selected material on it (clear it otherwise)
	INode* node = pb->GetINode(fNodeParamID);
	if (!pickedMtl || !node || node->GetMtl() != pickedMtl)
		pb->SetValue(fNodeParamID, 0, (INode*)nil);

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
			pb->SetValue(fAnimParamID, 0, "");
		else
		{
			// Get the name of the animation and save it
			char buf[256];
			ComboBox_GetText(hCombo, buf, sizeof(buf));
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


#include "plPickNodeBase.h"

static const char* kUserTypeAll = "(All)";

class plPickAllMtlNode : public plPickMtlNode
{
protected:
	void IAddUserType(HWND hList)
	{
		int idx = ListBox_AddString(hList, kUserTypeAll);
		if (!fPB->GetINode(fNodeParamID))
			ListBox_SetCurSel(hList, idx);
	}

	void ISetUserType(plMaxNode* node, const char* userType)
	{
		if (hsStrEQ(userType, kUserTypeAll))
			ISetNodeValue(nil);
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

