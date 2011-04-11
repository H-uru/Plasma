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
#include "hsWindows.h"

class plErrorMsg;
class plMaxNode;

class plGetLocationDlg
{
protected:
	plMaxNode *fNode;
	plErrorMsg *fErrMsg;
	plMaxNode *fDefaultLocation;
	
public:
	static plGetLocationDlg& Instance();
	bool GetLocation(plMaxNode *node, plErrorMsg *errMsg);

	// This should be called at the start of each convert to prevent any
	// problems with bad pointers
	void ResetDefaultLocation();

protected:
	plGetLocationDlg();
	void IListRooms(plMaxNode *node, HWND hList);
	void IAddSelection(HWND hList, bool setDefault);
	void ISetLocation(plMaxNode *locNode);

	static INT_PTR CALLBACK ForwardDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	INT_PTR DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
};