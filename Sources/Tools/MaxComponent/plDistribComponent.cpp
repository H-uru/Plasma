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

#include "hsWindows.h"
#include <commdlg.h>

#include "max.h"
#include "meshdlib.h" 
#include "stdmat.h"
#include "bmmlib.h"
#include "resource.h"
#include "plComponent.h"
#include "plComponentReg.h"
#include "../MaxMain/plPlasmaRefMsgs.h"

#include "../MaxMain/plMaxNode.h"
#include "../MaxExport/plExportProgressBar.h"
#include "../MaxExport/plExportErrorMsg.h"

#include "../MaxPlasmaMtls/Layers/plLayerTex.h"

#include "hsTypes.h"

#include "../MaxConvert/plDistributor.h"
#include "../MaxConvert/plDistTree.h"

#include "plDistribComponent.h"

void DummyCodeIncludeFuncDistrib()
{
	extern void DummyCodeIncludeFuncDistrib_old();
	DummyCodeIncludeFuncDistrib_old();
}

struct plIsoTypeStringValPair
{
	plDistributor::IsoType			fValue;
	const char*						fString;
};

static const int kNumSeparations = 4;
static plIsoTypeStringValPair kSeparationStrings[kNumSeparations] =
{
	{ plDistributor::kIsoNone, "None" },
	{ plDistributor::kIsoLow, "Low" },
	{ plDistributor::kIsoMedium, "Medium" },
	{ plDistributor::kIsoHigh, "High" }
};

struct plConformTypeStringValPair
{
	plDistributor::ConformType		fValue;
	const char*						fString;
};

static const int kNumConforms = 5;
static plConformTypeStringValPair kConformStrings[kNumConforms] =
{
	{ plDistributor::kConformNone, "None" },
	{ plDistributor::kConformCheck, "Check" },
	{ plDistributor::kConformAll, "All Verts" },
	{ plDistributor::kConformHeight, "Bottom" },
	{ plDistributor::kConformBase, "Base" }
};

struct plColorChanStringValPair
{
	plDistributor::ColorChan		fValue;
	const char*						fString;
};

static const int kNumColorChanOptions = 13;
static plColorChanStringValPair kProbColorChanStrings[kNumColorChanOptions] =
{
	{ plDistributor::kRed, "Red" },
	{ plDistributor::kGreen, "Green" },
	{ plDistributor::kBlue, "Blue" },
	{ plDistributor::kAlpha, "Alpha" },
	{ plDistributor::kAverageRedGreen, "Avg(R,G)" },
	{ plDistributor::kAverageRedGreenTimesAlpha, "Avg(R,G)xA" },
	{ plDistributor::kAverage, "Avg(R,G,B)" },
	{ plDistributor::kAverageTimesAlpha, "Avg(R,G,B)xA" },
	{ plDistributor::kMax, "Max(R,G,B)" },
	{ plDistributor::kMaxColor, "Max(R,G,B)" },
	{ plDistributor::kMaxColorTimesAlpha, "Max(R,G,B)xA" },
	{ plDistributor::kMaxRedGreen, "Max(R,G)" },
	{ plDistributor::kMaxRedGreenTimesAlpha, "Max(R,G)xA" }
};

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
#define WM_ROLLOUT_OPEN WM_USER+1

class plRollupMgrProc : public ParamMap2UserDlgProc
{
public:
	plRollupMgrProc(int iRoll) : fRollup(iRoll), fGotState(false) {}
protected:
	int		fIdx;
	BOOL	fGotState;
	int		fRollup;

	BOOL IHandleRollupState(IParamMap2* map, HWND hWnd, UINT msg)
	{
		return false;

		char buff[256];
		IRollupWindow *rollup = GetCOREInterface()->GetCommandPanelRollup();
		sprintf(buff, "%d\t%x\t%x\n", fRollup, rollup->GetPanelIndex(hWnd), msg);
		hsStatusMessage(buff);

		if( msg == 0x18 )
		{

		if( msg == WM_CLOSE )
		{
			fGotState = false;
		}
		else
		if( !fGotState )
		{
			fGotState = ISetRollupState(map, hWnd);
		}
		else
		{
			IRecordRollupState(map, hWnd);
		}

		}
		return FALSE;
	}

	void IRecordRollupState(IParamMap2* map, HWND hWnd)
	{
		IRollupWindow *rollup = GetCOREInterface()->GetCommandPanelRollup();
		int idx = rollup->GetPanelIndex(hWnd);
		if( idx >= 0 )
		{
			int state = map->GetParamBlock()->GetInt(plDistribComponent::kRollupState);
			if( rollup->IsPanelOpen(idx) )
				state |= (1 << fRollup);
			else
				state &= ~(1 << fRollup);
			map->GetParamBlock()->SetValue(plDistribComponent::kRollupState, TimeValue(0), state);
		}

	}
	BOOL ISetRollupState(IParamMap2* map, HWND hWnd)
	{
		IRollupWindow *rollup = GetCOREInterface()->GetCommandPanelRollup();
		int idx = rollup->GetPanelIndex(hWnd);
		if( idx >= 0 )
		{
			int state = map->GetParamBlock()->GetInt(plDistribComponent::kRollupState);
			rollup->SetPanelOpen(idx, 0 != (state & (1 << fRollup)));

			return true;
		}
		return false;
	}
};

class plDistCompNilProc : public plRollupMgrProc
{
public:
	plDistCompNilProc(int iRoll) : plRollupMgrProc(iRoll) {}

	BOOL DlgProc(TimeValue t, IParamMap2* map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		IHandleRollupState(map, hWnd, msg);
		return FALSE;
	}
	void DeleteThis() {}
};

class plDistCompActionProc : public plRollupMgrProc
{
public:
	plDistCompActionProc() : plRollupMgrProc(plDistribComponent::kRollAction) {}

	BOOL DlgProc(TimeValue t, IParamMap2* map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		IHandleRollupState(map, hWnd, msg);

		HWND cbox = NULL;
		switch (msg)
		{
		case WM_INITDIALOG:
			PostMessage(hWnd, WM_ROLLOUT_OPEN, 0, 0);
			break;

		case WM_COMMAND:
			if( (HIWORD(wParam) == BN_CLICKED) && (LOWORD(wParam) == IDC_DISTRIB_CLEAR) )
			{
				plDistribComponent* dc = (plDistribComponent*)map->GetParamBlock()->GetOwner();
				dc->Clear();

				return TRUE;
			}
			if( (HIWORD(wParam) == BN_CLICKED) && (LOWORD(wParam) == IDC_DISTRIB_PREVIEW) )
			{
				plDistribComponent* dc = (plDistribComponent*)map->GetParamBlock()->GetOwner();
				dc->Preview();

				return TRUE;
			}
		}
		return FALSE;
	}
	void DeleteThis() {}
};
static plDistCompActionProc gDistCompActionProc;

class plDistCompSpaceProc : public plRollupMgrProc
{
private:
public:
	plDistCompSpaceProc() : plRollupMgrProc(plDistribComponent::kRollSpacing) {}

	BOOL DlgProc(TimeValue t, IParamMap2* map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		IHandleRollupState(map, hWnd, msg);

		HWND cbox = NULL;
		switch (msg)
		{

		case WM_INITDIALOG:
			PostMessage(hWnd, WM_ROLLOUT_OPEN, 0, 0);

			cbox = GetDlgItem(hWnd, IDC_COMP_DISTRIB_SEPARATION);
			int i;
			for( i = 0; i < kNumSeparations; i++ )
			{
				SendMessage(cbox, CB_ADDSTRING, 0, (LPARAM)kSeparationStrings[i].fString);
			}
			SendMessage(cbox, CB_SETCURSEL, map->GetParamBlock()->GetInt(plDistribComponent::kSeparation), 0);

			return TRUE;
		case WM_COMMAND:
			switch( LOWORD(wParam) )
			{
			case IDC_COMP_DISTRIB_SEPARATION:
				{
					map->GetParamBlock()->SetValue(plDistribComponent::kSeparation, t, SendMessage(GetDlgItem(hWnd, LOWORD(wParam)), CB_GETCURSEL, 0, 0));
					return TRUE;
				}
				break;
			}
			break;
		}
		return FALSE;
	}
	void DeleteThis() {}
};
static plDistCompSpaceProc gDistCompSpaceProc;

class plDistCompConformProc : public plRollupMgrProc
{
private:
public:
	plDistCompConformProc() : plRollupMgrProc(plDistribComponent::kRollConform) {}
	BOOL DlgProc(TimeValue t, IParamMap2* map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		IHandleRollupState(map, hWnd, msg);

		HWND cbox = NULL;
		switch (msg)
		{

		case WM_INITDIALOG:
			PostMessage(hWnd, WM_ROLLOUT_OPEN, 0, 0);

			cbox = GetDlgItem(hWnd, IDC_COMP_DISTRIB_CONFORMTYPE);
			int i;
			for( i = 0; i < kNumConforms; i++ )
			{
				SendMessage(cbox, CB_ADDSTRING, 0, (LPARAM)kConformStrings[i].fString);
			}
			SendMessage(cbox, CB_SETCURSEL, map->GetParamBlock()->GetInt(plDistribComponent::kConformType), 0);

			return TRUE;
		case WM_COMMAND:
			switch( LOWORD(wParam) )
			{
			case IDC_COMP_DISTRIB_CONFORMTYPE:
				{
					map->GetParamBlock()->SetValue(plDistribComponent::kConformType, t, SendMessage(GetDlgItem(hWnd, LOWORD(wParam)), CB_GETCURSEL, 0, 0));
					return TRUE;
				}
				break;
			}
			break;
		}

		return FALSE;
	}
	void DeleteThis() {}
};
static plDistCompConformProc gDistCompConformProc;

class plDistCompScaleProc : public plRollupMgrProc
{
private:
	void ISetupScaleLock(IParamMap2* map, BOOL onInit=false)
	{
		IParamBlock2 *pb = map->GetParamBlock();
		if( pb->GetInt(plDistribComponent::kLockScaleXYZ) )
		{
			map->Enable(plDistribComponent::kLockScaleXY, FALSE);

			map->Enable(plDistribComponent::kScaleLoY, FALSE);
			map->Enable(plDistribComponent::kScaleHiY, FALSE);

			map->Enable(plDistribComponent::kScaleLoZ, FALSE);
			map->Enable(plDistribComponent::kScaleHiZ, FALSE);
		}
		else if( pb->GetInt(plDistribComponent::kLockScaleXY) )
		{
			map->Enable(plDistribComponent::kLockScaleXY, TRUE);

			map->Enable(plDistribComponent::kScaleLoY, FALSE);
			map->Enable(plDistribComponent::kScaleHiY, FALSE);

			map->Enable(plDistribComponent::kScaleLoZ, TRUE);
			map->Enable(plDistribComponent::kScaleHiZ, TRUE);
		}
		else
		{
			map->Enable(plDistribComponent::kLockScaleXY, TRUE);

			map->Enable(plDistribComponent::kScaleLoY, TRUE);
			map->Enable(plDistribComponent::kScaleHiY, TRUE);

			map->Enable(plDistribComponent::kScaleLoZ, TRUE);
			map->Enable(plDistribComponent::kScaleHiZ, TRUE);
		}
		if( onInit )
		{
			map->SetTooltip(plDistribComponent::kLockScaleXY, TRUE, "Lock scale in X and Y" );
			map->SetTooltip(plDistribComponent::kLockScaleXYZ, TRUE, "Lock scale in X, Y and Z (uniform scale)" );
		}
	}
public:
	plDistCompScaleProc() : plRollupMgrProc(plDistribComponent::kRollScale) {}

	BOOL DlgProc(TimeValue t, IParamMap2* map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		IHandleRollupState(map, hWnd, msg);

		HWND cbox = NULL;
		switch (msg)
		{

		case WM_INITDIALOG:
			PostMessage(hWnd, WM_ROLLOUT_OPEN, 0, 0);

			ISetupScaleLock(map, true);

			return TRUE;
		case WM_COMMAND:
			switch( LOWORD(wParam) )
			{
				case IDC_COMP_DISTRIB_LOCKSCALEXY:
				case IDC_COMP_DISTRIB_LOCKSCALEXYZ:
					ISetupScaleLock(map, false);
					return TRUE;
			}
			break;
		}

		return FALSE;
	}
	void DeleteThis() {}
};
static plDistCompScaleProc gDistCompScaleProc;

class plDistCompBitmapProc : public plRollupMgrProc
{
private:
public:
	plDistCompBitmapProc() : plRollupMgrProc(plDistribComponent::kRollBitmap) {}
	BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		IHandleRollupState(map, hWnd, msg);

		HWND cbox = NULL;
		switch (msg)
		{
		case WM_INITDIALOG:
			PostMessage(hWnd, WM_ROLLOUT_OPEN, 0, 0);

			cbox = GetDlgItem(hWnd, IDC_COMP_DISTRIB_PROBCOLORCHAN);
			int i;
			for( i = 0; i < kNumColorChanOptions; i++ )
			{
				SendMessage(cbox, CB_ADDSTRING, 0, (LPARAM)kProbColorChanStrings[i].fString);
			}
			SendMessage(cbox, CB_SETCURSEL, map->GetParamBlock()->GetInt(plDistribComponent::kProbColorChan), 0);
			return TRUE;

		case WM_COMMAND:
			switch( LOWORD(wParam) )
			{
				case IDC_COMP_DISTRIB_PROBCOLORCHAN:
				{
					map->GetParamBlock()->SetValue(plDistribComponent::kProbColorChan, t, SendMessage(GetDlgItem(hWnd, LOWORD(wParam)), CB_GETCURSEL, 0, 0));
					return TRUE;
				}
				break;
			}
			break;
		}
		return FALSE;
	}
	void DeleteThis() {}
};
static plDistCompBitmapProc gDistCompBitmapProc;

class plDistCompFadeProc : public plRollupMgrProc
{
private:
public:
	plDistCompFadeProc() : plRollupMgrProc(plDistribComponent::kRollFade) {}
	BOOL DlgProc(TimeValue t, IParamMap2* map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		IHandleRollupState(map, hWnd, msg);

		HWND cbox = NULL;
		switch (msg)
		{
		case WM_INITDIALOG:
			PostMessage(hWnd, WM_ROLLOUT_OPEN, 0, 0);
			break;

		case WM_COMMAND:
			switch( LOWORD(wParam) )
			{
				case IDC_COMP_DISTRIB_FADEINACTIVE:
				case IDC_COMP_DISTRIB_FADEINTRAN:
				case IDC_COMP_DISTRIB_FADEINOPAQ:
				case IDC_COMP_DISTRIB_FADEOUTTRAN:
				case IDC_COMP_DISTRIB_FADEOUTOPAQ:
				case IDC_COMP_DISTRIB_FADEINTRAN_SPIN:
				case IDC_COMP_DISTRIB_FADEINOPAQ_SPIN:
				case IDC_COMP_DISTRIB_FADEOUTTRAN_SPIN:
				case IDC_COMP_DISTRIB_FADEOUTOPAQ_SPIN:
				{
					plDistribComponent* dc = (plDistribComponent*)map->GetParamBlock()->GetOwner();
					Box3 fade;
					if( !dc->IValidateFade(fade) )
					{
						map->GetParamBlock()->SetValue(plDistribComponent::kFadeInTran, t, fade.Min()[0]);
						map->GetParamBlock()->SetValue(plDistribComponent::kFadeInOpaq, t, fade.Min()[1]);
						map->GetParamBlock()->SetValue(plDistribComponent::kFadeOutTran, t, fade.Max()[0]);
						map->GetParamBlock()->SetValue(plDistribComponent::kFadeOutOpaq, t, fade.Max()[1]);

						map->Invalidate(plDistribComponent::kFadeInTran);
						map->Invalidate(plDistribComponent::kFadeInOpaq);
						map->Invalidate(plDistribComponent::kFadeOutTran);
						map->Invalidate(plDistribComponent::kFadeOutOpaq);
						ShowWindow(hWnd, SW_HIDE);
						ShowWindow(hWnd, SW_SHOW);
					}
					return TRUE;
				}
				break;
			}
			break;

		}
		return FALSE;
	}
	void DeleteThis() {}
};
static plDistCompFadeProc gDistCompFadeProc;



//Max desc stuff necessary below.
CLASS_DESC(plDistribComponent, gDistribCompDesc, "Distributor",  "Distributor", COMP_TYPE_DISTRIBUTOR, DISTRIBUTOR_COMP_CID)

class plDistribCompAccessor : public PBAccessor
{
public:
	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)
	{
		if( id == plDistribComponent::kTemplates )
		{
			plDistribComponent *comp = (plDistribComponent*)owner;
			comp->NotifyDependents(FOREVER, PART_ALL, REFMSG_USER_COMP_REF_CHANGED);
		}
	}
};
plDistribCompAccessor gDistribCompAccessor;

plDistCompNilProc gDistCompTemplatesProc(plDistribComponent::kRollTemplates);
plDistCompNilProc gDistCompOrientProc(plDistribComponent::kRollOrient);
plDistCompNilProc gDistCompAngProbProc(plDistribComponent::kRollAngProb);
plDistCompNilProc gDistCompAltProbProc(plDistribComponent::kRollAltProb);
plDistCompNilProc gDistCompWindProc(plDistribComponent::kRollWind);
plDistCompNilProc gDistCompRandomizeProc(plDistribComponent::kRollRandomize);

//const int kROLLUP_START = 0;
const int kROLLUP_START = APPENDROLL_CLOSED | DONTAUTOCLOSE;

ParamBlockDesc2 gDistributorBk
(	// KLUDGE: not the defined block ID, but kept for backwards compat.
 plComponent::kBlkComp, _T("Distributor"), 0, &gDistribCompDesc, P_AUTO_CONSTRUCT + P_AUTO_UI + P_MULTIMAP, plComponent::kRefComp,

	// Roll Out the red carpet.
	plDistribComponent::kNumRollups,
		plDistribComponent::kRollTemplates, IDD_COMP_DIST_TEMPLATES, IDS_COMP_DIST_TEMPLATES, 0, 0, &gDistCompTemplatesProc,
		plDistribComponent::kRollSpacing, IDD_COMP_DIST_SPACING, IDS_COMP_DIST_SPACING, 0, kROLLUP_START, &gDistCompSpaceProc,
		plDistribComponent::kRollScale, IDD_COMP_DIST_SCALE, IDS_COMP_DIST_SCALE, 0, kROLLUP_START, &gDistCompScaleProc,
		plDistribComponent::kRollOrient, IDD_COMP_DIST_ORIENT, IDS_COMP_DIST_ORIENT, 0, kROLLUP_START, &gDistCompOrientProc,
		plDistribComponent::kRollAngProb, IDD_COMP_DIST_ANGPROB, IDS_COMP_DIST_ANGPROB, 0, kROLLUP_START, &gDistCompAngProbProc,
		plDistribComponent::kRollAltProb, IDD_COMP_DIST_ALTPROB, IDS_COMP_DIST_ALTPROB, 0, kROLLUP_START, &gDistCompAltProbProc,
		plDistribComponent::kRollConform, IDD_COMP_DIST_CONFORM, IDS_COMP_DIST_CONFORM, 0, kROLLUP_START, &gDistCompConformProc,
		plDistribComponent::kRollBitmap, IDD_COMP_DIST_BITMAP, IDS_COMP_DIST_BITMAP, 0, kROLLUP_START, &gDistCompBitmapProc,
		plDistribComponent::kRollFade, IDD_COMP_DIST_FADE, IDS_COMP_DIST_FADE, 0, kROLLUP_START, &gDistCompFadeProc,
		plDistribComponent::kRollWind, IDD_COMP_DIST_WIND, IDS_COMP_DIST_WIND, 0, kROLLUP_START, &gDistCompWindProc,
		plDistribComponent::kRollRandomize, IDD_COMP_DIST_RANDOMIZE, IDS_COMP_DIST_RANDOMIZE, 0, kROLLUP_START, &gDistCompRandomizeProc,
		plDistribComponent::kRollAction, IDD_COMP_DIST_ACTION, IDS_COMP_DIST_ACTION, 0, kROLLUP_START, &gDistCompActionProc,


	// TEMPLATES
	plDistribComponent::kTemplates,	_T("Templates"),	TYPE_INODE_TAB, 0,		P_CAN_CONVERT, 0,
		p_ui,			plDistribComponent::kRollTemplates, TYPE_NODELISTBOX, IDC_LIST_TARGS, IDC_ADD_TARGS, 0, IDC_DEL_TARGS,
		p_classID,		triObjectClassID,
		p_accessor,		&gDistribCompAccessor,
		end,

	// SPACING
	plDistribComponent::kSpacing, _T("Spacing"), TYPE_FLOAT, 	0, 0,	
		p_default, 10.0,
		p_range, 0.1, 100.0,
		p_ui, plDistribComponent::kRollSpacing, TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_DISTRIB_SPACING, IDC_COMP_DISTRIB_SPACING_SPIN, 1.0,
		end,	
	
	plDistribComponent::kSpaceRnd, _T("Space Randomness"), TYPE_FLOAT, 	0, 0,	
		p_default, 50.0,
		p_range, 0.0, 100.0,
		p_ui,	plDistribComponent::kRollSpacing, TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_DISTRIB_SPACERND, IDC_COMP_DISTRIB_SPACERND_SPIN, 1.0,
		end,	

	plDistribComponent::kDensity, _T("Density"), TYPE_FLOAT, 	0, 0,	
		p_default, 100.0,
		p_range, 1., 100.0,
		p_ui,	plDistribComponent::kRollSpacing, TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_DISTRIB_DENSITY, IDC_COMP_DISTRIB_DENSITY_SPIN, 1.0,
		end,	
	
	plDistribComponent::kSeparation,	_T("Separation"),	TYPE_INT,	0, 0,
		p_range, 0, 3,
		p_default, 1,
		end,

	// SCALE
	plDistribComponent::kScaleLoX, _T("ScaleMinX"), TYPE_FLOAT, 	0, 0,	
		p_default, 1.0,
		p_range, 0.1, 10.0,
		p_ui,	plDistribComponent::kRollScale, TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_DISTRIB_SCALELOX, IDC_COMP_DISTRIB_SCALELOX_SPIN, 1.0,
		end,	

	plDistribComponent::kScaleLoY, _T("ScaleMinY"), TYPE_FLOAT, 	0, 0,	
		p_default, 1.0,
		p_range, 0.1, 10.0,
		p_ui,	plDistribComponent::kRollScale, TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_DISTRIB_SCALELOY, IDC_COMP_DISTRIB_SCALELOY_SPIN, 1.0,
		end,	

	plDistribComponent::kScaleLoZ, _T("ScaleMinZ"), TYPE_FLOAT, 	0, 0,	
		p_default, 1.0,
		p_range, 0.1, 10.0,
		p_ui,	plDistribComponent::kRollScale, TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_DISTRIB_SCALELOZ, IDC_COMP_DISTRIB_SCALELOZ_SPIN, 1.0,
		end,	

	plDistribComponent::kScaleHiX, _T("ScaleMaxX"), TYPE_FLOAT, 	0, 0,	
		p_default, 1.0,
		p_range, 0.1, 10.0,
		p_ui,	plDistribComponent::kRollScale, TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_DISTRIB_SCALEHIX, IDC_COMP_DISTRIB_SCALEHIX_SPIN, 1.0,
		end,	

	plDistribComponent::kScaleHiY, _T("ScaleMaxY"), TYPE_FLOAT, 	0, 0,	
		p_default, 1.0,
		p_range, 0.1, 10.0,
		p_ui,	plDistribComponent::kRollScale, TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_DISTRIB_SCALEHIY, IDC_COMP_DISTRIB_SCALEHIY_SPIN, 1.0,
		end,	

	plDistribComponent::kScaleHiZ, _T("ScaleMaxZ"), TYPE_FLOAT, 	0, 0,	
		p_default, 1.0,
		p_range, 0.1, 10.0,
		p_ui,	plDistribComponent::kRollScale, TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_DISTRIB_SCALEHIZ, IDC_COMP_DISTRIB_SCALEHIZ_SPIN, 1.0,
		end,	

	plDistribComponent::kLockScaleXY,  _T("LockScaleXY"), TYPE_BOOL, 		0, 0,
		p_default,	FALSE,
		p_ui,	plDistribComponent::kRollScale, TYPE_SINGLECHEKBOX, IDC_COMP_DISTRIB_LOCKSCALEXY,
		end,

	plDistribComponent::kLockScaleXYZ,  _T("LockScaleXYZ"), TYPE_BOOL, 		0, 0,
		p_default,	FALSE,
		p_ui,	plDistribComponent::kRollScale, TYPE_SINGLECHEKBOX, IDC_COMP_DISTRIB_LOCKSCALEXYZ,
		end,

	// ORIENT
	plDistribComponent::kAlignWgt, _T("Align Weight"), TYPE_FLOAT, 	0, 0,	
		p_default, 50.0,
		p_range, 0.0, 100.0,
		p_ui,	plDistribComponent::kRollOrient, TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_DISTRIB_ALIGNWGT, IDC_COMP_DISTRIB_ALIGNWGT_SPIN, 1.0,
		end,	
	
	plDistribComponent::kPolarRange, _T("Normal Range (Deg)"), TYPE_FLOAT, 	0, 0,	
		p_default, 15.0,
		p_range, 0.0, 180.0,
		p_ui,	plDistribComponent::kRollOrient, TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_DISTRIB_POLARRANGE, IDC_COMP_DISTRIB_POLARRANGE_SPIN, 1.0,
		end,	
	
	plDistribComponent::kAzimuthRange, _T("Normal Range (Deg)"), TYPE_FLOAT, 	0, 0,	
		p_default, 180.0,
		p_range, 0.0, 180.0,
		p_ui,	plDistribComponent::kRollOrient, TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_DISTRIB_AZIMUTHRANGE, IDC_COMP_DISTRIB_AZIMUTHRANGE_SPIN, 1.0,
		end,	
	
	plDistribComponent::kPolarBunch, _T("Normal Bunching"), TYPE_FLOAT, 	0, 0,	
		p_default, 0.0,
		p_range, 0.0, 100.0,
		end,	
	
	// ANGPROB
	plDistribComponent::kAngProbHi, _T("AngProbHi"), TYPE_FLOAT, 	0, 0,	
		p_default, 0.0,
		p_range, 0.0, 180.0,
		p_ui,	plDistribComponent::kRollAngProb, TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_DISTRIB_ANGPROBHI, IDC_COMP_DISTRIB_ANGPROBHI_SPIN, 1.0,
		end,	
	
	plDistribComponent::kAngProbLo, _T("AngProbLo"), TYPE_FLOAT, 	0, 0,	
		p_default, 0.0,
		p_range, 0.0, 180.0,
		p_ui,	plDistribComponent::kRollAngProb, TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_DISTRIB_ANGPROBLO, IDC_COMP_DISTRIB_ANGPROBLO_SPIN, 1.0,
		end,	
	
	plDistribComponent::kAngProbTrans, _T("AngProbTrans"), TYPE_FLOAT, 	0, 0,	
		p_default, 0.0,
		p_range, 0.0, 180.0,
		p_ui,	plDistribComponent::kRollAngProb, TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_DISTRIB_ANGPROBTRANS, IDC_COMP_DISTRIB_ANGPROBTRANS_SPIN, 1.0,
		end,	

	// ALTPROB
	plDistribComponent::kAltProbHi, _T("AltProbHi"), TYPE_FLOAT, 	0, 0,	
		p_default, 0.0,
		p_range, -1000.0, 1000.0,
		p_ui,	plDistribComponent::kRollAltProb, TYPE_SPINNER,	EDITTYPE_FLOAT,	
		IDC_COMP_DISTRIB_ALTPROBHI, IDC_COMP_DISTRIB_ALTPROBHI_SPIN, 1.0,
		end,	
	
	plDistribComponent::kAltProbLo, _T("AltProbLo"), TYPE_FLOAT, 	0, 0,	
		p_default, 0.0,
		p_range, -1000.0, 1000.0,
		p_ui,	plDistribComponent::kRollAltProb, TYPE_SPINNER,	EDITTYPE_FLOAT,	
		IDC_COMP_DISTRIB_ALTPROBLO, IDC_COMP_DISTRIB_ALTPROBLO_SPIN, 1.0,
		end,	
	
	plDistribComponent::kAltProbTrans, _T("AltProbTrans"), TYPE_FLOAT, 	0, 0,	
		p_default, 0.0,
		p_range, 0.0, 1000.0,
		p_ui,	plDistribComponent::kRollAltProb, TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_DISTRIB_ALTPROBTRANS, IDC_COMP_DISTRIB_ALTPROBTRANS_SPIN, 1.0,
		end,	

	// CONFORM
	plDistribComponent::kConformType,	_T("ConformType"),	TYPE_INT,	0, 0,
		p_default, 0,
		end,

	plDistribComponent::kConformMax, _T("ConformMax"), TYPE_FLOAT, 	0, 0,	
		p_default, 0.0,
		p_range, 0.0, 1000.0,
		p_ui,	plDistribComponent::kRollConform, TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_DISTRIB_CONFORMMAX, IDC_COMP_DISTRIB_CONFORMMAX_SPIN, 1.0,
		end,	

	plDistribComponent::kOffsetMin, _T("OffsetMin"), TYPE_FLOAT, 	0, 0,	
		p_default, 0.0,
		p_range, -1000.0, 1000.0,
		p_ui,	plDistribComponent::kRollConform, TYPE_SPINNER,	EDITTYPE_FLOAT,	
		IDC_COMP_DISTRIB_OFFSETMIN, IDC_COMP_DISTRIB_OFFSETMIN_SPIN, 1.0,
		end,	

	plDistribComponent::kOffsetMax, _T("OffsetMax"), TYPE_FLOAT, 	0, 0,	
		p_default, 0.0,
		p_range, -1000.0, 1000.0,
		p_ui,	plDistribComponent::kRollConform, TYPE_SPINNER,	EDITTYPE_FLOAT,	
		IDC_COMP_DISTRIB_OFFSETMAX, IDC_COMP_DISTRIB_OFFSETMAX_SPIN, 1.0,
		end,	

	// BITMAP
	plDistribComponent::kProbTexmap,			_T("ProbTexmap"),	TYPE_TEXMAP,		0, 0,
		p_ui, plDistribComponent::kRollBitmap, TYPE_TEXMAPBUTTON, IDC_COMP_DISTRIB_PROBTEXMAP,
		end,

	plDistribComponent::kProbColorChan,	_T("ProbColorChan"),	TYPE_INT,	0, 0,
		p_default, 0,
		end,

	plDistribComponent::kRemapFromLo, _T("RemapFromLo"), TYPE_FLOAT, 	0, 0,	
		p_default, 0.0,
		p_range, 0.0, 255.0,
		p_ui,	plDistribComponent::kRollBitmap, TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_DISTRIB_REMAPFROMLO, IDC_COMP_DISTRIB_REMAPFROMLO_SPIN, 1.0,
		end,	

	plDistribComponent::kRemapFromHi, _T("RemapFromHi"), TYPE_FLOAT, 	0, 0,	
		p_default, 255.0,
		p_range, 0.0, 255.0,
		p_ui,	plDistribComponent::kRollBitmap, TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_DISTRIB_REMAPFROMHI, IDC_COMP_DISTRIB_REMAPFROMHI_SPIN, 1.0,
		end,	

	plDistribComponent::kRemapToLo, _T("RemapToLo"), TYPE_FLOAT, 	0, 0,	
		p_default, 0.0,
		p_range, 0.0, 255.0,
		p_ui,	plDistribComponent::kRollBitmap, TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_DISTRIB_REMAPTOLO, IDC_COMP_DISTRIB_REMAPTOLO_SPIN, 1.0,
		end,	

	plDistribComponent::kRemapToHi, _T("RemapToHi"), TYPE_FLOAT, 	0, 0,	
		p_default, 255.0,
		p_range, 0.0, 255.0,
		p_ui,	plDistribComponent::kRollBitmap, TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_DISTRIB_REMAPTOHI, IDC_COMP_DISTRIB_REMAPTOHI_SPIN, 1.0,
		end,	

	// FADE
	plDistribComponent::kFadeInTran, _T("FadeInTran"), TYPE_FLOAT, 	0, 0,	
		p_default, 0.0,
		p_range, 0.0, 1000.0,
		p_ui,	plDistribComponent::kRollFade, TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_DISTRIB_FADEINTRAN, IDC_COMP_DISTRIB_FADEINTRAN_SPIN, 1.0,
		end,	
	
	plDistribComponent::kFadeInOpaq, _T("FadeInOpaq"), TYPE_FLOAT, 	0, 0,	
		p_default, 0.0,
		p_range, 0.0, 1000.0,
		p_ui,	plDistribComponent::kRollFade, TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_DISTRIB_FADEINOPAQ, IDC_COMP_DISTRIB_FADEINOPAQ_SPIN, 1.0,
		end,	
	
	plDistribComponent::kFadeOutTran, _T("FadeOutTran"), TYPE_FLOAT, 	0, 0,	
		p_default, 0.0,
		p_range, 0.0, 1000.0,
		p_ui,	plDistribComponent::kRollFade, TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_DISTRIB_FADEOUTTRAN, IDC_COMP_DISTRIB_FADEOUTTRAN_SPIN, 1.0,
		end,	
	
	plDistribComponent::kFadeOutOpaq, _T("FadeOutOpaq"), TYPE_FLOAT, 	0, 0,	
		p_default, 0.0,
		p_range, 0.0, 1000.0,
		p_ui,	plDistribComponent::kRollFade, TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_DISTRIB_FADEOUTOPAQ, IDC_COMP_DISTRIB_FADEOUTOPAQ_SPIN, 1.0,
		end,	
	
	plDistribComponent::kFadeInActive,  _T("FadeInActive"), TYPE_BOOL, 		0, 0,
		p_default,	FALSE,
		p_ui,	plDistribComponent::kRollFade, TYPE_SINGLECHEKBOX, IDC_COMP_DISTRIB_FADEINACTIVE,
		p_enable_ctrls,		2, plDistribComponent::kFadeInTran, plDistribComponent::kFadeInOpaq,
		end,

	// WIND
	plDistribComponent::kWindBone, _T("WindBone"),	TYPE_INODE,		0, 0,
		p_ui,	plDistribComponent::kRollWind, TYPE_PICKNODEBUTTON, IDC_COMP_DISTRIB_WINDBONE,
		p_prompt, IDS_COMP_CLUSTER_CHOSE_WINDBONE,
		end,

	plDistribComponent::kWindBoneActive,  _T("WindBoneActive"), TYPE_BOOL, 		0, 0,
		p_default,	FALSE,
		p_ui,	plDistribComponent::kRollWind, TYPE_SINGLECHEKBOX, IDC_COMP_DISTRIB_WINDBONEACTIVE,
		p_enable_ctrls,		1, plDistribComponent::kWindBone,
		end,

	// RANDOMIZE
	plDistribComponent::kSeedLocked,  _T("Locked"), TYPE_BOOL, 		0, 0,
		p_default,	FALSE,
		p_ui,	plDistribComponent::kRollRandomize, TYPE_SINGLECHEKBOX, IDC_COMP_DISTRIB_LOCK,
		end,

	plDistribComponent::kSeed,	_T("Seed"),	TYPE_INT,	0, 0,
		p_default, 1,
		p_ui,	plDistribComponent::kRollRandomize, TYPE_SPINNER,	EDITTYPE_INT,	
		IDC_COMP_DISTRIB_SEED, IDC_COMP_DISTRIB_SEED_SPIN, 1.0,
		p_range, 0x80000000, 0x7fffffff,
		end,

	plDistribComponent::kNextSeed,	_T("NextSeed"),	TYPE_INT,	0, 0,
		p_default, 1,
		end,

	// REPLICANTS (non-input)
	plDistribComponent::kReplicants,	_T("Replicants"),	TYPE_INODE_TAB, 0,		0, 0,
		p_accessor,		&gDistribCompAccessor,
		end,

	plDistribComponent::kRollupState,	_T("RollupState"),	TYPE_INT,	0, 0,
		p_default, 0,
		end,

	plDistribComponent::kFaceNormals,  _T("FaceNormals"), TYPE_BOOL, 		0, 0,
		p_default,	FALSE,
		p_ui,	plDistribComponent::kRollOrient, TYPE_SINGLECHEKBOX, IDC_COMP_DISTRIB_FACENORMALS,
		end,

	end
);

plDistribComponent::plDistribComponent()
{
	fClassDesc = &gDistribCompDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

hsBool plDistribComponent::SetupProperties(plMaxNode* node, plErrorMsg* pErrMsg)
{
	int numReps = fCompPB->Count(kTemplates);
	int i;
	for( i = 0; i < numReps; i++ )
	{
#if 0
		plMaxNodeBase* temp = (plMaxNodeBase*)fCompPB->GetINode(kTemplates, TimeValue(0), i);
		if( temp )
			temp->SetCanConvert(false);
#else
		plMaxNodeBase* temp = (plMaxNodeBase*)fCompPB->GetINode(kTemplates, TimeValue(0), i);
		if( temp )
		{
			temp->SetDrawable(false);
			temp->SetPhysical(false);
			temp->SetForceLocal(true);
			temp->SetRunTimeLight(true);
			temp->SetNoPreShade(true);
//			temp->SetAlphaTestHigh(true);
		}
#endif
	}

	return true;
}

BOOL plDistribComponent::Distribute(plDistribInstTab& replicants, plErrorMsg* pErrMsg, plExportProgressBar& bar, plDistTree* distTree)
{
	plExportErrorMsg exportErrorMsg;
	if( !pErrMsg )
		pErrMsg = &exportErrorMsg;

	BOOL retVal = true;

	GetCOREInterface()->DisableSceneRedraw();

	Clear();

	plDistributor distrib;

	distrib.SetTheInterface(GetCOREInterface());

	// Spacing
	distrib.SetSpacing(fCompPB->GetFloat(kSpacing));

	distrib.SetSpacingRange(fCompPB->GetFloat(kSpaceRnd) * 0.01f * distrib.GetSpacing() * 0.5f);

	distrib.SetOverallProb(fCompPB->GetFloat(kDensity));

	distrib.SetIsolation(GetIsolation());

	distrib.SetDistTree(distTree);

	// Scale
	Point3 scaleLo(fCompPB->GetFloat(kScaleLoX), fCompPB->GetFloat(kScaleLoY), fCompPB->GetFloat(kScaleLoZ));
	Point3 scaleHi(fCompPB->GetFloat(kScaleHiX), fCompPB->GetFloat(kScaleHiY), fCompPB->GetFloat(kScaleHiZ));
	distrib.SetScaleRange(scaleLo, scaleHi);
	ULONG scaleLock = plDistributor::kLockNone;
	if( fCompPB->GetInt(kLockScaleXYZ) )
		scaleLock = plDistributor::kLockX | plDistributor::kLockY | plDistributor::kLockZ;
	else if( fCompPB->GetInt(kLockScaleXY) )
		scaleLock = plDistributor::kLockX | plDistributor::kLockY;
	distrib.SetScaleLock(scaleLock);

	// Orient
	// UI offers alignment to surface, distributor wants alignment to UP.
	distrib.SetAlignmentWeight(100.f - fCompPB->GetFloat(kAlignWgt));

	distrib.SetPolarRange(fCompPB->GetFloat(kPolarRange));

	distrib.SetAzimuthRange(fCompPB->GetFloat(kAzimuthRange));

	distrib.SetPolarBunch(fCompPB->GetFloat(kPolarBunch));

	distrib.SetFaceNormals(fCompPB->GetInt(kFaceNormals));

	// Angle prob
	distrib.SetAngleProbHi(fCompPB->GetFloat(kAngProbHi));
	distrib.SetAngleProbLo(fCompPB->GetFloat(kAngProbLo));
	distrib.SetAngleProbTransition(fCompPB->GetFloat(kAngProbTrans));

	// Altitude prob
	float minAlt = fCompPB->GetFloat(kAltProbLo);
	float maxAlt = fCompPB->GetFloat(kAltProbHi);
	if( maxAlt < minAlt )
	{
		float t = maxAlt;
		maxAlt = minAlt;
		minAlt = t;
	}
	distrib.SetMinAltitude(minAlt);
	distrib.SetMaxAltitude(maxAlt);
	distrib.SetAltitudeTransition(fCompPB->GetFloat(kAltProbTrans));

	// Conformity
	distrib.SetConformity(GetConformity());
	distrib.SetMaxConform(fCompPB->GetFloat(kConformMax));
	distrib.SetMinOffset(fCompPB->GetFloat(kOffsetMin));
	distrib.SetMaxOffset(fCompPB->GetFloat(kOffsetMax));

	// Bitmap
	ISetProbTexmap(distrib);

	distrib.SetProbabilityChan(GetProbabilityChan());

	distrib.SetProbabilityRemapFromLo(fCompPB->GetFloat(kRemapFromLo));
	distrib.SetProbabilityRemapFromHi(fCompPB->GetFloat(kRemapFromHi));
	distrib.SetProbabilityRemapToLo(fCompPB->GetFloat(kRemapToLo));
	distrib.SetProbabilityRemapToHi(fCompPB->GetFloat(kRemapToHi));

	// Fade
	distrib.SetFade(GetFade());

	// Wind
	if( fCompPB->GetInt(kWindBoneActive) )
		distrib.SetBone(fCompPB->GetINode(kWindBone));

	distrib.SetRigid(!IsFlexible());

	// Randomization
	if( !fCompPB->GetInt(kSeedLocked) )
		fCompPB->SetValue(kSeed, TimeValue(0), fCompPB->GetInt(kNextSeed));

	distrib.SetRandSeed(fCompPB->GetInt(kSeed));

	int numReps = fCompPB->Count(kTemplates);
	int i;
	for( i = 0; i < numReps; i++ )
	{
		INode* temp = fCompPB->GetINode(kTemplates, TimeValue(0), i);
		if( temp )
			distrib.AddReplicateNode(temp);
	}

	try {
		pErrMsg->Set(!distrib.GetNumReplicateNodes(), GetINode()->GetName(), "Distributor %s has nothing to replicate", GetINode()->GetName()).CheckAndAsk();

		int numTarg = NumTargets();
		for( i = 0; i < numTarg; i++ )
		{
			if( GetTarget(i) )
			{
				if( !distrib.Distribute(GetTarget(i), replicants, fDistCache, bar) )
				{
					break;
				}
			}
		}
	}

	catch (...) 
	{
		if( pErrMsg->IsBogus() )
			pErrMsg->Show();
		retVal = false;
	}

	fCompPB->SetValue(kNextSeed, TimeValue(0), int(distrib.GetRandSeed()));

	BOOL redrawDissed = GetCOREInterface()->IsSceneRedrawDisabled();

	GetCOREInterface()->EnableSceneRedraw();

	GetCOREInterface()->ForceCompleteRedraw(FALSE);

	return retVal;
}

void plDistribComponent::Done()
{
	int i;
	for( i = 0; i < fDistCache.Count(); i++ )
	{
		delete fDistCache[i].fMesh;
	}
	fDistCache.ZeroCount();
}

BOOL plDistribComponent::IsFlexible() const
{
	int numReps = fCompPB->Count(kTemplates);
	int i;
	for( i = 0; i < numReps; i++ )
	{
		plMaxNode* temp = (plMaxNode*)fCompPB->GetINode(kTemplates, TimeValue(0), i);
		if( temp )
		{
			if( temp->GetFlexibility()[0] > 0 )
				return true;
		}
	}
	return false;
}

void plDistribComponent::ISetProbTexmap(plDistributor& distrib)
{
	distrib.SetProbabilityBitmapTex(nil);

	Texmap* tex = fCompPB->GetTexmap(kProbTexmap);
	if( tex )
	{
		BitmapTex* bmt = GetIBitmapTextInterface(tex);
		if( bmt )
			distrib.SetProbabilityBitmapTex(bmt);
		else if( tex->ClassID() == LAYER_TEX_CLASS_ID )
			distrib.SetProbabilityLayerTex((plLayerTex*)tex);
	}
}

float plDistribComponent::GetIsoPriority() const
{
	float pri = fCompPB->GetFloat(kSpacing);
	pri *= pri;
	pri *= fCompPB->GetFloat(kDensity) * 0.01f;

	return pri;
}

plDistributor::ColorChan plDistribComponent::GetProbabilityChan() const
{
	return kProbColorChanStrings[fCompPB->GetInt(kProbColorChan)].fValue;
}

plDistributor::ConformType plDistribComponent::GetConformity() const
{
	return kConformStrings[fCompPB->GetInt(kConformType)].fValue;
}

plDistributor::IsoType plDistribComponent::GetIsolation() const
{
	return kSeparationStrings[fCompPB->GetInt(kSeparation)].fValue;
}

void plDistribComponent::Clear()
{
	GetCOREInterface()->DisableSceneRedraw();
	// First, clear out any we've already done.
	int numReps = fCompPB->Count(kReplicants);
	int i;
	for( i = 0; i < numReps; i++ )
	{
		INode* rep = fCompPB->GetINode(kReplicants, TimeValue(0), i);
		if( rep )
			rep->Delete(TimeValue(0), true);
	}
	fCompPB->ZeroCount(kReplicants);

	GetCOREInterface()->EnableSceneRedraw();
	GetCOREInterface()->ForceCompleteRedraw(FALSE);
}

void plDistribComponent::Preview()
{
	if( !NumTargets() )
		return;

	GetCOREInterface()->DisableSceneRedraw();

	plDistribInstTab replicants;

	plExportProgressBar bar;
	bar.Start("Preview", NumTargets() << 4);

	plDistTree distTree;
	Distribute(replicants, nil, bar, &distTree);

	IMakeOne(replicants);

	Done();

	GetCOREInterface()->EnableSceneRedraw();
	GetCOREInterface()->ForceCompleteRedraw(FALSE);
}

INode* plDistribComponent::IMakeOne(plDistribInstTab& nodes)
{
	if( !nodes.Count() )
		return nil;

	int iStartNode = 0;

	NameMaker *nn = GetCOREInterface()->NewNameMaker();

	while( iStartNode < nodes.Count() )
	{
		TriObject* triObj = CreateNewTriObject();
		Mesh* outMesh = &triObj->mesh;

		*outMesh = *nodes[iStartNode].fMesh;

		INode *outNode = GetCOREInterface()->CreateObjectNode(triObj);

		Matrix3 l2w = nodes[0].fObjectTM;
		Matrix3 w2l = Inverse(l2w);

		MeshDelta meshDelta(*outMesh);

		int i;
		for( i = iStartNode; i < nodes.Count(); i++ )
		{
			Mesh* nextMesh = nodes[i].fMesh;

			Matrix3 relativeTransform = nodes[i].fObjectTM * w2l;

			meshDelta.AttachMesh(*outMesh, *nextMesh, relativeTransform, 0);

			meshDelta.Apply(*outMesh);

			const int kFaceCutoff = 1000;
			if( outMesh->getNumFaces() > kFaceCutoff )
				break;
		}
		iStartNode = i;

		outNode->SetNodeTM(TimeValue(0), l2w);
		outNode->CopyProperties(nodes[0].fNode);
		outNode->SetMtl(nodes[0].fNode->GetMtl());
		outNode->SetObjOffsetPos(Point3(0,0,0));
		Quat identQuat;
		identQuat.Identity();
		outNode->SetObjOffsetRot(identQuat);
		outNode->SetObjOffsetScale(ScaleValue(Point3(1.f, 1.f, 1.f)));

		TSTR outName(TSTR("Preview"));
		nn->MakeUniqueName(outName);
		outNode->SetName(outName);

		fCompPB->Append(kReplicants, 1, &outNode);
	}

	return nil;
}

Box3 plDistribComponent::GetFade()
{
	Point3 pmin;
	if( fCompPB->GetInt(kFadeInActive) )
	{
		pmin.Set(fCompPB->GetFloat(kFadeInTran), fCompPB->GetFloat(kFadeInOpaq), 0);
		if( pmin[0] == pmin[1] )
			pmin[2] = 0;
		else if( pmin[0] < pmin[1] )
			pmin[2] = -1.f;
		else
			pmin[2] = 1.f;
	}
	else
	{
		pmin.Set(0.f,0.f,0.f);
	}

	Point3 pmax;
	pmax.Set(fCompPB->GetFloat(kFadeOutTran), fCompPB->GetFloat(kFadeOutOpaq), 0);
	if( pmax[0] == pmax[1] )
		pmax[2] = 0;
	else if( pmax[0] < pmax[1] )
		pmax[2] = -1.f;
	else
		pmax[2] = 1.f;

	return Box3(pmin, pmax);
}

BOOL plDistribComponent::IValidateFade(Box3& fade)
{
	BOOL retVal = true;
	fade = GetFade();

	if( fCompPB->GetInt(kFadeInActive) )
	{
		if( fade.Max()[0] < fade.Max()[1] )
		{
			if( fade.Min()[0] > fade.Max()[0] )
			{
				fade.pmin[0] = fade.Max()[0];
				retVal = false;
			}

			if( fade.Min()[1] > fade.Min()[0] )
			{
				fade.pmin[1] = fade.Min()[0];
				retVal = false;
			}
		}
		else if( fade.Max()[0] > fade.Max()[1] )
		{
			if( fade.Min()[1] > fade.Max()[1] )
			{
				fade.pmin[1] = fade.Max()[1];
				retVal = false;
			}

			if( fade.Min()[0] > fade.Min()[1] )
			{
				fade.pmin[0] = fade.Min()[1];
				retVal = false;
			}
		}
	}
	return retVal;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

