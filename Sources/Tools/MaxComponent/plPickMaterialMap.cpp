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
#include "plPickMaterialMap.h"
#include "max.h"

#include "../MaxMain/plMaxNode.h"

// MAXR4 HACK
// Coming in the backdoor...
#include "hsThread.h"
class hsHackWinFindThread : public hsThread
{
protected:
	enum
	{
		kOK = 1,

		kMtlLibrary = 0x9c6a,
		kMtlEditor,
		kActiveSlot,
		kSelected,
		kScene,
		kNew,
	};

public:
	hsError Run()
	{
		while (1)
		{
			HWND hMtlDlg = FindWindow(NULL, "Material/Map Browser");
			if (hMtlDlg && IsWindowVisible(GetDlgItem(hMtlDlg, kOK)))
			{
				SendMessage(GetDlgItem(hMtlDlg, kScene), BM_CLICK, 0, 0);
				EnableWindow(GetDlgItem(hMtlDlg, kMtlLibrary), FALSE);
				EnableWindow(GetDlgItem(hMtlDlg, kMtlEditor), FALSE);
				EnableWindow(GetDlgItem(hMtlDlg, kActiveSlot), FALSE);
				EnableWindow(GetDlgItem(hMtlDlg, kNew), FALSE);
				return 1;
			}
		}
	}
};

static bool IPickMaterial(IParamBlock2 *pb, int id, int flags)
{
	BOOL newMat, cancel;

	hsHackWinFindThread winFind;
	winFind.Start();

	// Get a material
	MtlBase *mtl = GetCOREInterface()->DoMaterialBrowseDlg(
		GetCOREInterface()->GetMAXHWnd(),
		flags | BROWSE_INSTANCEONLY,
		newMat,
		cancel);

	winFind.Stop();

	if (!cancel)
		pb->SetValue(id, 0, (ReferenceTarget*)mtl);

	return !cancel;
}

bool plPickMaterialMap::PickTexmap(IParamBlock2 *pb, int id)
{
	return IPickMaterial(pb, id, BROWSE_MAPSONLY);
}

#include "../MaxMain/plMtlCollector.h"
#include "resource.h"

static bool GetPickedMtl(HWND hDlg, Mtl** mtl)
{
	HWND hList = GetDlgItem(hDlg, IDC_MTL_LIST);
	int sel = ListBox_GetCurSel(hList);
	if (sel != LB_ERR)
	{
		*mtl = (Mtl*)ListBox_GetItemData(hList, sel);
		return true;
	}

	return false;
}

static BOOL CALLBACK PickMtlProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static plPickMaterialInfo* info;

	if (msg == WM_INITDIALOG)
	{
		info = (plPickMaterialInfo*)lParam;

		MtlSet mtls;
		plMtlCollector::GetMtls(&mtls, nil, info->fFlags);
		
		HWND hList = GetDlgItem(hDlg, IDC_MTL_LIST);

		MtlSet::iterator it = mtls.begin();
		for (; it != mtls.end(); it++)
		{
			int idx = ListBox_AddString(hList, (*it)->GetName());
			ListBox_SetItemData(hList, idx, (*it));
		}

		return TRUE;
	}
	else if (msg == WM_COMMAND)
	{
		if (HIWORD(wParam) == BN_CLICKED && (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL))
		{
			if (LOWORD(wParam) == IDOK)
			{
				if (GetPickedMtl(hDlg, &info->fMtl))
				{
					EndDialog(hDlg, 1);
					return TRUE;
				}
			}

			EndDialog(hDlg, 0);
			return TRUE;
		}
		else if (HIWORD(wParam) == LBN_DBLCLK && LOWORD(wParam) == IDC_MTL_LIST)
		{
			if (GetPickedMtl(hDlg, &info->fMtl))
				EndDialog(hDlg, 1);
			return TRUE;
		}
	}

	return FALSE;
}

Mtl *plPickMaterialMap::PickMaterial(unsigned int flags)
{
	plPickMaterialInfo info;
	info.fMtl = NULL;
	info.fFlags = flags;

	//Mtl *mtl = NULL;
	int ret = DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_PICK_MTL), GetCOREInterface()->GetMAXHWnd(), PickMtlProc, (LPARAM)&info);

	if (ret == 1)
	{
		//pb->SetValue(id, 0, (ReferenceTarget*)mtl);
		//return true;
		return info.fMtl;
	}

	return nil;
}

