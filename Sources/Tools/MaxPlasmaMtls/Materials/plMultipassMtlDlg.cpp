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
#include "hsTypes.h"
#include "max.h"
#include "MaxIcon.h"

#include "resource.h"
#include "plMultipassMtl.h"
#include "plMultipassMtlPB.h"
//#include "plMaxLayer.h"
#include "plMultipassMtlDlg.h"

struct LayerID
{
	int layerID;
	int activeID;
};
static LayerID kLayerID[] =
{
	{ IDC_TEX1, IDC_TEXON1 },
	{ IDC_TEX2, IDC_TEXON2 },
	{ IDC_TEX3, IDC_TEXON3 },
	{ IDC_TEX4, IDC_TEXON4 },
	{ IDC_TEX5, IDC_TEXON5 },
	{ IDC_TEX6, IDC_TEXON6 },
	{ IDC_TEX7, IDC_TEXON7 },
	{ IDC_TEX8, IDC_TEXON8 },
	{ IDC_TEX9, IDC_TEXON9 },
	{ IDC_TEX10, IDC_TEXON10 },
};

//-----------------------------------------------------------------------------
// Constructor and destructor
//-----------------------------------------------------------------------------

plMultipassMtlDlg::plMultipassMtlDlg(HWND hwMtlEdit, IMtlParams *imp, plMultipassMtl *m)
{
	fDADMgr.Init(this);

	fhMtlEdit = hwMtlEdit;
	fhRollup = NULL;
	fMtl = m;
	fPBlock = fMtl->GetParamBlockByID(plMultipassMtl::kBlkPasses);
	ip = imp;
	valid = FALSE;

	for (int i = 0; i < NSUBMTLS; i++)
		fLayerBtns[i] = NULL;

	curTime = imp->GetTime();

	fhRollup = ip->AddRollupPage(
		hInstance,
		MAKEINTRESOURCE(IDD_MULTIPASS),
		ForwardProc,
		"Multipass Parameters",
		(LPARAM)this);
}

plMultipassMtlDlg::~plMultipassMtlDlg()
{
	fMtl->SetParamDlg(NULL);
	for (int i = 0; i < NSUBMTLS; i++)
	{
		ReleaseICustButton(fLayerBtns[i]);
		fLayerBtns[i] = NULL; 
	}

	SetWindowLong(fhRollup, GWL_USERDATA, NULL);
	ip->DeleteRollupPage(fhRollup);

	fhRollup = NULL;
}

//-----------------------------------------------------------------------------
// Functions inheirited from ParamDlg
//-----------------------------------------------------------------------------

void plMultipassMtlDlg::SetThing(ReferenceTarget *m)
{
	assert(m->SuperClassID() == MATERIAL_CLASS_ID);
	assert(m->ClassID() == MULTIMTL_CLASS_ID);

	// Bad?
	if (fMtl) 
		fMtl->SetParamDlg(NULL);
	fMtl = (plMultipassMtl *)m;
	if (fMtl)
		fMtl->SetParamDlg(this);

	LoadDialog();
	IUpdateMtlDisplay();
}

void plMultipassMtlDlg::SetTime(TimeValue t)
{
	if (t != curTime)
	{
		curTime = t;
		Interval v;
		fMtl->Update(ip->GetTime(),v);
		LoadDialog();
		IUpdateMtlDisplay();
	}
}

void plMultipassMtlDlg::ReloadDialog()
{
	Interval v;
	fMtl->Update(ip->GetTime(), v);
	LoadDialog();
}

void plMultipassMtlDlg::ActivateDlg(BOOL onOff)
{
}

int plMultipassMtlDlg::FindSubMtlFromHWND(HWND hwnd)
{
	for (int i = 0; i < NSUBMTLS; i++)
	{
		if (hwnd == fLayerBtns[i]->GetHwnd()) 
			return i;
	}

	return -1;
}

BOOL plMultipassMtlDlg::ForwardProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	plMultipassMtlDlg *theDlg;
	if (msg == WM_INITDIALOG)
	{
		theDlg = (plMultipassMtlDlg*)lParam;
		theDlg->fhRollup = hDlg;
		SetWindowLong(hDlg, GWL_USERDATA, lParam);
	}
	else
	{
		if ((theDlg = (plMultipassMtlDlg *)GetWindowLong(hDlg, GWL_USERDATA)) == NULL)
			return FALSE; 
	}

	return theDlg->LayerPanelProc(hDlg,msg,wParam,lParam);
}



//----------------------------------------------------------------------------
// Layer panel processor
//----------------------------------------------------------------------------
BOOL plMultipassMtlDlg::LayerPanelProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int id = LOWORD(wParam);
	int code = HIWORD(wParam);
	int i;

	switch (msg)
	{
	case WM_INITDIALOG:
		{
			int nLayers = fPBlock->GetInt(kMultCount);

			for (i = 0; i < NSUBMTLS; i++)
			{
				fLayerBtns[i] = GetICustButton(GetDlgItem(hDlg, kLayerID[i].layerID));
				fLayerBtns[i]->SetDADMgr(&fDADMgr);

/*				if (i < nLayers)
				{
					SetCheckBox(hDlg, kLayerID[i].activeID, fPBlock->GetInt(kMtlLayerOn, curTime, i));
				}
*/
				fNumTexSpin = SetupIntSpinner(hDlg, IDC_LAYER_SPIN, IDC_LAYER_EDIT, 1, 10, nLayers);
			}

			// TEMP testing
			UpdateLayerDisplay();

			IUpdateMtlDisplay();
		}
		return TRUE;

	case WM_DESTROY:
		for (i = 0; i < NSUBMTLS; i++)
		{
			ReleaseICustButton(fLayerBtns[i]);
			fLayerBtns[i] = NULL;
		}
		ReleaseISpinner(fNumTexSpin);
		fNumTexSpin = NULL;
		break;

	case CC_SPINNER_CHANGE:
		if (id == IDC_LAYER_SPIN && !code)
		{
			IGetSpinnerVal();
			return TRUE;
		}
		break;
	case CC_SPINNER_BUTTONUP:
		if (id == IDC_LAYER_SPIN && code)
		{
			IGetSpinnerVal();
			return TRUE;
		}
		break;

    case WM_COMMAND:  
        {
			for (i = 0; i < NSUBMTLS; i++)
			{
				if (id == kLayerID[i].activeID)
				{
//					fMtl->EnableMap(i,GetCheckBox(hwndDlg, id));
					bool checked = SendMessage(GetDlgItem(hDlg, id), BM_GETCHECK, 0, 0) == BST_CHECKED;
					fPBlock->SetValue(kMultOn, curTime, checked, i);
					return TRUE;
				}
				if (id == kLayerID[i].layerID)
				{
					PostMessage(fhMtlEdit, WM_SUB_MTL_BUTTON, i, (LPARAM)fMtl);
					return TRUE;
				}
			}

		}
//		IUpdateMtlDisplay();
        break;
	}

	return FALSE;
}

void plMultipassMtlDlg::UpdateLayerDisplay() 
{
	int numlayers = fPBlock->GetInt(kMultCount);

	fNumTexSpin->SetValue(numlayers, FALSE);

	int i;
	for (i = 0; i < numlayers && i < NSUBMTLS; i++)
	{
		Mtl *m = fPBlock->GetMtl(kMultPasses, curTime, i);
		TSTR nm;
		if (m) 
			nm = m->GetName();
		else 
			nm = "None";
		fLayerBtns[i]->SetText(nm.data());
		
		ShowWindow(GetDlgItem(fhRollup, kLayerID[i].layerID), SW_SHOW);
		ShowWindow(GetDlgItem(fhRollup, kLayerID[i].activeID), SW_SHOW);
		SetCheckBox(fhRollup, kLayerID[i].activeID, fPBlock->GetInt(kMultOn, curTime, i));	
	}

	for (i = numlayers; i < NSUBMTLS; i++)
	{
		ShowWindow(GetDlgItem(fhRollup, kLayerID[i].layerID), SW_HIDE);
		ShowWindow(GetDlgItem(fhRollup, kLayerID[i].activeID), SW_HIDE);
	}
}

void plMultipassMtlDlg::LoadDialog()
{
	if (fMtl)
	{
		fPBlock = fMtl->GetParamBlockByID(plMultipassMtl::kBlkPasses);

		if (fhRollup)
			UpdateLayerDisplay();
	}
}

bool plMultipassMtlDlg::ISetNumLayers(int num)
{
	if (num >= 1 && num <= NSUBMTLS)
	{
		fMtl->SetNumSubMtls(num);
		UpdateLayerDisplay();
//		IUpdateMtlDisplay();

		return true;
	}

	return false;
}

void plMultipassMtlDlg::IGetSpinnerVal()
{
	ISpinnerControl *spin = GetISpinner(GetDlgItem(fhRollup, IDC_LAYER_SPIN));
	if (!spin)
		return;

	// If new number of layers is invalid, set to current num
	if (!ISetNumLayers(spin->GetIVal()))
	{
		int nLayers = fPBlock->GetInt(kMultCount);
		spin->SetValue(nLayers, FALSE);
	}

	ReleaseISpinner(spin);
}
