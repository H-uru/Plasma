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
#include <windowsx.h>

#include "HeadSpin.h"
#include "max.h"
#include "resource.h"
#include "hsTemplates.h"
#include "hsResMgr.h"
#include "plQuality.h"
#include "../pnMessage/plRefMsg.h"
#include "../../PubUtilLib/plSurface/hsGMaterial.h"
#include "../MaxMain/plMaxNode.h"
#include "../MaxConvert/hsMaterialConverter.h"

#include "plComponent.h"
#include "plComponentReg.h"
#include "plGrassComponent.h"
#include "../../PubUtilLib/plSurface/plGrassShaderMod.h"

#include "../pnKeyedObject/plUoid.h"
#include "../pnMessage/plObjRefMsg.h"
#include "../MaxMain/plPluginResManager.h"

void DummyCodeIncludeFuncGrassShader()
{
}

class GrassCompDlgProc : public ParamMap2UserDlgProc
{
public:
	GrassCompDlgProc() {}
	~GrassCompDlgProc() {}

protected:
	void ISetToWave(int wave, HWND hWnd, IParamBlock2 *pb, IParamMap2 *map)
	{
		HWND cbox = GetDlgItem(hWnd, IDC_GRASS_WAVE);
		SendMessage(cbox, CB_SETCURSEL, wave, 0);
		pb->SetValue(ParamID(plGrassComponent::kWave), 0, wave);
		pb->SetValue(ParamID(plGrassComponent::kDistX), 0, pb->GetFloat(ParamID(plGrassComponent::kDistXTab), 0, wave));
		pb->SetValue(ParamID(plGrassComponent::kDistY), 0, pb->GetFloat(ParamID(plGrassComponent::kDistYTab), 0, wave));
		pb->SetValue(ParamID(plGrassComponent::kDistZ), 0, pb->GetFloat(ParamID(plGrassComponent::kDistZTab), 0, wave));
		pb->SetValue(ParamID(plGrassComponent::kDirX), 0, pb->GetFloat(ParamID(plGrassComponent::kDirXTab), 0, wave));
		pb->SetValue(ParamID(plGrassComponent::kDirY), 0, pb->GetFloat(ParamID(plGrassComponent::kDirYTab), 0, wave));
		pb->SetValue(ParamID(plGrassComponent::kSpeed), 0, pb->GetFloat(ParamID(plGrassComponent::kSpeedTab), 0, wave));
		map->Invalidate();
	}

public:
	BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		int id = LOWORD(wParam);
		int code = HIWORD(wParam);

		IParamBlock2 *pb = map->GetParamBlock();
		HWND cbox = NULL;

		int selection;
		switch (msg)
		{
		case WM_INITDIALOG:
			cbox = GetDlgItem(hWnd, IDC_GRASS_WAVE);
			SendMessage(cbox, CB_ADDSTRING, 0, (LPARAM)"1");
			SendMessage(cbox, CB_ADDSTRING, 1, (LPARAM)"2");
			SendMessage(cbox, CB_ADDSTRING, 2, (LPARAM)"3");
			SendMessage(cbox, CB_ADDSTRING, 3, (LPARAM)"4");

			selection = pb->GetInt(plGrassComponent::kWave);
			ISetToWave(selection, hWnd, pb, map);
			return TRUE;

		case WM_COMMAND:
		case CC_SPINNER_CHANGE:
			int wave = SendMessage(GetDlgItem(hWnd, IDC_GRASS_WAVE), CB_GETCURSEL, 0, 0);
			if (id == IDC_GRASS_WAVE)
			{
				if (wave != pb->GetInt(ParamID(plGrassComponent::kWave)))
					ISetToWave(wave, hWnd, pb, map);
			}
			else if (id == IDC_GRASS_DIST_X || id == IDC_GRASS_DIST_X_SPIN)
				pb->SetValue(plGrassComponent::kDistXTab, 0, pb->GetFloat(ParamID(plGrassComponent::kDistX)), wave);
			else if (id == IDC_GRASS_DIST_Y || id == IDC_GRASS_DIST_Y_SPIN)
				pb->SetValue(plGrassComponent::kDistYTab, 0, pb->GetFloat(ParamID(plGrassComponent::kDistY)), wave);
			else if (id == IDC_GRASS_DIST_Z || id == IDC_GRASS_DIST_Z_SPIN)
				pb->SetValue(plGrassComponent::kDistZTab, 0, pb->GetFloat(ParamID(plGrassComponent::kDistZ)), wave);
			else if (id == IDC_GRASS_DIR_X || id == IDC_GRASS_DIR_X_SPIN)
				pb->SetValue(plGrassComponent::kDirXTab, 0, pb->GetFloat(ParamID(plGrassComponent::kDirX)), wave);
			else if (id == IDC_GRASS_DIR_Y || id == IDC_GRASS_DIR_Y_SPIN)
				pb->SetValue(plGrassComponent::kDirYTab, 0, pb->GetFloat(ParamID(plGrassComponent::kDirY)), wave);
			else if (id == IDC_GRASS_SPEED || id == IDC_GRASS_SPEED_SPIN)
				pb->SetValue(plGrassComponent::kSpeedTab, 0, pb->GetFloat(ParamID(plGrassComponent::kSpeed)), wave);

			return TRUE;
		}
		return FALSE;
	}
	void DeleteThis() {}
};
static GrassCompDlgProc gGrassCompDlgProc;

CLASS_DESC(plGrassComponent, gGrassDesc, "Grass",  "Grass", COMP_TYPE_SHADERS, GRASS_COMPONENT_CLASS_ID)

ParamBlockDesc2 gGrassBk
(

	plComponent::kBlkComp, _T("Grass"), 0, &gGrassDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	//Roll out
	IDD_COMP_GRASS, IDS_COMP_GRASS_ROLL, 0, 0, &gGrassCompDlgProc,


	plGrassComponent::kWave, _T("Wave"), TYPE_INT, 0, 0,
		p_default, 0,
		end,

	plGrassComponent::kDistX, _T("DistX"), TYPE_FLOAT, 0, 0,
		p_default, 0.0,
		p_range, -10.0, 10.0,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT,
		IDC_GRASS_DIST_X, IDC_GRASS_DIST_X_SPIN, 0.1,
		end,

	plGrassComponent::kDistY, _T("DistY"), TYPE_FLOAT, 0, 0,
		p_default, 0.0,
		p_range, -10.0, 10.0,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT,
		IDC_GRASS_DIST_Y, IDC_GRASS_DIST_Y_SPIN, 0.1,
		end,

	plGrassComponent::kDistZ, _T("DistZ"), TYPE_FLOAT, 0, 0,
		p_default, 0.0,
		p_range, -10.0, 10.0,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT,
		IDC_GRASS_DIST_Z, IDC_GRASS_DIST_Z_SPIN, 0.1,
		end,

	plGrassComponent::kDirX, _T("DirX"), TYPE_FLOAT, 0, 0,
		p_default, 0.0,
		p_range, -10.0, 10.0,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT,
		IDC_GRASS_DIR_X, IDC_GRASS_DIR_X_SPIN, 0.1,
		end,

	plGrassComponent::kDirY, _T("DirY"), TYPE_FLOAT, 0, 0,
		p_default, 0.0,
		p_range, -10.0, 10.0,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT,
		IDC_GRASS_DIR_Y, IDC_GRASS_DIR_Y_SPIN, 0.1,
		end,

	plGrassComponent::kSpeed, _T("Speed"), TYPE_FLOAT, 0, 0,
		p_default, 0.0,
		p_range, -10.0, 10.0,
		p_ui, TYPE_SPINNER, EDITTYPE_FLOAT,
		IDC_GRASS_SPEED, IDC_GRASS_SPEED_SPIN, 0.1,
		end,

		plGrassComponent::kDistXTab, _T("DistXTab"), TYPE_FLOAT_TAB, plGrassShaderMod::kNumWaves, 0, 0,
		p_default, 0.0,
		p_range, -10.0, 10.0,
		end,

	plGrassComponent::kDistYTab, _T("DistYTab"), TYPE_FLOAT_TAB, plGrassShaderMod::kNumWaves, 0, 0,
		p_default, 0.0,
		p_range, -10.0, 10.0,
		end,

	plGrassComponent::kDistZTab, _T("DistZTab"), TYPE_FLOAT_TAB, plGrassShaderMod::kNumWaves, 0, 0,
		p_default, 0.0,
		p_range, -10.0, 10.0,
		end,

	plGrassComponent::kDirXTab, _T("DirXTab"), TYPE_FLOAT_TAB, plGrassShaderMod::kNumWaves, 0, 0,
		p_default, 0.0,
		p_range, -10.0, 10.0,
		end,

	plGrassComponent::kDirYTab, _T("DirYTab"), TYPE_FLOAT_TAB, plGrassShaderMod::kNumWaves, 0, 0,
		p_default, 0.0,
		p_range, -10.0, 10.0,
		end,

	plGrassComponent::kSpeedTab, _T("SpeedTab"), TYPE_FLOAT_TAB, plGrassShaderMod::kNumWaves, 0, 0,
		p_default, 0.0,
		p_range, -10.0, 10.0,
		end,

	end
);

plGrassComponent::plGrassComponent() : fShader(nil)
{
	fClassDesc = &gGrassDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

hsBool plGrassComponent::PreConvert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	fShader = TRACKED_NEW plGrassShaderMod();

	plLoadMask loadMask;
	int qual = 1;
	int cap = plQuality::kPS_1_1;
	plLoadMask::ComputeRepMasks(1, &qual, &cap, &loadMask);
	plKey modKey = hsgResMgr::ResMgr()->NewKey(IGetUniqueName(node), fShader, node->GetLocation(), loadMask);

	int i;
	for (i = 0; i < plGrassShaderMod::kNumWaves; i++)
	{
		fShader->fWaves[i].fDistX = fCompPB->GetFloat(ParamID(kDistXTab), 0, i);
		fShader->fWaves[i].fDistY = fCompPB->GetFloat(ParamID(kDistYTab), 0, i);
		fShader->fWaves[i].fDistZ = fCompPB->GetFloat(ParamID(kDistZTab), 0, i);
		fShader->fWaves[i].fDirX = fCompPB->GetFloat(ParamID(kDirXTab), 0, i);
		fShader->fWaves[i].fDirY = fCompPB->GetFloat(ParamID(kDirYTab), 0, i);
		fShader->fWaves[i].fSpeed = fCompPB->GetFloat(ParamID(kSpeedTab), 0, i);
	}

	// Add a ref to the shader.
	fShader->GetKey()->RefObject();

	return true;
}

hsBool plGrassComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	plObjRefMsg* refMsg = TRACKED_NEW plObjRefMsg(node->GetKey(), plRefMsg::kOnRequest, -1, plObjRefMsg::kModifier);
	hsgResMgr::ResMgr()->AddViaNotify(fShader->GetKey(), refMsg, plRefFlags::kActiveRef);

	hsTArray<hsGMaterial*> mats;
	hsMaterialConverter::Instance().CollectConvertedMaterials(hsMaterialConverter::Instance().GetBaseMtl(node), mats);
	hsgResMgr::ResMgr()->SendRef(mats[0]->GetKey(), TRACKED_NEW plGenRefMsg(fShader->GetKey(), plRefMsg::kOnRequest, 0, plGrassShaderMod::kRefMaterial), plRefFlags::kActiveRef);

	return TRUE;
}

hsBool plGrassComponent::DeInit(plMaxNode* node, plErrorMsg* pErrMsg)
{
	if( fShader )
		fShader->GetKey()->UnRefObject();
	fShader = nil;

	return true;
}

plGrassShaderMod* plGrassComponent::GetShader(INode* node)
{
	if( !node )
		return nil;

	plComponentBase *comp = ((plMaxNodeBase*)node)->ConvertToComponent();
	if( !comp )
		return nil;

	if( comp->ClassID() != GRASS_COMPONENT_CLASS_ID )
		return nil;

	plGrassComponent *shader = (plGrassComponent*)comp;
	return shader->fShader;
}

plGrassShaderMod* plGrassComponent::GetShaderNode(plMaxNode* node)
{
	if( !node )
		return nil;

	int n = node->NumAttachedComponents();
	int i;
	for( i = 0; i < n; i++ )
	{
		plComponentBase* comp = node->GetAttachedComponent(i);
		if( comp && (comp->ClassID() == GRASS_COMPONENT_CLASS_ID) )
		{
			plGrassComponent* shader = (plGrassComponent*)comp;
			return shader->fShader;
		}
	}
	return nil;
}
