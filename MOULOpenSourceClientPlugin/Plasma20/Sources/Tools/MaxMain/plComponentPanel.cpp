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
#include "plComponentPanel.h"
#include "resource.h"

#include "plMaxNode.h"
#include "../MaxComponent/plComponent.h"
#include "../MaxComponent/plComponentMgr.h"
#include "plComponentDlg.h"
#include "plMaxAccelerators.h"

extern TCHAR *GetString(int id);

class ComponentUtilClassDesc : public ClassDesc
{
public:
	int 			IsPublic()				{ return TRUE; }
	void*			Create(BOOL loading)	{ return &plComponentUtil::Instance(); }
	const TCHAR*	ClassName()				{ return _T("Component Util"); }
	SClass_ID		SuperClassID()			{ return UTILITY_CLASS_ID; }
	Class_ID 		ClassID()				{ return Class_ID(0xb220659, 0x31015552); }
	const TCHAR* 	Category()				{ return _T(""); }
};

static ComponentUtilClassDesc theComponentUtilCD;
ClassDesc* GetComponentUtilDesc() { return &theComponentUtilCD; }

plComponentUtil::plComponentUtil() : fInterface(nil), fhPanel(nil), fCurComponent(nil), fLastComponent(nil)
{
}

plComponentUtil& plComponentUtil::Instance()
{
	static plComponentUtil theInstance;
	return theInstance;
}

////////////////////////////////////////////////////////////////////////////////
// Proc for the currently selected object dialog
//
BOOL CALLBACK plComponentUtil::ForwardDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return Instance().DlgProc(hDlg, msg, wParam, lParam);
}

BOOL plComponentUtil::DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_COMMAND:
		// Switch to next or previous target
		if (HIWORD(wParam) == BN_CLICKED && (LOWORD(wParam) == IDC_BACK || LOWORD(wParam) == IDC_FORWARD))
		{
			INextTarget(LOWORD(wParam) == IDC_FORWARD);
			return TRUE;
		}
		else if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_REF_BY_BUTTON)
		{
			IShowRefdBy();
			return TRUE;
		}
		break;

	case WM_NOTIFY:
		{
			NMHDR *nmhdr = (NMHDR*)lParam;
			if (nmhdr->idFrom == IDC_COMPLIST)
			{
				switch (nmhdr->code)
				{
				// Stop Max from reading keypresses while the list has focus
				case NM_SETFOCUS:
					plMaxAccelerators::Disable();
					return TRUE;
				case NM_KILLFOCUS:
					plMaxAccelerators::Enable();
					return TRUE;

				case LVN_KEYDOWN:
					{
						NMLVKEYDOWN *kd = (NMLVKEYDOWN*)lParam;
						if (kd->wVKey == VK_DELETE)
							IDeleteListSelection();
					}
					return TRUE;

				// The edit box this creates kills the focus on the listbox,
				// so add an extra disable to ignore it
				case LVN_BEGINLABELEDIT:
					plMaxAccelerators::Disable();
					return TRUE;

				// Finishing changing the name of a component
				case LVN_ENDLABELEDIT:
					{
						NMLVDISPINFO *di = (NMLVDISPINFO*)lParam;
						const char *name = di->item.pszText;

						// If the name was changed...
						if (name && *name != '\0')
						{
							// Update the name of the node
							plComponentBase* comp = IGetListSelection();
							comp->GetINode()->SetName(di->item.pszText);

							// Make sure the column is wide enough
							int width = ListView_GetStringWidth(nmhdr->hwndFrom, di->item.pszText)+10;
							if (width > ListView_GetColumnWidth(nmhdr->hwndFrom, 0))
							{
								ListView_SetColumnWidth(nmhdr->hwndFrom, 0, width);
								InvalidateRect(nmhdr->hwndFrom, NULL, FALSE);
							}

							// Update the name in the tree too
							plComponentDlg::Instance().IUpdateNodeName((plMaxNode*)comp->GetINode());

							// Return true to keep the changes
							SetWindowLong(hDlg, DWL_MSGRESULT, TRUE);
						}
						
						plMaxAccelerators::Enable();
					}
					return TRUE;

				// Selected component has changed.  This notification can come
				// more than necessary, so IAddRollups doesn't change the rollups
				// if the "new" one is the same as the old.
				case LVN_ITEMCHANGED:
					{
						plComponentBase* comp = IGetListSelection();
						IAddRollups(comp);
					}
					return TRUE;
				}
			}
		}
		break;
	}

	return FALSE;
}

void plComponentUtil::IDeleteListSelection()
{
	plComponentBase* comp = IGetListSelection();
	if (comp)
	{
		// Delete each of the selected nodes from this components target list
		int count = fInterface->GetSelNodeCount();
		for (int i = 0; i < count; i++)
		{
			plMaxNode *curNode = (plMaxNode*)fInterface->GetSelNode(i);
			comp->DeleteTarget(curNode);
		}

		IUpdateRollups();
	}
}

plComponentBase* plComponentUtil::IGetListSelection()
{
	HWND hList = GetDlgItem(fhPanel, IDC_COMPLIST);

	int index = ListView_GetNextItem(hList, -1, LVNI_SELECTED);
	if (index != -1)
	{
		LVITEM item;
		item.mask = LVIF_PARAM;
		item.iItem = index;
		item.iSubItem = 0;
		if (ListView_GetItem(hList, &item))
			return (plComponentBase*)item.lParam;
	}

	return nil;
}

void plComponentUtil::BeginEditParams(Interface *ip, IUtil *iu)
{
	fInterface = ip;
	
	fhPanel = fInterface->AddRollupPage(hInstance, MAKEINTRESOURCE(IDD_COMP_PANEL), ForwardDlgProc, "Components (Selected Obj)");

	// Add a column.  We don't use it (graphically), but it has to be there.
	HWND hList = GetDlgItem(fhPanel, IDC_COMPLIST);
	LVCOLUMN lvc;
	lvc.mask = LVCF_TEXT;
	lvc.pszText = "Description";
	ListView_InsertColumn(hList, 0, &lvc);

	IUpdateRollups();
}

void plComponentUtil::EndEditParams(Interface *ip, IUtil *iu)
{
	IDestroyRollups();

	GetCOREInterface()->DeleteRollupPage(fhPanel);
	fhPanel = nil;
	fCurComponent = nil;
	fInterface = nil;
}

void plComponentUtil::SelectionSetChanged(Interface *ip, IUtil *iu)
{
	IUpdateRollups();
}

void plComponentUtil::IUpdateRollups()
{
	if (!fhPanel)
		return;
	
	// Destroy any current rollups
	IDestroyRollups();

	HWND hList = GetDlgItem(fhPanel, IDC_COMPLIST);
	ListView_DeleteAllItems(hList);

	// Check that something is selected.
	int nodeCount = fInterface->GetSelNodeCount();
	if (nodeCount == 0)
	{
		IAddRollups(nil);
		return;
	}

	// Get the components shared among the selected nodes
	int i;
	INodeTab selNodes;
	selNodes.SetCount(nodeCount);
	for (i = 0; i < nodeCount; i++)
		selNodes[i] = fInterface->GetSelNode(i);

	INodeTab sharedComps;
	plSharedComponents(selNodes, sharedComps);

	// Add the shared components to the list
	for (i = 0; i < sharedComps.Count(); i++)
	{
		plComponentBase *comp = ((plMaxNode*)sharedComps[i])->ConvertToComponent();

		if (plComponentDlg::Instance().IIsHidden(comp->ClassID()))
			continue;

		IParamBlock2 *pb = comp->GetParamBlockByID(plComponent::kBlkComp);

		LVITEM item = {0};
		item.mask = LVIF_TEXT | LVIF_PARAM;
		item.pszText = sharedComps[i]->GetName();
		item.iItem = ListView_GetItemCount(hList);
		item.lParam = (LPARAM)comp;
		ListView_InsertItem(hList, &item);
	}

	// Make sure the column is wide enough
	ListView_SetColumnWidth(hList, 0, LVSCW_AUTOSIZE);

	// If there are rollups to show
	if (ListView_GetItemCount(hList) > 0)
	{
		// Try and find the last used rollup
		int idx = IFindListItem(fLastComponent);

		// If last one wasn't found, just use the first
		if (idx == -1)
			idx = 0;

		ListView_SetItemState(hList, idx, LVIS_SELECTED, LVIS_SELECTED);
		ListView_EnsureVisible(hList, idx, FALSE);
	}
	else
		IAddRollups(nil);
}

int plComponentUtil::IFindListItem(plComponentBase* comp)
{
	LVFINDINFO fi;
	fi.flags = LVFI_PARAM;
	fi.lParam = (LPARAM)comp;
	return ListView_FindItem(GetDlgItem(fhPanel, IDC_COMPLIST), -1, &fi);
}

#include "../MaxComponent/plAutoUIComp.h"

void plComponentUtil::IAddRollups(plComponentBase* comp)
{
	if (fCurComponent == comp)
		return;

	IDestroyRollups();
	fCurComponent = comp;
	if (comp)
		fLastComponent = comp;

	//
	// Update the targets dialog
	//
	UInt32 numTargs = 0;
	if (fCurComponent)
	{
		// Only count non-nil targets
		for (UInt32 i = 0; i < fCurComponent->NumTargets(); i++)
			if (fCurComponent->GetTarget(i))
				numTargs++;
	}

	// Put the number of targets in the text box
	char buf[12];
	itoa(numTargs, buf, 10);
	SetWindowText(GetDlgItem(fhPanel, IDC_NUM_TARGS), buf);

	// Enable the forward/back buttons if there are multiple targets
	BOOL useButtons = (numTargs > 1);
	EnableWindow(GetDlgItem(fhPanel, IDC_BACK), useButtons);
	EnableWindow(GetDlgItem(fhPanel, IDC_FORWARD), useButtons);

	//
	// Add the component rollups
	//
	if (fCurComponent)
		fCurComponent->CreateRollups();
}

void plComponentUtil::IDestroyRollups()
{
	if (fCurComponent)
		fCurComponent->DestroyRollups();
}

void plComponentUtil::INextTarget(bool forward)
{
//	fCurComponent = IGetListSelection();
//	plComponentBase *comp = fCurComponent->ConvertToComponent();

	// Loop through the selected component's targets until we find the currently selected node.
	// This gives us a starting point to find the next or previous target in this component's list.
	plMaxNode *curNode = (plMaxNode*)GetCOREInterface()->GetSelNode(0);
	UInt32 count = fCurComponent->NumTargets();
	for (UInt32 i = 0; i < count; i++)
	{
		if (fCurComponent->GetTarget(i) == curNode)
		{
			// Got to loop until the target is non-nil here, so we skip over
			// any deleted nodes.
			UInt32 targIdx = i;
			do
			{
				// Figure out which target to change to
				if (forward)
				{
					if (targIdx == count-1)
						targIdx = 0;
					else
						targIdx = targIdx + 1;
				}
				else
				{
					if (targIdx == 0)
						targIdx = count-1;
					else
						targIdx = targIdx - 1;
				}
			} while (!fCurComponent->GetTarget(targIdx));

			// Select the new target
			theHold.Begin();
			fInterface->RedrawViews(fInterface->GetTime(), REDRAW_BEGIN);

			fInterface->SelectNode(fCurComponent->GetTarget(targIdx));

			fInterface->RedrawViews(fInterface->GetTime(), REDRAW_END);
			theHold.Accept("Select");

			return;
		}
	}
}

void plComponentUtil::IUpdateNodeName(plMaxNode *node)
{
	if (!fhPanel)
		return;

	// Update the name in the list
	int idx = IFindListItem(node->ConvertToComponent());
	if (idx != -1)
	{
		HWND hList = GetDlgItem(fhPanel, IDC_COMPLIST);
		ListView_SetItemText(hList, idx, 0, node->GetName());
		// Make sure the column is wide enough
		ListView_SetColumnWidth(hList, 0, LVSCW_AUTOSIZE);
	}
}

void plComponentUtil::IComponentPreDelete(plComponentBase* comp)
{
	if (fCurComponent == comp)
	{
		IDestroyRollups();
		fCurComponent = nil;
	}
}

////////////////////////////////////////////////////////////////////////////////

#include <vector>

void IGetReferencesRecur(plMaxNode* node, INode* target, std::vector<plMaxNode*>& nodes)
{
	plComponentBase* comp = node->ConvertToComponent();
	if (comp && comp->DoReferenceNode(target))
	{
		const char* name = node->GetName();
		nodes.push_back(node);
	}

	for (int i = 0; i < node->NumberOfChildren(); i++)
	{
		IGetReferencesRecur((plMaxNode*)node->GetChildNode(i), target, nodes);
	}
}

BOOL CALLBACK RefDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
		{
			if (GetCOREInterface()->GetSelNodeCount() > 0)
			{
				INode* node = GetCOREInterface()->GetSelNode(0);

				char buf[256];
				sprintf(buf, "%s is Ref'd By", node->GetName());
				SetWindowText(hDlg, buf);

				std::vector<plMaxNode*> nodes;
				IGetReferencesRecur((plMaxNode*)GetCOREInterface()->GetRootNode(), node, nodes);

				HWND hList = GetDlgItem(hDlg, IDC_REF_LIST);
				for (int i = 0; i < nodes.size(); i++)
				{
					plMaxNode* node = nodes[i];
					int idx = ListBox_AddString(hList, node->GetName());
					ListBox_SetItemData(hList, idx, node);
				}
			}
		}
		return TRUE;

	case WM_COMMAND:
		if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_CLOSE)
		{
			EndDialog(hDlg, 0);
			return TRUE;
		}
		else if (HIWORD(wParam) == LBN_DBLCLK && LOWORD(wParam) == IDC_REF_LIST)
		{
			// If the user double clicked on a component, return it so we can
			// select the nodes it's attached to
			HWND hList = HWND(lParam);
			int sel = ListBox_GetCurSel(hList);
			LRESULT node = ListBox_GetItemData(hList, sel);
			EndDialog(hDlg, node);
			return TRUE;
		}
		break;
	}

	return FALSE;
}

void plComponentUtil::IShowRefdBy()
{
	INode* node = (INode*)DialogBox(hInstance,
									MAKEINTRESOURCE(IDD_REF_BY),
									GetCOREInterface()->GetMAXHWnd(),
									RefDlgProc);
	if (node)
	{
		INodeTab nodes;
		nodes.Append(1, &node);
		plComponentDlg::Instance().SelectComponentTargs(nodes);
	}
}
