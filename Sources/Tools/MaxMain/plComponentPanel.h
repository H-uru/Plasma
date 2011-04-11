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
#include "utilapi.h"
#include "notify.h"

class plMaxNode;
class plComponentBase;

class plComponentUtil : public UtilityObj
{
protected:
	friend class plComponentDlg;
	
	Interface *fInterface;
	HWND fhPanel;
	plComponentBase* fCurComponent;
	// Used to try and load the last displayed component on reload, just for pointer compares
	plComponentBase* fLastComponent;

	plComponentUtil();

public:
	static plComponentUtil& Instance();

	void BeginEditParams(Interface *ip, IUtil *iu);
	void EndEditParams(Interface *ip, IUtil *iu);
	void SelectionSetChanged(Interface *ip, IUtil *iu);
	void DeleteThis() {};

	static BOOL CALLBACK ForwardDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

	bool IsOpen() { return (fhPanel != NULL); }

protected:
	void IUpdateRollups();
	void IAddRollups(plComponentBase* comp);
	void IDestroyRollups();

	void INextTarget(bool forward);

	plComponentBase *IGetListSelection();
	void IDeleteListSelection();
	int IFindListItem(plComponentBase* comp);

	void IShowRefdBy();

	// plComponentDlg is about to delete this comp, get rid of it's interface
	void IComponentPreDelete(plComponentBase* comp);
	// To syncronize with plComponentDlg when a name is changed
	void IUpdateNodeName(plMaxNode *node);
};
