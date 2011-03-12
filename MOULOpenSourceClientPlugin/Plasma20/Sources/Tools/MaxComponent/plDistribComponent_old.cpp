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

#include "../MaxPlasmaMtls/Layers/plLayerTex.h"

#include "hsTypes.h"

#include "../MaxConvert/plDistributor.h"
#include "../MaxConvert/plDistTree.h" // FISH HACK - just testing

#include "plDistribComponent_old.h"

static const int kNumColorChanOptions = 13;

struct plProbChanStringValPair
{
	plDistributor::ColorChan		fValue;
	const char*						fString;
};

static plProbChanStringValPair kProbColorChanStrings[kNumColorChanOptions] =
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

void DummyCodeIncludeFuncDistrib_old()
{

}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

class plDistribComponentProc_old : public ParamMap2UserDlgProc
{
private:
	void ISetupScaleLock(IParamMap2* map, BOOL onInit=false)
	{
		IParamBlock2 *pb = map->GetParamBlock();
		if( pb->GetInt(plDistribComponent_old::kLockScaleXYZ) )
		{
			map->Enable(plDistribComponent_old::kLockScaleXY, FALSE);

			map->Enable(plDistribComponent_old::kScaleLoY, FALSE);
			map->Enable(plDistribComponent_old::kScaleHiY, FALSE);

			map->Enable(plDistribComponent_old::kScaleLoZ, FALSE);
			map->Enable(plDistribComponent_old::kScaleHiZ, FALSE);
		}
		else if( pb->GetInt(plDistribComponent_old::kLockScaleXY) )
		{
			map->Enable(plDistribComponent_old::kLockScaleXY, TRUE);

			map->Enable(plDistribComponent_old::kScaleLoY, FALSE);
			map->Enable(plDistribComponent_old::kScaleHiY, FALSE);

			map->Enable(plDistribComponent_old::kScaleLoZ, TRUE);
			map->Enable(plDistribComponent_old::kScaleHiZ, TRUE);
		}
		else
		{
			map->Enable(plDistribComponent_old::kLockScaleXY, TRUE);

			map->Enable(plDistribComponent_old::kScaleLoY, TRUE);
			map->Enable(plDistribComponent_old::kScaleHiY, TRUE);

			map->Enable(plDistribComponent_old::kScaleLoZ, TRUE);
			map->Enable(plDistribComponent_old::kScaleHiZ, TRUE);
		}
		if( onInit )
		{
			map->SetTooltip(plDistribComponent_old::kLockScaleXY, TRUE, "Lock scale in X and Y" );
			map->SetTooltip(plDistribComponent_old::kLockScaleXYZ, TRUE, "Lock scale in X, Y and Z (uniform scale)" );
		}
	}
public:
	BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		HWND cbox = NULL;
		switch (msg)
		{

		case WM_INITDIALOG:
			cbox = GetDlgItem(hWnd, IDC_COMP_DISTRIB_PROBCOLORCHAN);
			int i;
			for( i = 0; i < kNumColorChanOptions; i++ )
			{
				SendMessage(cbox, CB_ADDSTRING, 0, (LPARAM)kProbColorChanStrings[i].fString);
			}
			SendMessage(cbox, CB_SETCURSEL, map->GetParamBlock()->GetInt(plDistribComponent_old::kProbColorChan), 0);

			ISetupScaleLock(map, true);

			return TRUE;
		case WM_COMMAND:
			if( (HIWORD(wParam) == BN_CLICKED) && (LOWORD(wParam) == IDC_DISTRIB_CLEAR) )
			{
				plDistribComponent_old* dc = (plDistribComponent_old*)map->GetParamBlock()->GetOwner();
				dc->Clear();

				return TRUE;
			}
			switch( LOWORD(wParam) )
			{
				case IDC_COMP_DISTRIB_PROBCOLORCHAN:
				{
					map->GetParamBlock()->SetValue(plDistribComponent_old::kProbColorChan, t, SendMessage(GetDlgItem(hWnd, LOWORD(wParam)), CB_GETCURSEL, 0, 0));
					return TRUE;
				}
				break;

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
					plDistribComponent_old* dc = (plDistribComponent_old*)map->GetParamBlock()->GetOwner();
					Box3 fade;
					if( !dc->IValidateFade(fade) )
					{
						map->GetParamBlock()->SetValue(plDistribComponent_old::kFadeInTran, t, fade.Min()[0]);
						map->GetParamBlock()->SetValue(plDistribComponent_old::kFadeInOpaq, t, fade.Min()[1]);
						map->GetParamBlock()->SetValue(plDistribComponent_old::kFadeOutTran, t, fade.Max()[0]);
						map->GetParamBlock()->SetValue(plDistribComponent_old::kFadeOutOpaq, t, fade.Max()[1]);

						map->Invalidate(plDistribComponent_old::kFadeInTran);
						map->Invalidate(plDistribComponent_old::kFadeInOpaq);
						map->Invalidate(plDistribComponent_old::kFadeOutTran);
						map->Invalidate(plDistribComponent_old::kFadeOutOpaq);
						ShowWindow(hWnd, SW_HIDE);
						ShowWindow(hWnd, SW_SHOW);
					}
					return TRUE;
				}
				break;
#if 0 // Obsolete, now kRndPosRadius is percentage of kSpacing
				case IDC_COMP_DISTRIB_RNDPOSRADIUS:
				case IDC_COMP_DISTRIB_RNDPOSRADIUS_SPIN:
				{
					IParamBlock2 *pb = map->GetParamBlock();

					float maxRndPosRad = pb->GetFloat(plDistribComponent_old::kSpacing) * 0.5f;
					if( pb->GetFloat(plDistribComponent_old::kRndPosRadius) > maxRndPosRad )
					{
						pb->SetValue(plDistribComponent_old::kRndPosRadius, t, maxRndPosRad);
						map->Invalidate(plDistribComponent_old::kRndPosRadius);
						ShowWindow(hWnd, SW_HIDE);
						ShowWindow(hWnd, SW_SHOW);
					}
					
					return TRUE;
				}
#endif // Obsolete, now kRndPosRadius is percentage of kSpacing
				case IDC_COMP_DISTRIB_LOCKSCALEXY:
				case IDC_COMP_DISTRIB_LOCKSCALEXYZ:
					ISetupScaleLock(map, false);
					return TRUE;

			}
			break;
		}

		return false;
	}
	void DeleteThis() {}
};
static plDistribComponentProc_old gDistribCompProc_old;


//Max desc stuff necessary below.
OBSOLETE_CLASS_DESC(plDistribComponent_old, gDistribCompDesc_old, "Distributor(old)",  "Distributor(old)", COMP_TYPE_DISTRIBUTOR, DISTRIBUTOR_COMP_CID_OLD)

class plDistribCompAccessor_old : public PBAccessor
{
public:
	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)
	{
		if( id == plDistribComponent_old::kTemplates )
		{
			plDistribComponent_old* comp = (plDistribComponent_old*)owner;
			comp->NotifyDependents(FOREVER, PART_ALL, REFMSG_USER_COMP_REF_CHANGED);
		}
	}
};
plDistribCompAccessor_old gDistribCompAccessor_old;


ParamBlockDesc2 gDistributorBk_old
(	// KLUDGE: not the defined block ID, but kept for backwards compat.
 plComponent::kBlkComp, _T("Distributor_old"), 0, &gDistribCompDesc_old, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	IDD_COMP_DISTRIBUTOR, IDS_COMP_DISTRIBUTORS, 0, 0, &gDistribCompProc_old,

	plDistribComponent_old::kTemplates,	_T("Templates"),	TYPE_INODE_TAB, 0,		P_CAN_CONVERT, 0,
		p_ui,			TYPE_NODELISTBOX, IDC_LIST_TARGS, IDC_ADD_TARGS, 0, IDC_DEL_TARGS,
		p_classID,		triObjectClassID,
		p_accessor,		&gDistribCompAccessor_old,
		end,

	plDistribComponent_old::kSpacing, _T("Spacing"), TYPE_FLOAT, 	0, 0,	
		p_default, 10.0,
		p_range, 0.1, 100.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_DISTRIB_SPACING, IDC_COMP_DISTRIB_SPACING_SPIN, 1.0,
		end,	
	
	plDistribComponent_old::kRndPosRadius, _T("Space Range"), TYPE_FLOAT, 	0, 0,	
		p_default, 50.0,
		p_range, 0.0, 100.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_DISTRIB_RNDPOSRADIUS, IDC_COMP_DISTRIB_RNDPOSRADIUS_SPIN, 1.0,
		end,	

	plDistribComponent_old::kAlignVecX, _T("AlignX"), TYPE_FLOAT, 	0, 0,	
		p_default, 0.0,
		p_range, -1.f, 1.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_FLOAT,	
		IDC_COMP_DISTRIB_ALIGNVECX, IDC_COMP_DISTRIB_ALIGNVECX_SPIN, 0.1,
		end,	

	plDistribComponent_old::kAlignVecY, _T("AlignY"), TYPE_FLOAT, 	0, 0,	
		p_default, 0.0,
		p_range, -1.f, 1.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_FLOAT,	
		IDC_COMP_DISTRIB_ALIGNVECY, IDC_COMP_DISTRIB_ALIGNVECY_SPIN, 0.1,
		end,	

	plDistribComponent_old::kAlignVecZ, _T("AlignZ"), TYPE_FLOAT, 	0, 0,	
		p_default, 1.0,
		p_range, -1.f, 1.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_FLOAT,	
		IDC_COMP_DISTRIB_ALIGNVECZ, IDC_COMP_DISTRIB_ALIGNVECZ_SPIN, 0.1,
		end,	

	plDistribComponent_old::kAlignWgt, _T("Align Weight"), TYPE_FLOAT, 	0, 0,	
		p_default, 50.0,
		p_range, 0.0, 100.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_DISTRIB_ALIGNWGT, IDC_COMP_DISTRIB_ALIGNWGT_SPIN, 1.0,
		end,	
	
	plDistribComponent_old::kPolarRange, _T("Normal Range (Deg)"), TYPE_FLOAT, 	0, 0,	
		p_default, 15.0,
		p_range, 0.0, 180.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_DISTRIB_POLARRANGE, IDC_COMP_DISTRIB_POLARRANGE_SPIN, 1.0,
		end,	
	
	plDistribComponent_old::kAzimuthRange, _T("Normal Range (Deg)"), TYPE_FLOAT, 	0, 0,	
		p_default, 180.0,
		p_range, 0.0, 180.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_DISTRIB_AZIMUTHRANGE, IDC_COMP_DISTRIB_AZIMUTHRANGE_SPIN, 1.0,
		end,	
	
	plDistribComponent_old::kOverallProb, _T("Overall Probability"), TYPE_FLOAT, 	0, 0,	
		p_default, 100.0,
		p_range, 1., 100.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_DISTRIB_OVERALLPROB, IDC_COMP_DISTRIB_OVERALLPROB_SPIN, 1.0,
		end,	
	
	plDistribComponent_old::kPolarBunch, _T("Normal Bunching"), TYPE_FLOAT, 	0, 0,	
		p_default, 0.0,
		p_range, 0.0, 100.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_DISTRIB_POLARBUNCH, IDC_COMP_DISTRIB_POLARBUNCH_SPIN, 1.0,
		end,	
	
	plDistribComponent_old::kScaleLoX, _T("ScaleMinX"), TYPE_FLOAT, 	0, 0,	
		p_default, 1.0,
		p_range, 0.1, 10.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_DISTRIB_SCALELOX, IDC_COMP_DISTRIB_SCALELOX_SPIN, 1.0,
		end,	

	plDistribComponent_old::kScaleLoY, _T("ScaleMinY"), TYPE_FLOAT, 	0, 0,	
		p_default, 1.0,
		p_range, 0.1, 10.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_DISTRIB_SCALELOY, IDC_COMP_DISTRIB_SCALELOY_SPIN, 1.0,
		end,	

	plDistribComponent_old::kScaleLoZ, _T("ScaleMinZ"), TYPE_FLOAT, 	0, 0,	
		p_default, 1.0,
		p_range, 0.1, 10.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_DISTRIB_SCALELOZ, IDC_COMP_DISTRIB_SCALELOZ_SPIN, 1.0,
		end,	

	plDistribComponent_old::kScaleHiX, _T("ScaleMaxX"), TYPE_FLOAT, 	0, 0,	
		p_default, 1.0,
		p_range, 0.1, 10.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_DISTRIB_SCALEHIX, IDC_COMP_DISTRIB_SCALEHIX_SPIN, 1.0,
		end,	

	plDistribComponent_old::kScaleHiY, _T("ScaleMaxY"), TYPE_FLOAT, 	0, 0,	
		p_default, 1.0,
		p_range, 0.1, 10.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_DISTRIB_SCALEHIY, IDC_COMP_DISTRIB_SCALEHIY_SPIN, 1.0,
		end,	

	plDistribComponent_old::kScaleHiZ, _T("ScaleMaxZ"), TYPE_FLOAT, 	0, 0,	
		p_default, 1.0,
		p_range, 0.1, 10.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_DISTRIB_SCALEHIZ, IDC_COMP_DISTRIB_SCALEHIZ_SPIN, 1.0,
		end,	

	plDistribComponent_old::kReplicants,	_T("Replicants"),	TYPE_INODE_TAB, 0,		0, 0,
		p_accessor,		&gDistribCompAccessor_old,
		end,

	plDistribComponent_old::kProbTexmap,			_T("ProbTexmap"),	TYPE_TEXMAP,		0, 0,
		p_ui,				TYPE_TEXMAPBUTTON, IDC_COMP_DISTRIB_PROBTEXMAP,
		end,

	plDistribComponent_old::kProbColorChan,	_T("ProbColorChan"),	TYPE_INT,	0, 0,
		p_default, 0,
		end,

	plDistribComponent_old::kSeedLocked,  _T("Locked"), TYPE_BOOL, 		0, 0,
		p_default,	FALSE,
		p_ui,	TYPE_SINGLECHEKBOX, IDC_COMP_DISTRIB_LOCK,
		end,

	plDistribComponent_old::kSeed,	_T("Seed"),	TYPE_INT,	0, 0,
		p_default, 1,
		end,

	plDistribComponent_old::kNextSeed,	_T("NextSeed"),	TYPE_INT,	0, 0,
		p_default, 1,
		end,

	plDistribComponent_old::kRemapFromLo, _T("RemapFromLo"), TYPE_FLOAT, 	0, 0,	
		p_default, 0.0,
		p_range, 0.0, 255.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_DISTRIB_REMAPFROMLO, IDC_COMP_DISTRIB_REMAPFROMLO_SPIN, 1.0,
		end,	

	plDistribComponent_old::kRemapFromHi, _T("RemapFromHi"), TYPE_FLOAT, 	0, 0,	
		p_default, 255.0,
		p_range, 0.0, 255.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_DISTRIB_REMAPFROMHI, IDC_COMP_DISTRIB_REMAPFROMHI_SPIN, 1.0,
		end,	

	plDistribComponent_old::kRemapToLo, _T("RemapToLo"), TYPE_FLOAT, 	0, 0,	
		p_default, 0.0,
		p_range, 0.0, 255.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_DISTRIB_REMAPTOLO, IDC_COMP_DISTRIB_REMAPTOLO_SPIN, 1.0,
		end,	

	plDistribComponent_old::kRemapToHi, _T("RemapToHi"), TYPE_FLOAT, 	0, 0,	
		p_default, 255.0,
		p_range, 0.0, 255.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_DISTRIB_REMAPTOHI, IDC_COMP_DISTRIB_REMAPTOHI_SPIN, 1.0,
		end,	

	plDistribComponent_old::kAngProbX, _T("AngProbX"), TYPE_FLOAT, 	0, 0,	
		p_default, 0.0,
		p_range, -1.f, 1.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_FLOAT,	
		IDC_COMP_DISTRIB_ANGPROBX, IDC_COMP_DISTRIB_ANGPROBX_SPIN, 0.1,
		end,	

	plDistribComponent_old::kAngProbY, _T("AngProbY"), TYPE_FLOAT, 	0, 0,	
		p_default, 0.0,
		p_range, -1.f, 1.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_FLOAT,	
		IDC_COMP_DISTRIB_ANGPROBY, IDC_COMP_DISTRIB_ANGPROBY_SPIN, 0.1,
		end,	

	plDistribComponent_old::kAngProbZ, _T("AngProbZ"), TYPE_FLOAT, 	0, 0,	
		p_default, 1.0,
		p_range, -1.f, 1.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_FLOAT,	
		IDC_COMP_DISTRIB_ANGPROBZ, IDC_COMP_DISTRIB_ANGPROBZ_SPIN, 0.1,
		end,	

	plDistribComponent_old::kAngProbHi, _T("AngProbHi"), TYPE_FLOAT, 	0, 0,	
		p_default, 0.0,
		p_range, 0.0, 180.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_DISTRIB_ANGPROBHI, IDC_COMP_DISTRIB_ANGPROBHI_SPIN, 1.0,
		end,	
	
	plDistribComponent_old::kAngProbLo, _T("AngProbLo"), TYPE_FLOAT, 	0, 0,	
		p_default, 0.0,
		p_range, 0.0, 180.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_DISTRIB_ANGPROBLO, IDC_COMP_DISTRIB_ANGPROBLO_SPIN, 1.0,
		end,	
	
	plDistribComponent_old::kFadeInTran, _T("FadeInTran"), TYPE_FLOAT, 	0, 0,	
		p_default, 0.0,
		p_range, 0.0, 1000.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_DISTRIB_FADEINTRAN, IDC_COMP_DISTRIB_FADEINTRAN_SPIN, 1.0,
		end,	
	
	plDistribComponent_old::kFadeInOpaq, _T("FadeInOpaq"), TYPE_FLOAT, 	0, 0,	
		p_default, 0.0,
		p_range, 0.0, 1000.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_DISTRIB_FADEINOPAQ, IDC_COMP_DISTRIB_FADEINOPAQ_SPIN, 1.0,
		end,	
	
	plDistribComponent_old::kFadeOutTran, _T("FadeOutTran"), TYPE_FLOAT, 	0, 0,	
		p_default, 0.0,
		p_range, 0.0, 1000.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_DISTRIB_FADEOUTTRAN, IDC_COMP_DISTRIB_FADEOUTTRAN_SPIN, 1.0,
		end,	
	
	plDistribComponent_old::kFadeOutOpaq, _T("FadeOutOpaq"), TYPE_FLOAT, 	0, 0,	
		p_default, 0.0,
		p_range, 0.0, 1000.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_DISTRIB_FADEOUTOPAQ, IDC_COMP_DISTRIB_FADEOUTOPAQ_SPIN, 1.0,
		end,	
	
	plDistribComponent_old::kFadeInActive,  _T("FadeInActive"), TYPE_BOOL, 		0, 0,
		p_default,	FALSE,
		p_ui,	TYPE_SINGLECHEKBOX, IDC_COMP_DISTRIB_FADEINACTIVE,
		p_enable_ctrls,		2, plDistribComponent_old::kFadeInTran, plDistribComponent_old::kFadeInOpaq,
		end,

	plDistribComponent_old::kWindBone, _T("WindBone"),	TYPE_INODE,		0, 0,
		p_ui,	TYPE_PICKNODEBUTTON, IDC_COMP_DISTRIB_WINDBONE,
		p_prompt, IDS_COMP_CLUSTER_CHOSE_WINDBONE,
		end,

	plDistribComponent_old::kLockScaleXY,  _T("LockScaleXY"), TYPE_BOOL, 		0, 0,
		p_default,	FALSE,
		p_ui,	TYPE_SINGLECHEKBOX, IDC_COMP_DISTRIB_LOCKSCALEXY,
		end,

	plDistribComponent_old::kLockScaleXYZ,  _T("LockScaleXYZ"), TYPE_BOOL, 		0, 0,
		p_default,	FALSE,
		p_ui,	TYPE_SINGLECHEKBOX, IDC_COMP_DISTRIB_LOCKSCALEXYZ,
		end,

	plDistribComponent_old::kWindBoneActive,  _T("WindBoneActive"), TYPE_BOOL, 		0, 0,
		p_default,	FALSE,
		p_ui,	TYPE_SINGLECHEKBOX, IDC_COMP_DISTRIB_WINDBONEACTIVE,
		p_enable_ctrls,		1, plDistribComponent_old::kWindBone,
		end,

	// Make this a combo box in real gig.
	plDistribComponent_old::kIsolation,	_T("Isolation"),	TYPE_INT,	0, 0,
		p_ui,	TYPE_SPINNER, EDITTYPE_INT,	IDC_COMP_DISTRIB_ISOLATION, IDC_COMP_DISTRIB_ISOLATION_SPIN,	0.4,
//		p_range, 0, 3,
//		p_default, 1,
		end,

	end
);

plDistribComponent_old::plDistribComponent_old()
{
	fClassDesc = &gDistribCompDesc_old;
	fClassDesc->MakeAutoParamBlocks(this);
}

hsBool plDistribComponent_old::SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg)
{
	int numReps = fCompPB->Count(kTemplates);
	int i;
	for( i = 0; i < numReps; i++ )
	{
		plMaxNodeBase* temp = (plMaxNodeBase*)fCompPB->GetINode(kTemplates, TimeValue(0), i);
		if( temp )
			temp->SetCanConvert(false);
	}

	return true;
}

BOOL plDistribComponent_old::Distribute(plDistribInstTab& replicants, plExportProgressBar& bar, plDistTree* distTree)
{
	return false;

	BOOL retVal = true;

	GetCOREInterface()->DisableSceneRedraw();

	Clear();

	plDistributor distrib;

	distrib.SetTheInterface(GetCOREInterface());

	distrib.SetSpacing(fCompPB->GetFloat(kSpacing));

	distrib.SetSpacingRange(fCompPB->GetFloat(kRndPosRadius) * 0.01f * distrib.GetSpacing() * 0.5f);

	Point3 align(fCompPB->GetFloat(kAlignVecX), fCompPB->GetFloat(kAlignVecY), fCompPB->GetFloat(kAlignVecZ));
	align.FNormalize();
	distrib.SetAlignmentVec(align);

	distrib.SetAlignmentWeight(fCompPB->GetFloat(kAlignWgt));

	distrib.SetPolarRange(fCompPB->GetFloat(kPolarRange));

	distrib.SetAzimuthRange(fCompPB->GetFloat(kAzimuthRange));

	distrib.SetOverallProb(fCompPB->GetFloat(kOverallProb));

	distrib.SetPolarBunch(fCompPB->GetFloat(kPolarBunch));

	Point3 scaleLo(fCompPB->GetFloat(kScaleLoX), fCompPB->GetFloat(kScaleLoY), fCompPB->GetFloat(kScaleLoZ));
	Point3 scaleHi(fCompPB->GetFloat(kScaleHiX), fCompPB->GetFloat(kScaleHiY), fCompPB->GetFloat(kScaleHiZ));
	distrib.SetScaleRange(scaleLo, scaleHi);
	ULONG scaleLock = plDistributor::kLockNone;
	if( fCompPB->GetInt(kLockScaleXYZ) )
		scaleLock = plDistributor::kLockX | plDistributor::kLockY | plDistributor::kLockZ;
	else if( fCompPB->GetInt(kLockScaleXY) )
		scaleLock = plDistributor::kLockX | plDistributor::kLockY;
	distrib.SetScaleLock(scaleLock);

	distrib.SetProbabilityChan(kProbColorChanStrings[fCompPB->GetInt(kProbColorChan)].fValue);

	ISetProbTexmap(distrib);

	// Setup the new params here. FISH...
	Point3 probVec(fCompPB->GetFloat(kAngProbX), fCompPB->GetFloat(kAngProbY), fCompPB->GetFloat(kAngProbZ));
	distrib.SetAngleProbVec(probVec);
	distrib.SetAngleProbHi(fCompPB->GetFloat(kAngProbHi));
	distrib.SetAngleProbLo(fCompPB->GetFloat(kAngProbLo));

	distrib.SetProbabilityRemapFromLo(fCompPB->GetFloat(kRemapFromLo));
	distrib.SetProbabilityRemapFromHi(fCompPB->GetFloat(kRemapFromHi));
	distrib.SetProbabilityRemapToLo(fCompPB->GetFloat(kRemapToLo));
	distrib.SetProbabilityRemapToHi(fCompPB->GetFloat(kRemapToHi));

	if( !fCompPB->GetInt(kSeedLocked) )
		fCompPB->SetValue(kSeed, TimeValue(0), fCompPB->GetInt(kNextSeed));

	distrib.SetRandSeed(fCompPB->GetInt(kSeed));

	distrib.SetFade(GetFade());

	if( fCompPB->GetInt(kWindBoneActive) )
		distrib.SetBone(fCompPB->GetINode(kWindBone));

	distrib.SetRigid(!IsFlexible());

	// FISH HACK, get this passed in from Cluster
	distrib.SetDistTree(distTree);
	distrib.SetIsolation(GetIsolation());

	int numReps = fCompPB->Count(kTemplates);
	int i;
	for( i = 0; i < numReps; i++ )
	{
		INode* temp = fCompPB->GetINode(kTemplates, TimeValue(0), i);
		if( temp )
			distrib.AddReplicateNode(temp);
	}

	int numTarg = NumTargets();
	for( i = 0; i < numTarg; i++ )
	{
		if( GetTarget(i) )
		{
			if( !distrib.Distribute(GetTarget(i), replicants, fDistCache, bar) )
			{
				retVal = false;
				break;
			}
		}
	}

	fCompPB->SetValue(kNextSeed, TimeValue(0), int(distrib.GetRandSeed()));

	BOOL redrawDissed = GetCOREInterface()->IsSceneRedrawDisabled();

	GetCOREInterface()->EnableSceneRedraw();

	GetCOREInterface()->ForceCompleteRedraw(FALSE);

	return retVal;
}

void plDistribComponent_old::Done()
{
	int i;
	for( i = 0; i < fDistCache.Count(); i++ )
	{
		delete fDistCache[i].fMesh;
	}
	fDistCache.ZeroCount();
}

BOOL plDistribComponent_old::IsFlexible() const
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

void plDistribComponent_old::ISetProbTexmap(plDistributor& distrib)
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

float plDistribComponent_old::GetIsoPriority() const
{
	float pri = fCompPB->GetFloat(kSpacing);
	pri *= pri;
	pri *= fCompPB->GetFloat(kOverallProb) * 0.01f;

	return pri;
}

plDistributor::IsoType plDistribComponent_old::GetIsolation() const
{
	return (plDistributor::IsoType)fCompPB->GetInt(kIsolation);
}

void plDistribComponent_old::Clear()
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

void plDistribComponent_old::Preview()
{
	if( !NumTargets() )
		return;

	GetCOREInterface()->DisableSceneRedraw();

	plDistribInstTab replicants;

	plExportProgressBar bar;
	bar.Start("Preview", NumTargets() << 4);

	plDistTree distTree;
	Distribute(replicants, bar, &distTree);

	IMakeOne(replicants);

	Done();

	GetCOREInterface()->EnableSceneRedraw();
	GetCOREInterface()->ForceCompleteRedraw(FALSE);
}

INode* plDistribComponent_old::IMakeOne(plDistribInstTab& nodes)
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

Box3 plDistribComponent_old::GetFade()
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

BOOL plDistribComponent_old::IValidateFade(Box3& fade)
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

