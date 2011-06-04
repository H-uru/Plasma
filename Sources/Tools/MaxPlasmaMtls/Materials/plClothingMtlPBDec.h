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
//#include "plClothingMtl.h"
#include "../../AssetMan/PublicInterface/AssManBaseTypes.h"
#include "max.h"

class plClothingEditBox
{
protected:
	int fCtrlID;
	int fPBEditID;

	plClothingEditBox() {};

	void IGetText(IParamBlock2 *pb, HWND hCtrl)
	{
		// Get the previous value
		const char *oldVal = pb->GetStr(fPBEditID);
		if (!oldVal)
			oldVal = "";

		// Get the text from the edit and store it in the paramblock
		int len = GetWindowTextLength(hCtrl)+1;
		if (len > 1)
		{
			char *buf = TRACKED_NEW char[len];
			GetWindowText(hCtrl, buf, len);

			// If the old value is different from the current one, update
			if (strcmp(oldVal, buf))
				pb->SetValue(fPBEditID, 0, buf);

			delete [] buf;
		}
		else
		{
			// If the old value wasn't empty, update
			if (*oldVal != '\0')
				pb->SetValue(fPBEditID, 0, "");
		}
	}

public:
	plClothingEditBox(int ctrlID, int pbEditID) : fCtrlID(ctrlID), fPBEditID(pbEditID) {}

	void UpdateText(IParamBlock2 *pb, HWND hWnd)
	{
		const char *str = pb->GetStr(fPBEditID);
		SetDlgItemText(hWnd, fCtrlID, (str != nil ? str : ""));
	}

	BOOL ProcessMsg(IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		// If this isn't a message we process, return
		if (!(msg == WM_INITDIALOG || msg == WM_DESTROY ||
			 (msg == WM_COMMAND && LOWORD(wParam) == fCtrlID)))
			return FALSE;

		IParamBlock2 *pb = map->GetParamBlock();

		// Initializing the dialog, set the text box
		if (msg == WM_INITDIALOG)
		{
			UpdateText(pb, hWnd);
			return FALSE;
		}

		// Being destroyed, but we may not have gotten the text from the edit box
		// (EN_KILLFOCUS seems to be sent after the window isn't recieving any more messages)
		if (msg == WM_DESTROY)
		{
			IGetText(pb, GetDlgItem(hWnd, fCtrlID));
			return FALSE;
		}

		int notification = HIWORD(wParam);
		HWND hCtrl = (HWND)lParam;

		// Disable Max accelerators when the edit box gets focus
		if (notification == EN_SETFOCUS)
		{
			//plMaxAccelerators::Disable();
			DisableAccelerators();
			return TRUE;
		}
		// The edit control is losing focus, get it's contents
		else if (notification == EN_KILLFOCUS)
		{
			IGetText(pb, hCtrl);
			//plMaxAccelerators::Enable();
			EnableAccelerators();
			return TRUE;
		}

		return FALSE;
	}
};

class ClothingBasicDlgProc;
extern ClothingBasicDlgProc gClothingBasicDlgProc;

static ParamBlockDesc2 gClothingMtlPB
(
	plClothingMtl::kBlkBasic, _T("Clothing"), IDS_PASS_BASIC, GetClothingMtlDesc(), 
	P_AUTO_CONSTRUCT + P_AUTO_UI + P_CALLSETS_ON_LOAD, plClothingMtl::kRefBasic, 

	// UI
	IDD_CLOTHING, IDS_PASS_BASIC, 0, 0, &gClothingBasicDlgProc,

	plClothingMtl::kTileset,	_T("tileset"),	TYPE_INT, 0, 0,
		p_default, 0,
		end,

	plClothingMtl::kTexmap,		_T("texmap"),	TYPE_TEXMAP_TAB, plClothingMtl::kMaxTiles,		0, 0,
		end,

	plClothingMtl::kDescription,	_T("ItemDescription"),	TYPE_STRING,	0, 0,
		p_ui,	TYPE_EDITBOX, IDC_CLOTHING_DESCRIPTION,
		end,

	plClothingMtl::kThumbnail,	_T("Thumbnail"),	TYPE_TEXMAP, 0, 0,
		end,

	plClothingMtl::kLayer,	_T("Layer"),		TYPE_INT, 0, 0,
		p_default, plClothingElement::kLayerTint1,
		end,

	plClothingMtl::kTexmapSkin, _T("SkinLayer"),	TYPE_TEXMAP_TAB, plClothingMtl::kMaxTiles, 0, 0,
		end,

	plClothingMtl::kTexmap2, _T("TintLayer2"),	TYPE_TEXMAP_TAB, plClothingMtl::kMaxTiles, 0, 0,
		end,
		
	plClothingMtl::kDefault, _T("Default"),	TYPE_BOOL, 0, 0,
		p_ui,	TYPE_SINGLECHEKBOX, IDC_CLOTHING_DEFAULT,
		p_default, 0,
		end,

	plClothingMtl::kCustomTextSpecs, _T("TextSpecs"), TYPE_STRING, 0, 0,
		end,

	plClothingMtl::kTexmapBase, _T("BaseLayer"), TYPE_TEXMAP_TAB, plClothingMtl::kMaxTiles, 0, 0,
		end, 
		
	plClothingMtl::kTexmapSkinBlend1, _T("SkinBlend(1)"), TYPE_TEXMAP_TAB, plClothingMtl::kMaxTiles, 0, 0,
		end,

	plClothingMtl::kTexmapSkinBlend2, _T("SkinBlend(2)"), TYPE_TEXMAP_TAB, plClothingMtl::kMaxTiles, 0, 0,
		end,

	plClothingMtl::kTexmapSkinBlend3, _T("SkinBlend(3)"), TYPE_TEXMAP_TAB, plClothingMtl::kMaxTiles, 0, 0,
		end,

	plClothingMtl::kTexmapSkinBlend4, _T("SkinBlend(4)"), TYPE_TEXMAP_TAB, plClothingMtl::kMaxTiles, 0, 0,
		end,

	plClothingMtl::kTexmapSkinBlend5, _T("SkinBlend(5)"), TYPE_TEXMAP_TAB, plClothingMtl::kMaxTiles, 0, 0,
		end,

	plClothingMtl::kTexmapSkinBlend6, _T("SkinBlend(6)"), TYPE_TEXMAP_TAB, plClothingMtl::kMaxTiles, 0, 0,
		end,

	plClothingMtl::kDefaultTint1,	_T("DefaultTint1"),	TYPE_RGBA,	0, 0,
		p_ui, TYPE_COLORSWATCH, IDC_CLOTHING_TINT1,
		p_default,		Color(1,1,1),		
		end,

	plClothingMtl::kDefaultTint2,	_T("DefaultTint2"),	TYPE_RGBA,	0, 0,
		p_ui, TYPE_COLORSWATCH, IDC_CLOTHING_TINT2,
		p_default,		Color(1,1,1),		
		end,

	plClothingMtl::kForcedAcc,	_T("ForcedAcc"), TYPE_STRING, 0, 0,
		p_ui, TYPE_EDITBOX, IDC_CLOTHING_FORCED_ACC,
		end,
		
	end
);

class ClothingBasicDlgProc : public ParamMap2UserDlgProc
{
protected:
	plClothingEditBox fCustomText;
	
public:
	ClothingBasicDlgProc() : fCustomText(IDC_CLOTHING_CUSTOM_TEXT_SPECS, plClothingMtl::kCustomTextSpecs) {}
	~ClothingBasicDlgProc() {}

	void UpdateDisplay(IParamMap2 *pmap)
	{
		HWND hWnd = pmap->GetHWnd();
		IParamBlock2 *pb = pmap->GetParamBlock();

		plClothingMtl *mtl = (plClothingMtl *)pb->GetOwner();
		HWND cbox = NULL;
		plPlasmaMAXLayer *layer;
		PBBitmap *pbbm;
		ICustButton *bmSelectBtn;
		char buff[256];

		// Setup the tiles
		int i, j;
		int layerSet = pb->GetInt(ParamID(plClothingMtl::kLayer));
		int layerIdx = plClothingMtl::LayerToPBIdx[layerSet];
		for (j = 0; j < plClothingMtl::kMaxTiles; j++)
		{
			layer = (plPlasmaMAXLayer *)pb->GetTexmap(ParamID(layerIdx), 0, j);
			pbbm = (layer == nil ? nil : layer->GetPBBitmap());

			bmSelectBtn = GetICustButton(GetDlgItem(hWnd, plClothingMtl::ButtonConstants[j]));
			bmSelectBtn->SetText(pbbm ? (TCHAR*)pbbm->bi.Filename() : "(none)");
			ReleaseICustButton(bmSelectBtn);
		}

		// And the thumbnail...
		layer = (plPlasmaMAXLayer *)pb->GetTexmap(ParamID(plClothingMtl::kThumbnail));
		if (layer == nil)
		{
			layer = TRACKED_NEW plLayerTex;
			pb->SetValue(ParamID(plClothingMtl::kThumbnail), 0, layer);
		}
		pbbm = layer->GetPBBitmap();
		if (pbbm)
		{
			bmSelectBtn = GetICustButton(GetDlgItem(hWnd, IDC_CLOTHING_THUMBNAIL));
			bmSelectBtn->SetText((TCHAR*)pbbm->bi.Filename());
			ReleaseICustButton(bmSelectBtn);
		}

		int setIdx = pb->GetInt(ParamID(plClothingMtl::kTileset));
		ComboBox_SetCurSel(GetDlgItem(hWnd, IDC_CLOTHING_TILESET), setIdx);
		ComboBox_SetCurSel(GetDlgItem(hWnd, IDC_CLOTHING_LAYER), pb->GetInt(ParamID(plClothingMtl::kLayer)));
		mtl->InitTilesets();
		plClothingTileset *tileset = mtl->fTilesets.Get(setIdx);
		for (i = 0; i < tileset->fElements.GetCount(); i++)
		{
			plClothingElement *element = tileset->fElements.Get(i);
			SendMessage(GetDlgItem(hWnd, plClothingMtl::TextConstants[2 * i]), 
						WM_SETTEXT, NULL, (LPARAM)element->fName);
			sprintf(buff, "(%d, %d)", element->fWidth, element->fHeight);
			SendMessage(GetDlgItem(hWnd, plClothingMtl::TextConstants[2 * i + 1]), 
						WM_SETTEXT, NULL, (LPARAM)buff);
			
			ShowWindow(GetDlgItem(hWnd, plClothingMtl::TextConstants[2 * i]), SW_SHOW);	
			ShowWindow(GetDlgItem(hWnd, plClothingMtl::TextConstants[2 * i + 1]), SW_SHOW);	
			ShowWindow(GetDlgItem(hWnd, plClothingMtl::ButtonConstants[i]), SW_SHOW);	
		}
		for (i = tileset->fElements.GetCount(); i < plClothingMtl::kMaxTiles; i++)
		{
			ShowWindow(GetDlgItem(hWnd, plClothingMtl::TextConstants[2 * i]), SW_HIDE);	
			ShowWindow(GetDlgItem(hWnd, plClothingMtl::TextConstants[2 * i + 1]), SW_HIDE);	
			ShowWindow(GetDlgItem(hWnd, plClothingMtl::ButtonConstants[i]), SW_HIDE);
		}
		mtl->ReleaseTilesets();

		fCustomText.UpdateText(pb, hWnd);
	}

	virtual void Update(TimeValue t, Interval& valid, IParamMap2* pmap) { UpdateDisplay(pmap); }

	BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		// Check if it is for our edit box
		if (fCustomText.ProcessMsg(map, hWnd, msg, wParam, lParam))
			return TRUE;

		int id = LOWORD(wParam);
		int code = HIWORD(wParam);

		IParamBlock2 *pb = map->GetParamBlock();
		plClothingMtl *mtl = (plClothingMtl *)pb->GetOwner();
		HWND cbox = NULL;
		plPlasmaMAXLayer *layer;
		PBBitmap *pbbm;
		ICustButton *bmSelectBtn;
		int layerIdx = plClothingMtl::LayerToPBIdx[pb->GetInt(ParamID(plClothingMtl::kLayer))];		 

		switch (msg)
		{
		case WM_INITDIALOG:
			int j;
			mtl->InitTilesets();
			cbox = GetDlgItem(hWnd, IDC_CLOTHING_TILESET);
			for (j = 0; j < mtl->fTilesets.GetCount(); j++)
				SendMessage(cbox, CB_ADDSTRING, 0, (LPARAM)mtl->fTilesets.Get(j)->fName);

			mtl->ReleaseTilesets();

			cbox = GetDlgItem(hWnd, IDC_CLOTHING_LAYER);
			for (j = 0; j < plClothingElement::kLayerMax; j++)
				ComboBox_AddString(cbox, plClothingMtl::LayerStrings[j]);

			return TRUE;

		case WM_COMMAND:
	       	if (id == IDC_CLOTHING_TILESET)
			{
				int setIdx = SendMessage(GetDlgItem(hWnd, id), CB_GETCURSEL, 0, 0);
				pb->SetValue(plClothingMtl::kTileset, t, setIdx);
				return TRUE;
			}

			if (id == IDC_CLOTHING_LAYER)
			{
				
				pb->SetValue(plClothingMtl::kLayer, t, ComboBox_GetCurSel(GetDlgItem(hWnd, id)));
				return TRUE;
			}

			if (id == IDC_CLOTHING_THUMBNAIL)
			{
				layer = (plPlasmaMAXLayer *)pb->GetTexmap(ParamID(plClothingMtl::kThumbnail));
				if (layer == nil)
					return FALSE;

				BitmapInfo bi;
				bi.SetName(layer->GetPBBitmap() == nil ? "" : layer->GetPBBitmap()->bi.Name());

				BOOL selectedNewBitmap = layer->HandleBitmapSelection();
				if (selectedNewBitmap)
				{
					pbbm = layer->GetPBBitmap();
					bmSelectBtn = GetICustButton(GetDlgItem(hWnd, IDC_CLOTHING_THUMBNAIL));
					bmSelectBtn->SetText(pbbm != nil ? (TCHAR*)pbbm->bi.Filename() : "(none)");
					ReleaseICustButton(bmSelectBtn);
				}
				return TRUE;
			}

			int buttonIdx = -1;
			if (id == IDC_CLOTHING_TEXTURE1) buttonIdx = 0;
			else if (id == IDC_CLOTHING_TEXTURE2) buttonIdx = 1;
			else if (id == IDC_CLOTHING_TEXTURE3) buttonIdx = 2;
			else if (id == IDC_CLOTHING_TEXTURE4) buttonIdx = 3;
			
			if (buttonIdx != -1)
			{
				layer = (plPlasmaMAXLayer *)pb->GetTexmap(ParamID(layerIdx), 0, buttonIdx);
				if (layer == nil)
				{ // First time we've set a layer on this spot
					layer = TRACKED_NEW plLayerTex;
					pb->SetValue(ParamID(layerIdx), 0, layer, buttonIdx);
				}

				jvUniqueId oldId;
				layer->GetBitmapAssetId(oldId);

				BitmapInfo bi;
				bi.SetName(layer->GetPBBitmap() == nil ? "" : layer->GetPBBitmap()->bi.Name());

				BOOL selectedNewBitmap = layer->HandleBitmapSelection();
				if (selectedNewBitmap)
				{
					// Check if it's ok, and undo if not.
					hsBool choiceOk = true;

					pbbm = layer->GetPBBitmap();
					if (pbbm != nil)
					{
						mtl->InitTilesets();

						plClothingTileset *tileset = mtl->fTilesets.Get(pb->GetInt(plClothingMtl::kTileset));
						plClothingElement *element = tileset->fElements.Get(buttonIdx);
						float targRatio = (float)element->fWidth / (float)element->fHeight;
						float ratio = (float)pbbm->bi.Width() / (float)pbbm->bi.Height();

						if (targRatio != ratio)
						{
							choiceOk = false;
							hsMessageBox("That image's width/height ratio does not match the one for this tile. "
										 "Restoring the old selection.", "Invalid image", hsMessageBoxNormal);
						}
						else if (pbbm->bi.Width() < element->fWidth)
						{
							choiceOk = false;
							hsMessageBox("The chosen image is too small for that tile slot. "
										 "Restoring the old selection.", "Invalid image", hsMessageBoxNormal);
						}

						mtl->ReleaseTilesets();
					}
					if (!choiceOk)
					{
						layer->SetBitmapAssetId(oldId);
						layer->SetBitmap(&bi);
					}
					else
					{
						bmSelectBtn = GetICustButton(GetDlgItem(hWnd, plClothingMtl::ButtonConstants[buttonIdx]));
						bmSelectBtn->SetText(pbbm != nil ? (TCHAR*)pbbm->bi.Filename() : "(none)");
						ReleaseICustButton(bmSelectBtn);
					}
				}
			}
			return TRUE;
		}

		return FALSE;
	}

	void DeleteThis() {}
};
static ClothingBasicDlgProc gClothingBasicDlgProc;
