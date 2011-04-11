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
#include "max.h"
#include "iparamb2.h"

#include "plComponentDlg.h"
#include "../MaxComponent/plComponentBase.h"
#include "../MaxComponent/plComponentMgr.h"
#include "../MaxComponent/plComponentReg.h"
#include "resource.h"
#include "plMaxNode.h"
#include "plComponentPanel.h"
#include "plMaxAccelerators.h"

#include <algorithm>

extern HINSTANCE hInstance;

plComponentDlg::plComponentDlg() : fhDlg(nil), fCompMenu(nil), fTypeMenu(nil), fCommentNode(nil)
{
	fInterface = GetCOREInterface();

	RegisterNotification(INotify, 0, NOTIFY_FILE_PRE_OPEN);
	RegisterNotification(INotify, 0, NOTIFY_SYSTEM_PRE_NEW);
	RegisterNotification(INotify, 0, NOTIFY_SYSTEM_PRE_RESET);
	RegisterNotification(INotify, 0, NOTIFY_FILE_PRE_MERGE);
	RegisterNotification(INotify, 0, NOTIFY_PRE_IMPORT);
	RegisterNotification(INotify, 0, NOTIFY_FILE_PRE_SAVE);
	RegisterNotification(INotify, 0, NOTIFY_FILE_PRE_SAVE_OLD);

	RegisterNotification(INotify, 0, NOTIFY_FILE_POST_OPEN);
	RegisterNotification(INotify, 0, NOTIFY_SYSTEM_POST_NEW);
	RegisterNotification(INotify, 0, NOTIFY_SYSTEM_POST_RESET);
	RegisterNotification(INotify, 0, NOTIFY_FILE_POST_MERGE);
	RegisterNotification(INotify, 0, NOTIFY_POST_IMPORT);

	RegisterNotification(INotify, 0, NOTIFY_SYSTEM_SHUTDOWN);
}

plComponentDlg::~plComponentDlg()
{
	if (fhDlg)
	{
		fInterface->UnRegisterDlgWnd(fhDlg);
		DestroyWindow(fhDlg);
	}
	if (fCompMenu)
		DestroyMenu(fCompMenu);
	if (fTypeMenu)
		DestroyMenu(fTypeMenu);
}

plComponentDlg& plComponentDlg::Instance()
{
	static plComponentDlg theInstance;
	return theInstance;
}

void plComponentDlg::Open()
{
	if (!fhDlg)
	{
		fhDlg = CreateDialog(hInstance,
							MAKEINTRESOURCE(IDD_COMP_MAIN),
							GetCOREInterface()->GetMAXHWnd(),
							ForwardDlgProc);

		GetWindowRect(fhDlg, &fLastRect);
		fSmallestSize.x = fLastRect.right - fLastRect.left;
		fSmallestSize.y = fLastRect.bottom - fLastRect.top;

		RECT rect;
		memcpy(&rect, &fLastRect, sizeof(RECT));
		rect.right = rect.left + 235;
		rect.bottom = rect.top + 335;
		IPositionControls(&rect, WMSZ_BOTTOM);
		SetWindowPos(fhDlg, NULL, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_NOMOVE | SWP_NOZORDER);
	}
	
	fInterface->RegisterDlgWnd(fhDlg);
	ShowWindow(fhDlg, SW_SHOW);

	if (IsIconic(fhDlg))
		ShowWindow(fhDlg, SW_RESTORE);
}

void plComponentDlg::IPositionControls(RECT *newRect, int edge)
{
	// Get the new width and height
	int newW = newRect->right - newRect->left;
	int newH = newRect->bottom - newRect->top;

	// If an edge we don't support is being dragged, don't allow the resize.
	if (!(edge == WMSZ_BOTTOM ||
		edge == WMSZ_BOTTOMRIGHT ||
		edge == WMSZ_RIGHT))
	{
		memcpy(newRect, &fLastRect, sizeof(RECT));
		return;
	}

	// If the width or height is too small, set it to the minimum
	if (newW < fSmallestSize.x)
		newRect->right = newRect->left + fSmallestSize.x;
	if (newH < fSmallestSize.y)
		newRect->bottom = newRect->top + fSmallestSize.y;

	// Calculate the new width and height
	int hDiff = (newRect->bottom - newRect->top) - (fLastRect.bottom - fLastRect.top);
	int wDiff = (newRect->right - newRect->left) - (fLastRect.right - fLastRect.left);

	// Copy our new rect to the last rect
	memcpy(&fLastRect, newRect, sizeof(RECT));

	// If the size has changed, reposition and resize controls
	if (hDiff != 0 || wDiff != 0)
	{
		IPositionControl(GetDlgItem(fhDlg, IDC_TREE),			hDiff, wDiff, kResizeX | kResizeY);
		IPositionControl(GetDlgItem(fhDlg, IDC_COMMENT_TEXT),	hDiff);
		IPositionControl(GetDlgItem(fhDlg, IDC_COMMENTS),		hDiff, wDiff, kResizeX | kMoveY);
		IPositionControl(GetDlgItem(fhDlg, IDC_ATTACH),			hDiff);
	}

	InvalidateRect(fhDlg, NULL, TRUE);
}

void plComponentDlg::IPositionControl(HWND hControl, int hDiff, int wDiff, int flags)
{
	RECT rect;
	GetWindowRect(hControl, &rect);

	hsAssert(!((flags & kMoveX) & (flags & kResizeX)), "Moving AND resizing in X in IPositionControl");
	hsAssert(!((flags & kMoveY) & (flags & kResizeY)), "Moving AND resizing in Y in IPositionControl");

	if (flags & kMoveX || flags & kMoveY)
	{
		POINT pos = { rect.left, rect.top };
		ScreenToClient(fhDlg, &pos);
		if (flags & kMoveX)
			pos.x += wDiff;
		if (flags & kMoveY)
			pos.y += hDiff;

		SetWindowPos(hControl, NULL, pos.x, pos.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOREDRAW);
	}

	if (flags & kResizeX || flags & kResizeY)
	{
		int w = rect.right - rect.left;
		int h = rect.bottom - rect.top;
		if (flags & kResizeX)
			w += wDiff;
		if (flags & kResizeY)
			h += hDiff;

		SetWindowPos(hControl, NULL, 0, 0, w, h, SWP_NOMOVE | SWP_NOZORDER | SWP_NOREDRAW);
	}
}

void plComponentDlg::IGetComment()
{
	if (fCommentNode)
	{
		// Get the text from the edit and store it in the UserPropBuffer
		int len = GetWindowTextLength(GetDlgItem(fhDlg, IDC_COMMENTS))+1;
		if (len != 0)
		{
			char *buf = TRACKED_NEW char[len];
			GetDlgItemText(fhDlg, IDC_COMMENTS, buf, len);
			fCommentNode->SetUserPropBuffer(buf);
			delete [] buf;
		}
		else
			fCommentNode->SetUserPropBuffer("");
	}
}

BOOL plComponentDlg::ForwardDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return Instance().DlgProc(hDlg, msg, wParam, lParam);
}

#define MENU_ID_START 41000

BOOL plComponentDlg::DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
		fhDlg = hDlg;
		IAddComponentsRecur(GetDlgItem(hDlg, IDC_TREE), (plMaxNode*)GetCOREInterface()->GetRootNode());

		ICreateMenu();
		ICreateRightClickMenu();
		return TRUE;

	case WM_SIZING:
		IPositionControls((RECT*)lParam, wParam);
		return TRUE;

	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE)
			plMaxAccelerators::Enable();
		else
			plMaxAccelerators::Disable();
		return TRUE;

	case WM_COMMAND:
		if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDCANCEL)
		{
			ShowWindow(hDlg, SW_HIDE);
			fInterface->UnRegisterDlgWnd(hDlg);
			return TRUE;
		}
		else if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_ATTACH)
		{
			IAttachTreeSelection();
			return TRUE;
		}
		else if (HIWORD(wParam) == EN_KILLFOCUS && LOWORD(wParam) == IDC_COMMENTS)
		{
			IGetComment();
			return TRUE;
		}
		// "Refresh" menu item
		else if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == ID_REFRESH)
		{
			IRefreshTree();
			return TRUE;
		}
		// "Remove unused components" menu item
		else if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == ID_REMOVE_UNUSED)
		{
			IRemoveUnusedComps();
			return TRUE;
		}
		// Item selected from 'New' menu
		else if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) >= MENU_ID_START)
		{
			ClassDesc *desc = plComponentMgr::Inst().Get(LOWORD(wParam)-MENU_ID_START);
			// If this is a component type (not a category)
			if (desc)
			{
				// Create an object of that type and a node to reference it
				Object *obj = (Object*)GetCOREInterface()->CreateInstance(desc->SuperClassID(), desc->ClassID());
				INode *node = GetCOREInterface()->CreateObjectNode(obj);

				plComponentBase *comp = (plComponentBase*)obj;
				node->Hide(!comp->AllowUnhide());
				node->Freeze(TRUE);

				// Add the new component to the tree
				HWND hTree = GetDlgItem(hDlg, IDC_TREE);
				HTREEITEM item = IAddComponent(hTree, (plMaxNode*)node);
				TreeView_SelectItem(hTree, item);
				TreeView_EnsureVisible(hTree, item);
			}
		}
		break;

	case WM_NOTIFY:
		NMHDR *nmhdr = (NMHDR*)lParam;
		if (nmhdr->idFrom == IDC_TREE)
		{
			switch (nmhdr->code)
			{
			case TVN_SELCHANGED:
				{
					NMTREEVIEW *tv = (NMTREEVIEW*)lParam;

					IGetComment();

					bool isComponent = IIsComponent(tv->itemNew.lParam);

					// If the new selection is a component, enable the attach button and comment field
					EnableWindow(GetDlgItem(hDlg, IDC_ATTACH), isComponent);
					SendDlgItemMessage(hDlg, IDC_COMMENTS, EM_SETREADONLY, !isComponent, 0);

					if (isComponent)
					{
						fCommentNode = (plMaxNode*)tv->itemNew.lParam;
						
						TSTR buf;
						fCommentNode->GetUserPropBuffer(buf);
						SetDlgItemText(hDlg, IDC_COMMENTS, buf);
					}
					else
					{
						fCommentNode = nil;
						SetDlgItemText(hDlg, IDC_COMMENTS, "");
					}

					return TRUE;
				}
				break;

			case TVN_BEGINLABELEDIT:
				// If this isn't a component, don't allow the edit
				if (!IIsComponent(((NMTVDISPINFO*)lParam)->item.lParam))
				{
					SetWindowLong(hDlg, DWL_MSGRESULT, TRUE);
					return TRUE;
				}

				// The edit box this creates kills the focus on our window, causing
				// accelerators to be enabled.  Add an extra disable to counteract that.
				plMaxAccelerators::Disable();

				return TRUE;

			// Finishing changing the name of a component
			case TVN_ENDLABELEDIT:
				{
					NMTVDISPINFO *di = (NMTVDISPINFO*)lParam;
					char* text = di->item.pszText;
					// If the name was changed...
					if (text && *text != '\0')
					{
						// Update the name of the node
						plMaxNode *node = IGetTreeSelection();
						node->SetName(text);

						// Update the name in the panel too
						if (plComponentUtil::Instance().IsOpen())
							plComponentUtil::Instance().IUpdateNodeName(node);

						// Make sure Max knows the file was changed
						SetSaveRequiredFlag();

						// Return true to keep the changes
						SetWindowLong(hDlg, DWL_MSGRESULT, TRUE);
					}

					plMaxAccelerators::Enable();
				}
				return TRUE;

			// User double-clicked.  Select the objects the selected component is attached to.
			case NM_DBLCLK:
				ISelectTreeSelection();
				return TRUE;

			case NM_RCLICK:
				IOpenRightClickMenu();
				return TRUE;
				
			case TVN_KEYDOWN:
				// User pressed delete
				if (((NMTVKEYDOWN*)lParam)->wVKey == VK_DELETE)
				{
					IDeleteComponent(IGetTreeSelection());
					return TRUE;
				}
				break;
			}
		}
		break;
	}

	return FALSE;
}

HTREEITEM plComponentDlg::IAddLeaf(HWND hTree, HTREEITEM hParent, const char *text, LPARAM lParam)
{
	TVITEM tvi = {0};
	tvi.mask       = TVIF_TEXT | TVIF_PARAM;
	tvi.pszText    = (char*)text;
	tvi.cchTextMax = strlen(text);  
	tvi.lParam     = lParam;

	TVINSERTSTRUCT tvins = {0};
	tvins.item         = tvi;
	tvins.hParent      = hParent;
	tvins.hInsertAfter = TVI_SORT;

	return TreeView_InsertItem(hTree, &tvins);
}

HTREEITEM plComponentDlg::IFindTreeItem(HWND hTree, const char *name, HTREEITEM hParent)
{
	HTREEITEM hChild = TreeView_GetChild(hTree, hParent);

	while (hChild)
	{
		char buf[256];
		TVITEM tvi;
		tvi.mask  = TVIF_TEXT;
		tvi.hItem = hChild;
		tvi.pszText = buf;
		tvi.cchTextMax = sizeof(buf);
		TreeView_GetItem(hTree, &tvi);

		if (!strcmp(name, tvi.pszText))
			return hChild;

		hChild = TreeView_GetNextSibling(hTree, hChild);
	}

	return nil;
}

HTREEITEM plComponentDlg::IAddComponent(HWND hTree, plMaxNode *node)
{
	plComponentBase *comp = node->ConvertToComponent();

	// Try and find the component category in the tree
	const char *category = comp->GetCategory();
	HTREEITEM hCat = IFindTreeItem(hTree, category, TVI_ROOT);
	// If it isn't there yet, add it
	if (!hCat)
		hCat = IAddLeaf(hTree, TVI_ROOT, category, 0);

	// Try and find the component type in the tree
	int idx = plComponentMgr::Inst().FindClassID(comp->ClassID());
	HTREEITEM hType = ISearchTree(hTree, idx+1, hCat);
	if (!hType)
	{
		// If it isn't there yet, add it
		TSTR type;
		comp->GetClassName(type);

		if (IIsHidden(comp->ClassID()))
			type.Append(" (Hidden)");

		hType = IAddLeaf(hTree, hCat, type, idx+1);
	}

	// Add the name of this component to this type
	return IAddLeaf(hTree, hType, node->GetName(), (LPARAM)node);
}

void plComponentDlg::IAddComponentsRecur(HWND hTree, plMaxNode *node)
{
	if (node->IsComponent())
		IAddComponent(hTree, node);

	for (int i = 0; i < node->NumberOfChildren(); i++)
	{
		plMaxNode *child = (plMaxNode*)node->GetChildNode(i);
		IAddComponentsRecur(hTree, child);
	}
}

void plComponentDlg::ICreateMenu()
{
	// Add a refresh option to the system menu, for those rare cases where the manager gets out of sync
	HMENU hMenu = GetMenu(fhDlg);

	HMENU hNew = CreatePopupMenu();
	InsertMenu(hMenu, 0, MF_POPUP | MF_STRING | MF_BYPOSITION, (UINT)hNew, "New");

	const char *lastCat = nil;
	HMENU hCurType = nil;

	UInt32 count = plComponentMgr::Inst().Count();
	for (UInt32 i = 0; i < count; i++)
	{
		plComponentClassDesc *desc = (plComponentClassDesc*)plComponentMgr::Inst().Get(i);

		// Don't put in the create menu if obsolete
		if (desc->IsObsolete())
			continue;

		if (!lastCat || strcmp(lastCat, desc->Category()))
		{
			lastCat = desc->Category();

			hCurType = CreatePopupMenu();
			AppendMenu(hNew, MF_POPUP | MF_STRING, (UINT)hCurType, lastCat);
		}

		AppendMenu(hCurType, MF_STRING, MENU_ID_START+i, desc->ClassName());
	}
}

// Taking advantage of the fact that the node pointers we store in the lParam
// will certainly be higher than the number of component types
bool plComponentDlg::IIsComponent(LPARAM lParam)
{
	return (lParam > plComponentMgr::Inst().Count()+1);
}

bool plComponentDlg::IIsType(LPARAM lParam)
{
	return (lParam > 0 && lParam <= plComponentMgr::Inst().Count()+1);
}

void plComponentDlg::IAttachTreeSelection()
{
	HWND hTree = GetDlgItem(fhDlg, IDC_TREE);

	// Get the current selection from the tree
	HTREEITEM hSelected = TreeView_GetSelection(hTree);
	TVITEM item;
	item.mask = TVIF_PARAM;
	item.hItem = hSelected;
	TreeView_GetItem(hTree, &item);

	// If the item has a lParam it is a component
	if (IIsComponent(item.lParam))
	{
		plMaxNode *node = (plMaxNode*)item.lParam;
		plComponentBase *comp = node->ConvertToComponent();

		// Add each of the selected nodes that is not a component to the targets list
		int count = fInterface->GetSelNodeCount();
		for (int i = 0; i < count; i++)
		{
			plMaxNode *target = (plMaxNode*)fInterface->GetSelNode(i);
			if (!target->IsComponent())
				comp->AddTarget(target);
		}

		// Update the rollups to reflect the new component
		if (plComponentUtil::Instance().IsOpen())
			plComponentUtil::Instance().IUpdateRollups();
	}
}

// Wow, this INodeTab class is very thorough
bool FindNodeInTab(INode *node, INodeTab& nodes)
{
	for (int i = 0; i < nodes.Count(); i++)
	{
		if (node == nodes[i])
			return true;
	}

	return false;
}

void plComponentDlg::SelectComponentTargs(INodeTab& nodes)
{
	// Make an INode tab with all the targets in it
	INodeTab targets;
	for (int i = 0; i < nodes.Count(); i++)
	{
		plComponentBase *comp = ((plMaxNode*)nodes[i])->ConvertToComponent();

		for (int j = 0; j < comp->NumTargets(); j++)
		{
			INode *node = comp->GetTarget(j);
			if (node && !FindNodeInTab(node, targets))
				targets.Append(1, &node);
		}
	}

	// If the user is selecting a single component, make sure it is selected in the rollup too
	if (plComponentUtil::Instance().IsOpen() && nodes.Count() == 1)
		plComponentUtil::Instance().fLastComponent = ((plMaxNode*)nodes[0])->ConvertToComponent();
	
	theHold.Begin();
	fInterface->RedrawViews(fInterface->GetTime(), REDRAW_BEGIN);
	fInterface->ClearNodeSelection(FALSE);				// Deselect current nodes

	// If there is at least one valid target, select it
	if (targets.Count() > 0)
		fInterface->SelectNodeTab(targets, TRUE, FALSE);

	fInterface->RedrawViews(fInterface->GetTime(), REDRAW_END);
	theHold.Accept("Select");
}

void plComponentDlg::ISelectTreeSelection()
{
	INodeTab nodes;

	INode *curComponent = (INode*)IGetTreeSelection();
	if (curComponent)
	{
		nodes.Append(1, &curComponent);
	}
	else
	{
		HWND hTree = GetDlgItem(fhDlg, IDC_TREE);
		HTREEITEM hRoot = TreeView_GetSelection(hTree);

		IGetComponentsRecur(hTree, hRoot, nodes);
	}

	SelectComponentTargs(nodes);
}

void plComponentDlg::IGetComponentsRecur(HWND hTree, HTREEITEM hItem, INodeTab& nodes)
{
	if (hItem)
	{
		INode *node = (INode*)ITreeItemToNode(hTree, hItem);
		if (node)
			nodes.Append(1, &node);
		else
		{
			HTREEITEM hChild = TreeView_GetChild(hTree, hItem);
			IGetComponentsRecur(hTree, hChild, nodes);

			while (hChild = TreeView_GetNextSibling(hTree, hChild))
			{
				IGetComponentsRecur(hTree, hChild, nodes);
			}
		}
	}
}

void plComponentDlg::IDeleteComponent(plMaxNode *component)
{
	if (!component)
		return;

	// Make sure this components interface isn't showing
	if (plComponentUtil::Instance().IsOpen())
		plComponentUtil::Instance().IComponentPreDelete(component->ConvertToComponent());
	
	// Delete the component from the scene
	theHold.Begin();
	fInterface->DeleteNode(component);
	theHold.Accept(_T("Delete Component"));

	// Delete the component from the tree
	HWND hTree = GetDlgItem(fhDlg, IDC_TREE);
	HTREEITEM hItem = TreeView_GetSelection(hTree);
	HTREEITEM hParent = TreeView_GetParent(hTree, hItem);
	TreeView_DeleteItem(hTree, hItem);

	// If that was the only component of this type, delete the type too
	if (!TreeView_GetChild(hTree, hParent))
	{
		HTREEITEM hCategory = TreeView_GetParent(hTree, hParent);
		TreeView_DeleteItem(hTree, hParent);

		// If this is the only type in this category, delete the category too!
		// Sadly, this is the most we can delete.
		if (!TreeView_GetChild(hTree, hCategory))
			TreeView_DeleteItem(hTree, hCategory);
	}

	// Update the rollups in case the selected object had this component attached
	if (plComponentUtil::Instance().IsOpen())
		plComponentUtil::Instance().IUpdateRollups();
}

plMaxNode *plComponentDlg::IGetTreeSelection()
{
	HWND hTree = GetDlgItem(fhDlg, IDC_TREE);

	HTREEITEM hItem = TreeView_GetSelection(hTree);
	return ITreeItemToNode(hTree, hItem);
}

plMaxNode *plComponentDlg::ITreeItemToNode(HWND hTree, HTREEITEM hItem)
{
	if (hItem)
	{
		TVITEM item;
		item.mask = TVIF_PARAM;
		item.hItem = hItem;
		TreeView_GetItem(hTree, &item);

		if (IIsComponent(item.lParam))
			return (plMaxNode*)item.lParam;
	}

	return nil;
}

enum
{
	// Comp menu
	kMenuDelete = 1,
	kMenuRename,
	kMenuCopy,

	// Type menu
	kMenuHide
};

void plComponentDlg::ICreateRightClickMenu()
{
	fCompMenu = CreatePopupMenu();
	AppendMenu(fCompMenu, MF_STRING, kMenuDelete, "Delete");
	AppendMenu(fCompMenu, MF_STRING, kMenuRename, "Rename");
	AppendMenu(fCompMenu, MF_STRING, kMenuCopy, "Copy");

	fTypeMenu = CreatePopupMenu();
	AppendMenu(fTypeMenu, MF_STRING, kMenuHide, "Hide/Show");
}

void plComponentDlg::IOpenRightClickMenu()
{
	HWND hTree = GetDlgItem(fhDlg, IDC_TREE);

	// Get the position of the cursor in screen and tree client coords
	POINT point, localPoint;
	GetCursorPos(&point);
	localPoint = point;
	ScreenToClient(hTree, &localPoint);

	// Check if there is a tree item at that point
	TVHITTESTINFO hitTest;
	hitTest.pt = localPoint;
	TreeView_HitTest(hTree, &hitTest);
	if (!(hitTest.flags & TVHT_ONITEMLABEL))
		return;

	// Check if the tree item has an lParam (is a component)
	TVITEM item;
	item.mask = TVIF_PARAM;
	item.hItem = hitTest.hItem;
	TreeView_GetItem(hTree, &item);

	HMENU menu = nil;
	if (IIsComponent(item.lParam))
		menu = fCompMenu;
	else if (IIsType(item.lParam))
		menu = fTypeMenu;
	else
		return;

	// Select the item we're working with, so the user isn't confused
	TreeView_SelectItem(hTree, item.hItem);

	// Create the popup menu and get the option the user selects
	SetForegroundWindow(fhDlg);
	int sel = TrackPopupMenu(menu, TPM_NONOTIFY | TPM_RETURNCMD, point.x, point.y, 0, fhDlg, NULL);
	switch(sel)
	{
	case kMenuDelete:
		IDeleteComponent((plMaxNode*)item.lParam);
		break;

	case kMenuRename:
		TreeView_EditLabel(hTree, hitTest.hItem);
		break;

	case kMenuCopy:
		{
			// Component to copy
			INode *node = (INode*)item.lParam;
			INodeTab tab;
			tab.Append(1, &node);

			// Copy
			INodeTab copy;

			// Make the copy
			fInterface->CloneNodes(tab, Point3(0,0,0), true, NODE_COPY, NULL, &copy);

			// Delete the targets for the copy and add it to the tree
			plMaxNode *newNode = (plMaxNode*)copy[0];
			newNode->ConvertToComponent()->DeleteAllTargets();
			HTREEITEM hItem = IAddComponent(GetDlgItem(fhDlg, IDC_TREE), newNode);
			TreeView_SelectItem(GetDlgItem(fhDlg, IDC_TREE), hItem);
		}
		break;

	case kMenuHide:
		{
			ClassDesc *desc = plComponentMgr::Inst().Get(item.lParam-1);

			std::vector<Class_ID>::iterator it;
			it = std::find(fHiddenComps.begin(), fHiddenComps.end(), desc->ClassID());

			TSTR name = desc->ClassName();
			if (it == fHiddenComps.end())
			{
				fHiddenComps.push_back(desc->ClassID());
				name.Append(" (Hidden)");
			}
			else
				fHiddenComps.erase(it);

			item.mask = TVIF_TEXT;
			item.pszText = name;
			TreeView_SetItem(GetDlgItem(fhDlg, IDC_TREE), &item);

			plComponentUtil::Instance().IUpdateRollups();
		}
		break;
	}

	PostMessage(fhDlg, WM_USER, 0, 0);
}

HTREEITEM plComponentDlg::ISearchTree(HWND hTree, LPARAM lParam, HTREEITEM hCur)
{
	// Get the param for the current item
	TVITEM tvi;
	tvi.mask  = TVIF_PARAM;
	tvi.hItem = hCur;
	TreeView_GetItem(hTree, &tvi);

	// If the lParam matches the one searching for, return the handle
	if (tvi.lParam == lParam)
		return hCur;

	// Do a recursive search on the items children
	HTREEITEM hChild = TreeView_GetChild(hTree, hCur);
	while (hChild)
	{
		HTREEITEM hResult = ISearchTree(hTree, lParam, hChild);
		if (hResult)
			return hResult;

		hChild = TreeView_GetNextSibling(hTree, hChild);
	}

	return NULL;
}

void plComponentDlg::IRefreshTree()
{
	if (fhDlg)
	{
		fCommentNode = nil;

		HWND hTree = GetDlgItem(fhDlg, IDC_TREE);
		TreeView_DeleteAllItems(hTree);
		IAddComponentsRecur(hTree, (plMaxNode*)GetCOREInterface()->GetRootNode());
	}
}

void plComponentDlg::INotify(void *param, NotifyInfo *info)
{
	if (info->intcode == NOTIFY_SYSTEM_SHUTDOWN)
	{
		UnRegisterNotification(INotify, 0, NOTIFY_FILE_PRE_OPEN);
		UnRegisterNotification(INotify, 0, NOTIFY_SYSTEM_PRE_NEW);
		UnRegisterNotification(INotify, 0, NOTIFY_SYSTEM_PRE_RESET);
		UnRegisterNotification(INotify, 0, NOTIFY_FILE_PRE_MERGE);
		UnRegisterNotification(INotify, 0, NOTIFY_PRE_IMPORT);
		UnRegisterNotification(INotify, 0, NOTIFY_FILE_PRE_SAVE);
		UnRegisterNotification(INotify, 0, NOTIFY_FILE_PRE_SAVE_OLD);

		UnRegisterNotification(INotify, 0, NOTIFY_FILE_POST_OPEN);
		UnRegisterNotification(INotify, 0, NOTIFY_SYSTEM_POST_NEW);
		UnRegisterNotification(INotify, 0, NOTIFY_SYSTEM_POST_RESET);
		UnRegisterNotification(INotify, 0, NOTIFY_FILE_POST_MERGE);
		UnRegisterNotification(INotify, 0, NOTIFY_POST_IMPORT);

		UnRegisterNotification(INotify, 0, NOTIFY_SYSTEM_SHUTDOWN);
	}
	// New nodes are coming in, refresh the scene component list
	else if (info->intcode == NOTIFY_FILE_POST_OPEN ||
			 info->intcode == NOTIFY_SYSTEM_POST_NEW ||
			 info->intcode == NOTIFY_SYSTEM_POST_RESET ||
			 info->intcode == NOTIFY_FILE_POST_MERGE ||
			 info->intcode == NOTIFY_POST_IMPORT)
	{
		Instance().IRefreshTree();
	}
	// Nodes may be going away, save the comment now
	else if (info->intcode == NOTIFY_FILE_PRE_OPEN ||
			 info->intcode == NOTIFY_SYSTEM_PRE_NEW ||
			 info->intcode == NOTIFY_SYSTEM_PRE_RESET ||
			 info->intcode == NOTIFY_FILE_PRE_MERGE ||
			 info->intcode == NOTIFY_PRE_IMPORT ||
			 info->intcode == NOTIFY_FILE_PRE_SAVE ||
			 info->intcode == NOTIFY_FILE_PRE_SAVE_OLD)
	{
		// This is causing a crash, so for now if you add a comment and don't
		// pick another component or close the manager before closing the file,
		// you lose the comment -Colin
//		Instance().IGetComment();
	}
}

void plComponentDlg::IUpdateNodeName(plMaxNode *node)
{
	if (!fhDlg)
		return;

	// Update the name in the tree too
	HWND hTree = GetDlgItem(fhDlg, IDC_TREE);
	TVITEM tvi = {0};
	tvi.hItem = ISearchTree(hTree, (LPARAM)node);
	tvi.mask = TVIF_TEXT;
	tvi.pszText = node->GetName();
	TreeView_SetItem(hTree, &tvi);
}

void FindUnusedCompsRecur(plMaxNode *node, std::vector<plMaxNode*>& unused)
{
	plComponentBase *comp = node->ConvertToComponent();
	if (comp)
	{
		bool isAttached = false;

		int num = comp->NumTargets();
		for (int i = 0; i < num; i++)
		{
			if (comp->GetTarget(i))
			{
				isAttached = true;
				break;
			}
		}

		if (!isAttached)
			unused.push_back(node);
	}
	
	for (int i = 0; i < node->NumberOfChildren(); i++)
		FindUnusedCompsRecur((plMaxNode*)node->GetChildNode(i), unused);
}

void plComponentDlg::IRemoveUnusedComps()
{
	std::vector<plMaxNode*> unused;
	FindUnusedCompsRecur((plMaxNode*)GetCOREInterface()->GetRootNode(), unused);

	for (int i = 0; i < unused.size(); i++)
		GetCOREInterface()->DeleteNode(unused[i], FALSE);

	IRefreshTree();
}

bool plComponentDlg::IIsHidden(Class_ID& cid)
{
	return (std::find(fHiddenComps.begin(), fHiddenComps.end(), cid) != fHiddenComps.end());
}

////////////////////////////////////////////////////////////////////////////////

#include "hsUtils.h"

class plCopyCompCallback : public HitByNameDlgCallback
{
protected:
	Tab<plComponentBase*> fSharedComps;
	INodeTab fSelectedNodes;

public:
	bool GetComponents()
	{
		fSelectedNodes.ZeroCount();
		fSharedComps.ZeroCount();

		Interface *ip = GetCOREInterface();

		int nodeCount = ip->GetSelNodeCount();
		if (nodeCount == 0)
			return false;
		
		// Get the components shared among the selected nodes
		int i;
		fSelectedNodes.SetCount(nodeCount);
		for (i = 0; i < nodeCount; i++)
			fSelectedNodes[i] = ip->GetSelNode(i);

		INodeTab sharedComps;
		if (plSharedComponents(fSelectedNodes, sharedComps) == 0)
			return false;

		// Put the shared components in a list
		fSharedComps.SetCount(sharedComps.Count());
		for (i = 0; i < sharedComps.Count(); i++)
			fSharedComps[i] = ((plMaxNode*)sharedComps[i])->ConvertToComponent();

		return true;
	}

	virtual TCHAR *dialogTitle() { return "Select Nodes"; }
	virtual TCHAR *buttonText() { return "Copy"; }

	virtual int filter(INode *node)
	{
		// Make sure this node doesn't already have the components
		for (int i = 0; i < fSelectedNodes.Count(); i++)
		{
			if (fSelectedNodes[i] == node)
				return FALSE;
		}

		return TRUE;
	}

	virtual void proc(INodeTab &nodeTab)
	{
		for (int i = 0; i < nodeTab.Count(); i++)
		{
			for (int j = 0; j < fSharedComps.Count(); j++)
			{
				fSharedComps[j]->AddTarget((plMaxNodeBase*)nodeTab[i]);
			}
		}
	}
};
static plCopyCompCallback copyCompCallback;

void CopyComponents()
{
	if (copyCompCallback.GetComponents())
		GetCOREInterface()->DoHitByNameDialog(&copyCompCallback);
	else
	{
		int count = GetCOREInterface()->GetSelNodeCount();
		if (count == 0)
			hsMessageBox("No object(s) selected", "Component Copy", hsMessageBoxNormal);
		else if (count > 1)
			hsMessageBox("No components are shared among the selected objects", "Component Copy", hsMessageBoxNormal);
		else
			hsMessageBox("No components on the selected object", "Component Copy", hsMessageBoxNormal);
	}
}
