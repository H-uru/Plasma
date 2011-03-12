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
#include "plMaxUtils.h"
#include "resource.h"

#include "hsResMgr.h"
#include "../plResMgr/plPageInfo.h"
#include "../pnKeyedObject/plUoid.h"
#include "../pnKeyedObject/plKey.h"
#include "../pnFactory/plFactory.h"

class MaxUtilsClassDesc : public ClassDesc
{
public:
	int 			IsPublic()				{ return TRUE; }
	void*			Create(BOOL loading)	{ return &plMaxUtils::Instance(); }
	const TCHAR*	ClassName()				{ return _T("Plasma Debug Utils"); }
	SClass_ID		SuperClassID()			{ return UTILITY_CLASS_ID; }
	Class_ID 		ClassID()				{ return Class_ID(0x316610ee, 0xebe62c3); }
	const TCHAR* 	Category()				{ return _T(""); }
};

static MaxUtilsClassDesc theMaxUtilsClassDesc;
ClassDesc* GetMaxUtilsDesc() { return &theMaxUtilsClassDesc; }

plMaxUtils::plMaxUtils() : 	fhPanel(nil), fhResDlg(nil)
{
}

plMaxUtils& plMaxUtils::Instance()
{
	static plMaxUtils theInstance;
	return theInstance;
}

void plMaxUtils::BeginEditParams(Interface *ip, IUtil *iu)
{
	fhPanel = GetCOREInterface()->AddRollupPage(hInstance,
												MAKEINTRESOURCE(IDD_UTILS),
												ForwardDlgProc,
												"Plasma Debug Utils");
}

void plMaxUtils::EndEditParams(Interface *ip, IUtil *iu)
{
	GetCOREInterface()->DeleteRollupPage(fhPanel);
}

BOOL CALLBACK plMaxUtils::ForwardDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return Instance().DlgProc(hDlg, msg, wParam, lParam);
}

int ClearTextureIds();

BOOL plMaxUtils::DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_COMMAND:
		if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_RES)
		{
			int numCleared = ClearTextureIds();
			char buf[256];
			sprintf(buf, "Cleared %d texture ids", numCleared);
			MessageBox(NULL, buf, "AssetMan Clear", MB_OK);
			return TRUE;
		}
		break;
	}

	return FALSE;
}

#include "plMtlCollector.h"
#include "../MaxPlasmaMtls/Layers/plPlasmaMAXLayer.h"
#include "../../AssetMan/PublicInterface/AssManTypes.h"

int ClearTextureIds()
{
	int numCleared = 0;

	TexSet texmaps;
	plMtlCollector::GetMtls(nil, &texmaps);

	TexSet::iterator texIt = texmaps.begin();
	for (; texIt != texmaps.end(); texIt++)
	{
		Texmap* texmap = (*texIt);

		plPlasmaMAXLayer* layer = plPlasmaMAXLayer::GetPlasmaMAXLayer(texmap);
		if (layer)
		{
			int numBitmaps = layer->GetNumBitmaps();
			for (int i = 0; i < numBitmaps; i++)
			{
				jvUniqueId assetId;
				layer->GetBitmapAssetId(assetId, i);
				if (!assetId.IsEmpty())
				{
					assetId.SetEmpty();
					layer->SetBitmapAssetId(assetId, i);
					numCleared++;
				}
			}
		}
	}

	return numCleared;
}
/*
void ClearAssetsRecur(plMaxNode* node)
{
	plComponentBase* comp = node->ConvertToComponent();
	if (comp && plAudioComp::IsSoundComponent(comp))
	{
		plBaseSoundEmitterComponent* soundComp = (plBaseSoundEmitterComponent*)comp;
		soundComp->SetSoundAssetId(kBaseSound, 
			kCoverSound
)
	}
}
*/