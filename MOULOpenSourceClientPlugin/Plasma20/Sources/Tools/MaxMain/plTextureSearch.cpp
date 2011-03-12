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
#include "plTextureSearch.h"
#include "resource.h"
#include "hsUtils.h"

#define PB2Export __declspec( dllexport )	// Because I don't feel like including all the paramblock crap
#include "pbbitmap.h"
#include "bmmlib.h"
#include "IMtlEdit.h"

#include "plMtlCollector.h"
#include "plMaxAccelerators.h"
#include "../MaxPlasmaMtls/Layers/plPlasmaMAXLayer.h"
#include "../../AssetMan/PublicInterface/MaxAssInterface.h"

// Not a class member so we don't have to make everyone who uses this know about AssetMan
static jvUniqueId gAssetID;

plTextureSearch::plTextureSearch() : fDlg(NULL)
{
	gAssetID.SetEmpty();
	memset(fFileName, 0, sizeof(fFileName));
}

plTextureSearch& plTextureSearch::Instance()
{
	static plTextureSearch theInstance;
	return theInstance;
}

void plTextureSearch::Toggle()
{
	if (!fDlg)
	{
		fDlg = CreateDialog(hInstance,
							MAKEINTRESOURCE(IDD_FIND_TEXTURE),
							GetCOREInterface()->GetMAXHWnd(),
							ForwardDlgProc);

		HWND hList = GetDlgItem(fDlg, IDC_TEXTURE_LIST);
		LVCOLUMN lvc;
		lvc.mask = LVCF_TEXT;
		lvc.pszText = "Material";
		ListView_InsertColumn(hList, 0, &lvc);

		lvc.pszText = "Layer";
		ListView_InsertColumn(hList, 1, &lvc);

		lvc.pszText = "Texture";
		ListView_InsertColumn(hList, 2, &lvc);

		IUpdateTextures(kUpdateLoadList);

		GetCOREInterface()->RegisterDlgWnd(fDlg);
		ShowWindow(fDlg, SW_SHOW);
	}
	else
	{
		DestroyWindow(fDlg);
		fDlg = NULL;
	}
}


BOOL plTextureSearch::ForwardDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return Instance().DlgProc(hDlg, msg, wParam, lParam);
}

BOOL plTextureSearch::DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE)
			plMaxAccelerators::Enable();
		else
			plMaxAccelerators::Disable();
		return TRUE;

	case WM_COMMAND:
		if (HIWORD(wParam) == BN_CLICKED)
		{
			int id = LOWORD(wParam);
			if (id == IDCANCEL)
			{
				Toggle();
				return TRUE;
			}
			else if (id == IDC_UPDATE_BUTTON)
			{
				IUpdateTextures(kUpdateLoadList);
				return TRUE;
			}
			else if (id == IDC_REPLACE_ALL_BUTTON)
			{
				if (hsMessageBox("Are you sure?", "Confirmation", hsMessageBoxYesNo) == hsMBoxYes)
				{
					IUpdateTextures(kUpdateReplace);
				}
				return TRUE;
			}
			else if (id == IDC_SET_ALL_BUTTON)
			{
				if (hsMessageBox("Are you sure?", "Confirmation", hsMessageBoxYesNo) == hsMBoxYes)
				{
					IUpdateTextures(kUpdateSetSize);
				}
				return TRUE;
			}
			else if (id == IDC_REPLACE_BUTTON)
			{
				IPickReplaceTexture();
				return TRUE;
			}
		}
		else if (HIWORD(wParam) == EN_CHANGE && LOWORD(wParam) == IDC_FIND_EDIT)
		{
			bool findText = (SendDlgItemMessage(hDlg, IDC_FIND_EDIT, EM_LINELENGTH, 0, 0) > 0);
			bool replace = (fFileName[0] != '\0');

			EnableWindow(GetDlgItem(hDlg, IDC_REPLACE_BUTTON), findText);
			EnableWindow(GetDlgItem(hDlg, IDC_REPLACE_ALL_BUTTON), findText && replace);

			return TRUE;
		}
		break;

	case WM_NOTIFY:
		{
			NMHDR* nmhdr = (NMHDR*)lParam;
			// User double-clicked a material in the texture list
			if (nmhdr->idFrom == IDC_TEXTURE_LIST && nmhdr->code == NM_DBLCLK)
			{
				NMITEMACTIVATE* itema = (NMITEMACTIVATE*)lParam;
				if (itema->iItem != -1)
				{
					// Get the material the user clicked on
					HWND hList = GetDlgItem(fDlg, IDC_TEXTURE_LIST);
					LV_ITEM item;
					item.iItem = itema->iItem;
					item.mask = LVIF_PARAM;
					ListView_GetItem(hList, &item);

					Mtl* mtl = (Mtl*)item.lParam;

					// Make sure the material is still in the scene (paranoid check)
					MtlSet mtls;
					plMtlCollector::GetMtls(&mtls, nil, plMtlCollector::kPlasmaOnly | plMtlCollector::kNoMultiMtl);
					if (mtls.find(mtl) != mtls.end())
					{
						// Put the material in the current slot of the material editor
						IMtlEditInterface* mtlInterface = GetMtlEditInterface();
						int slot = mtlInterface->GetActiveMtlSlot();
						mtlInterface->PutMtlToMtlEditor(mtl, slot);
					}
				}

				return TRUE;
			}
		}
		break;
	}

	return FALSE;
}

static int FloorPow2(int value)
{
	int v;
	for (v = 1; v <= value; v <<= 1);
	return v >> 1;
}

void plTextureSearch::IUpdateTextures(plTextureSearch::Update update)
{
	MtlSet mtls;
	plMtlCollector::GetMtls(&mtls, nil, plMtlCollector::kPlasmaOnly | plMtlCollector::kNoMultiMtl);

	char searchStr[256];
	GetDlgItemText(fDlg, IDC_FIND_EDIT, searchStr, sizeof(searchStr));
	strlwr(searchStr);

	HWND hList = GetDlgItem(fDlg, IDC_TEXTURE_LIST);
	ListView_DeleteAllItems(hList);

	int sizeX = -1, sizeY = -1;

	HWND hCombo = GetDlgItem(fDlg, IDC_SIZE_COMBO);

	// If we're updating the size, get whatever the user selected
	if (update == kUpdateSetSize)
	{
		int sel = ComboBox_GetCurSel(hCombo);
		UInt32 data = ComboBox_GetItemData(hCombo, sel);
		sizeX = LOWORD(data);
		sizeY = HIWORD(data);
	}

	MtlSet::iterator it = mtls.begin();
	for (; it != mtls.end(); it++)
	{
		Mtl *mtl = (*it);

		LayerSet layers;
		plMtlCollector::GetMtlLayers(mtl, layers);

		LayerSet::iterator layerIt = layers.begin();
		for (; layerIt != layers.end(); layerIt++)
		{
			plPlasmaMAXLayer *layer = (*layerIt);

			int numBitmaps = layer->GetNumBitmaps();

			for (int i = 0; i < numBitmaps; i++)
			{
				PBBitmap *pbbm = layer->GetPBBitmap(i);
				if (pbbm)
				{
					const char *name = pbbm->bi.Filename();
					if (name && *name != '\0')
					{
						char buf[256];
						strncpy(buf, name, sizeof(buf));
						strlwr(buf);

						// If we don't have a search string, or we do and it was
						// found in the texture name, add the texture to the list.
						if (searchStr[0] == '\0' || strstr(buf, searchStr))
						{
							if (update == kUpdateLoadList)
							{
								LVITEM item = {0};
								item.mask = LVIF_TEXT | LVIF_PARAM;
								item.pszText = mtl->GetName();
								item.lParam = (LPARAM)mtl;	// A little dangerous, since the user could delete this
								int idx = ListView_InsertItem(hList, &item);

								ListView_SetItemText(hList, idx, 1, layer->GetName());
								ListView_SetItemText(hList, idx, 2, (char*)name);

								// If size is uninitialized or the same as the last, keep size
								if ((sizeX == -1 && sizeY == -1) || (sizeX == pbbm->bi.Width() && sizeY == pbbm->bi.Height()))
								{
									sizeX = pbbm->bi.Width();
									sizeY = pbbm->bi.Height();
								}
								// Otherwise clear it
								else
								{
									sizeX = sizeY = 0;
								}
							}
							else if (update == kUpdateReplace)
							{
								layer->SetBitmapAssetId(gAssetID, i);

								BitmapInfo info;
								info.SetName(fFileName);
								layer->SetBitmap(&info, i);
							}
							else if (update == kUpdateSetSize)
							{
								layer->SetExportSize(sizeX, sizeY);
							}
						}
					}
				}
			}
		}
	}

	if (update == kUpdateLoadList)
	{
		HWND hButton = GetDlgItem(fDlg, IDC_SET_ALL_BUTTON);
		ComboBox_ResetContent(hCombo);

		// If all bitmaps are the same size, enable resizing
		if (sizeX != -1 && sizeX != 0)
		{
			sizeX = FloorPow2(sizeX);
			sizeY = FloorPow2(sizeY);

			char buf[256];

			while (sizeX >= 4 && sizeY >= 4)
			{
				sprintf(buf, "%d x %d", sizeX, sizeY);
				int idx = ComboBox_AddString(hCombo, buf);
				ComboBox_SetItemData(hCombo, idx, MAKELPARAM(sizeX, sizeY));

				sizeX >>= 1;
				sizeY >>= 1;
			}

			ComboBox_SetCurSel(hCombo, 0);

			EnableWindow(hCombo, TRUE);
			EnableWindow(hButton, TRUE);
		}
		else
		{
			EnableWindow(hCombo, FALSE);
			EnableWindow(hButton, FALSE);
		}
	}

	int autoSizeType = LVSCW_AUTOSIZE;
	if (ListView_GetItemCount(hList) == 0)
		autoSizeType = LVSCW_AUTOSIZE_USEHEADER;

	ListView_SetColumnWidth(hList, 0, autoSizeType);
	ListView_SetColumnWidth(hList, 1, autoSizeType);
	ListView_SetColumnWidth(hList, 2, autoSizeType);
}

void plTextureSearch::IPickReplaceTexture()
{
	fFileName[0] = '\0';

	// if we have the assetman plug-in, then try to use it, unless shift is held down
	MaxAssInterface* maxAssInterface = GetMaxAssInterface();
	if (maxAssInterface && !(GetKeyState(VK_SHIFT) & 0x8000))
	{
		maxAssInterface->OpenBitmapDlg(gAssetID, fFileName, sizeof(fFileName));
	}
	else
	{
		gAssetID.SetEmpty();
		BitmapInfo bi;
		TheManager->SelectFileInput(&bi, GetCOREInterface()->GetMAXHWnd(), _T("Select Bitmap Image File"));
		strcpy(fFileName, bi.Filename());
	}

	if (fFileName[0] == '\0')
	{
		SetDlgItemText(fDlg, IDC_REPLACE_BUTTON, "(none)");
		EnableWindow(GetDlgItem(fDlg, IDC_REPLACE_ALL_BUTTON), FALSE);
	}
	else
	{
		char fname[_MAX_FNAME+_MAX_EXT], ext[_MAX_EXT];
		_splitpath(fFileName, NULL, NULL, fname, ext);
		strcat(fname, ext);

		SetDlgItemText(fDlg, IDC_REPLACE_BUTTON, fname);
		EnableWindow(GetDlgItem(fDlg, IDC_REPLACE_ALL_BUTTON), TRUE);
	}
}
