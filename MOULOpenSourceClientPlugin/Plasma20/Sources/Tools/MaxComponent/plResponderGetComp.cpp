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
#include "plResponderGetComp.h"

#include <algorithm>

#include "../MaxMain/plMaxNodeBase.h"
#include "resource.h"
#include "plComponentBase.h"
#include "../MaxMain/plMaxAccelerators.h"

plResponderGetComp& plResponderGetComp::Instance()
{
	static plResponderGetComp theInstance;
	return theInstance;
}

bool plResponderGetComp::GetComp(IParamBlock2 *pb, int nodeID, int compID, ClassIDs *classIDs)
{
	fPB = pb;
	fNodeID = nodeID;
	fCompID = compID;
	fClassIDs = classIDs;

	plMaxAccelerators::Disable();

	int ret = DialogBox(hInstance,
						MAKEINTRESOURCE(IDD_COMP_RESPOND_ANIMPICK),
						GetCOREInterface()->GetMAXHWnd(),
						ForwardDlgProc);

	plMaxAccelerators::Enable();

	return (ret != 0);
}

plComponentBase *plResponderGetComp::GetSavedComp(IParamBlock2 *pb, int nodeID, int compID, bool convertTime)
{
	plMaxNodeBase *node = (plMaxNodeBase*)pb->GetReferenceTarget(nodeID);
	// This value could be whack, only use if node is valid
	plMaxNodeBase *comp = (plMaxNodeBase*)pb->GetReferenceTarget(compID);

	if (!node || (convertTime && !node->CanConvert()) || !comp)
		return nil;

	int numComps = node->NumAttachedComponents();
	for (int i = 0; i < numComps; i++)
	{
		plComponentBase *thisComp = node->GetAttachedComponent(i);
		if (thisComp->GetINode() == comp)
			return thisComp;
	}

	return nil;
}
void plResponderGetComp::IFindCompsRecur(plMaxNodeBase *node, NodeSet& nodes)
{
	plComponentBase *comp = node->ConvertToComponent();
	if (comp)
	{
		// If we're not filtering, or we are and this component is in our accepted list, add it
		if (!fClassIDs ||
			std::find(fClassIDs->begin(), fClassIDs->end(), comp->ClassID()) != fClassIDs->end())
		{
			nodes.insert(node);
		}
	}

	for (int i = 0; i < node->NumberOfChildren(); i++)
		IFindCompsRecur((plMaxNodeBase*)node->GetChildNode(i), nodes);
}

void plResponderGetComp::ILoadNodes(plMaxNodeBase *compNode, HWND hDlg)
{
	HWND hNodes = GetDlgItem(hDlg, IDC_OBJ_LIST);
	ListBox_ResetContent(hNodes);

	plComponentBase *comp = compNode ? compNode->ConvertToComponent() : nil;
	if (!comp)
		return;

	plMaxNodeBase *savedNode = (plMaxNodeBase*)fPB->GetReferenceTarget(fNodeID);

	for (int i = 0; i < comp->NumTargets(); i++)
	{
		plMaxNodeBase *node = comp->GetTarget(i);
		if (node)
		{
			int idx = ListBox_AddString(hNodes, node->GetName());
			ListBox_SetItemData(hNodes, idx, node);

			if (savedNode == node)
				ListBox_SetCurSel(hNodes, idx);
		}
	}
}

BOOL CALLBACK plResponderGetComp::ForwardDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return Instance().DlgProc(hDlg, msg, wParam, lParam);
}

BOOL plResponderGetComp::DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
		{
			NodeSet nodes;
			IFindCompsRecur((plMaxNodeBase*)GetCOREInterface()->GetRootNode(), nodes);

			HWND hComps = GetDlgItem(hDlg, IDC_COMP_LIST);

			plMaxNodeBase *node = (plMaxNodeBase*)fPB->GetReferenceTarget(fCompID);

			for (NodeSet::iterator it = nodes.begin(); it != nodes.end(); it++)
			{
				int idx = ListBox_AddString(hComps, (*it)->GetName());
				ListBox_SetItemData(hComps, idx, *it);

				if (*it == node)
					ListBox_SetCurSel(hComps, idx);
			}

			ILoadNodes(node, hDlg);
		}
		return TRUE;

	case WM_COMMAND:
		if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, 0);
			return TRUE;
		}
		else if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDOK)
		{
			// Get the selected node
			HWND hNode = GetDlgItem(hDlg, IDC_OBJ_LIST);
			int idx = ListBox_GetCurSel(hNode);
			plMaxNodeBase *node = nil;
			if (idx != LB_ERR)
				node = (plMaxNodeBase*)ListBox_GetItemData(hNode, idx);

			// Get the selected component
			HWND hComp = GetDlgItem(hDlg, IDC_COMP_LIST);
			idx = ListBox_GetCurSel(hComp);
			plMaxNodeBase *comp = nil;
			if (idx != LB_ERR)
				comp = (plMaxNodeBase*)ListBox_GetItemData(hComp, idx);

			// If both were selected, save them in the PB and as the last setting
			if (node && comp)
			{
#if 0
				if (fType == kFindAnim)
				{
					fLastAnimObj = obj->GetHandle();
					fLastAnimComp = comp->GetHandle();
				}
				else if (fType == kFindSound)
				{
					fLastSoundObj = obj->GetHandle();
					fLastSoundComp = comp->GetHandle();
				}
#endif
				fPB->SetValue(fNodeID, 0, (ReferenceTarget*)node);
				fPB->SetValue(fCompID, 0, (ReferenceTarget*)comp);
				
				EndDialog(hDlg, 1);
			}
			else
				EndDialog(hDlg, 0);

			return TRUE;
		}
		else if (HIWORD(wParam) == LBN_SELCHANGE)
		{
			if (LOWORD(wParam) == IDC_COMP_LIST)
			{
				int idx = ListBox_GetCurSel((HWND)lParam);
				plMaxNodeBase *node = (plMaxNodeBase*)ListBox_GetItemData((HWND)lParam, idx);
				ILoadNodes(node, hDlg);
			}
		}
		break;
	}

	return FALSE;
}

///////////////////////////////////////////////////////////////////////////

#include "plPickNode.h"

void plResponderCompNode::Init(IParamBlock2 *pb, int compID, int nodeID, int compResID, int nodeResID, ClassIDs *compCIDs)
{
	fPB = pb;
	fCompID = compID;
	fNodeID = nodeID;
	fCompResID = compResID;
	fNodeResID = nodeResID;

	if (compCIDs)
		fCompCIDs = *compCIDs;
}

void plResponderCompNode::InitDlg(HWND hWnd)
{
	IValidate();

	IUpdateNodeButton(hWnd);
	IUpdateCompButton(hWnd);
}

bool plResponderCompNode::IValidate()
{
	plMaxNodeBase *savedComp = (plMaxNodeBase*)fPB->GetReferenceTarget(fCompID);
	plComponentBase *comp = savedComp ? savedComp->ConvertToComponent() : nil;
	plMaxNodeBase *node = (plMaxNodeBase*)fPB->GetReferenceTarget(fNodeID);
	if (comp && node)
	{
		// Make sure the selected comp has the correct CID
		if (fCompCIDs.size() > 0)
		{
			Class_ID compCID = comp->ClassID();
			bool foundCID = false;
			for (int i = 0; i < fCompCIDs.size(); i++)
			{
				if (fCompCIDs[i] == compCID)
					foundCID = true;
			}

			if (!foundCID)
			{
				fPB->SetValue(fCompID, 0, (INode*)nil);
				fPB->SetValue(fNodeID, 0, (INode*)nil);
				return false;
			}
		}

		// Make sure the comp is really attached to the node
		if (comp->IsTarget(node))
			return true;
		else
			fPB->SetValue(fNodeID, 0, (INode*)nil);
	}

	return false;
}

void plResponderCompNode::CompButtonPress(HWND hWnd)
{
	plPick::Node(fPB, fCompID, &fCompCIDs, true, false);

	IUpdateCompButton(hWnd);
	IUpdateNodeButton(hWnd);
}

void plResponderCompNode::NodeButtonPress(HWND hWnd)
{
	plMaxNodeBase *node = (plMaxNodeBase*)fPB->GetReferenceTarget(fCompID);
	plComponentBase *comp = node ? node->ConvertToComponent() : nil;
	if (comp)
		plPick::CompTargets(fPB, fNodeID, comp);

	IUpdateNodeButton(hWnd);
}

void plResponderCompNode::IUpdateCompButton(HWND hWnd)
{
	HWND hComp = GetDlgItem(hWnd, fCompResID);

	plMaxNodeBase *savedComp = (plMaxNodeBase*)fPB->GetReferenceTarget(fCompID);

	if (savedComp)
		SetWindowText(hComp, savedComp->GetName());
	else
		SetWindowText(hComp, "(none)");
}

void plResponderCompNode::IUpdateNodeButton(HWND hWnd)
{
	HWND hNode = GetDlgItem(hWnd, fNodeResID);

	// If there is no component, disable the node button
	plMaxNodeBase *node = (plMaxNodeBase*)fPB->GetReferenceTarget(fCompID);
	plComponentBase *comp = node ? node->ConvertToComponent() : nil;
	if (!comp)
	{
		EnableWindow(hNode, FALSE);
		SetWindowText(hNode, "(none)");
		return;
	}
	EnableWindow(hNode, TRUE);

	plMaxNodeBase *objNode = (plMaxNodeBase*)fPB->GetReferenceTarget(fNodeID);
	if (objNode && comp->IsTarget(objNode))
		SetWindowText(hNode, objNode->GetName());
	else
		SetWindowText(hNode, "(none)");
}

bool plResponderCompNode::GetCompAndNode(plComponentBase*& comp, plMaxNodeBase*& node)
{
	if (!IValidate())
	{
		comp = nil;
		node = nil;
		return false;
	}

	plMaxNodeBase *savedComp = (plMaxNodeBase*)fPB->GetReferenceTarget(fCompID);
	comp = savedComp ? savedComp->ConvertToComponent() : nil;
	node = (plMaxNodeBase*)fPB->GetReferenceTarget(fNodeID);

	return true;
}
