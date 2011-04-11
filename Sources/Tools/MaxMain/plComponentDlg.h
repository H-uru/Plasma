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
#include "max.h"
#include "notify.h"

#include <vector>

class plMaxNode;

class plComponentDlg
{
protected:
	friend class plComponentUtil;
	
	HWND fhDlg;
	HMENU fCompMenu;
	HMENU fTypeMenu;
	
	Interface *fInterface;
	POINT fSmallestSize;
	RECT fLastRect;
	
	// The node we're currently editing the comment for
	plMaxNode *fCommentNode;

	std::vector<Class_ID> fHiddenComps;

public:
	~plComponentDlg();
	static plComponentDlg &Instance();

	void Open();
	
	void SelectComponentTargs(INodeTab& nodes);

protected:
	plComponentDlg();

	static BOOL CALLBACK ForwardDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

	void IPositionControls(RECT *newRect, int edge);
	enum { kResizeX = 1, kResizeY = 2, kMoveX = 4, kMoveY = 8 };
	void IPositionControl(HWND hControl, int hDiff, int wDiff=0, int flags=kMoveY);

	HTREEITEM IAddLeaf(HWND hTree, HTREEITEM hParent, const char *text, LPARAM lParam);
	// Search for an item in the tree by name, but only in the children of hParent
	HTREEITEM IFindTreeItem(HWND hTree, const char *name, HTREEITEM hParent);
	HTREEITEM IAddComponent(HWND hTree, plMaxNode *node);
	void IAddComponentsRecur(HWND hTree, plMaxNode *node);
	void ICreateMenu();

	bool IIsComponent(LPARAM lParam);
	bool IIsType(LPARAM lParam);

	void IAttachTreeSelection();
	void ISelectTreeSelection();
	plMaxNode *IGetTreeSelection();
	plMaxNode *ITreeItemToNode(HWND hTree, HTREEITEM hItem);
	void IDeleteComponent(plMaxNode *component);

	void IGetComponentsRecur(HWND hTree, HTREEITEM hItem, INodeTab& nodes);

	void ICreateRightClickMenu();
	void IOpenRightClickMenu();

	HTREEITEM ISearchTree(HWND hTree, LPARAM lParam, HTREEITEM hCur=TVGN_ROOT);
	void IRefreshTree();
	
	void IRemoveUnusedComps();

	static void INotify(void *param, NotifyInfo *info);

	// To syncronize with plComponentUtil when a name is changed
	void IUpdateNodeName(plMaxNode *node);

	void IGetComment();

	bool IIsHidden(Class_ID& cid);
};

// Brings up the copy components dialog.  Stuck here for no particular reason.
void CopyComponents();