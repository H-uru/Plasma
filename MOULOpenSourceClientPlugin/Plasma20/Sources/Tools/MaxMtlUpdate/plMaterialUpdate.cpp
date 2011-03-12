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
#include "plMaterialUpdate.h"

#define MAXR3
#define MAXR4

//#include "OldMat/hsMaxMtl.h"
#include "OldMat/hsMaxLayer.h"

#include "../MaxPlasmaMtls/Layers/plLayerTex.h"
#include "../MaxPlasmaMtls/Layers/plLayerTexBitmapPB.h"

#include "../MaxPlasmaMtls/Materials/plPassMtl.h"
#include "../MaxPlasmaMtls/Materials/plPassMtlBase.h"
#include "../MaxPlasmaMtls/Materials/plPassMtlBasicPB.h"
#include "../MaxPlasmaMtls/Materials/plPassMtlAdvPB.h"
#include "../MaxPlasmaMtls/Materials/plPassMtlLayersPB.h"

#include "../MaxExport/plExportProgressBar.h"

#define PLMATERIALUPDATE_CLASS_ID	Class_ID(0x70acddfe, 0x68f42f3f)

#include <map>

class plMaterialUpdate : public UtilityObj
{
protected:
	HWND		fhPanel;
	Interface	*fInterface;
	std::map<MtlBase*, MtlBase*> fDoneMaterials;
	bool fConvertSecondLayer;

	plMaterialUpdate();

public:
	static plMaterialUpdate &Instance();

	~plMaterialUpdate();
	void DeleteThis() {}

	void BeginEditParams(Interface *ip,IUtil *iu);
	void EndEditParams(Interface *ip,IUtil *iu);

	bool ConvertAllMtls(INode *node, plExportProgressBar *bar);

protected:
	static BOOL CALLBACK DlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	void IRenameMtls(MtlBase *oldMtl, MtlBase *newMtl);

	void IConvert(INode *node);
	plPassMtl *IConvertMtl(Mtl *mtl, Mtl *multi=NULL, int subNum=-1);
	plLayerTex *IConvertLayer(hsMaxLayer *layer);

	void ICopyMtlParams(plPassMtl *mtl, hsMaxLayer *layer);
};

class plMaterialUpdateClassDesc:public ClassDesc2 {
	public:
	int 			IsPublic() { return TRUE; }
	void *			Create(BOOL loading = FALSE) { return &plMaterialUpdate::Instance(); }
	const TCHAR *	ClassName() { return "Plasma Material Converter"; }
	SClass_ID		SuperClassID() { return UTILITY_CLASS_ID; }
	Class_ID		ClassID() { return PLMATERIALUPDATE_CLASS_ID; }
	const TCHAR* 	Category() { return GetString(IDS_CATEGORY); }

	const TCHAR*	InternalName() { return _T("plMaterialConverter"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle
};

static plMaterialUpdateClassDesc plMaterialUpdateDesc;
ClassDesc2* GetMaterialUpdateDesc() { return &plMaterialUpdateDesc; }

plMaterialUpdate &plMaterialUpdate::Instance()
{
	static plMaterialUpdate theInstance;
	return theInstance;
}

plMaterialUpdate::plMaterialUpdate() : fInterface(NULL), fhPanel(NULL), fConvertSecondLayer(false)
{
}

plMaterialUpdate::~plMaterialUpdate()
{
}

BOOL plMaterialUpdate::DlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_BUTTON1 && HIWORD(wParam) == BN_CLICKED)
		{
			plMaterialUpdate &p = Instance();

			plExportProgressBar bar;
			bar.Start("Convert Materials");

			if (IsDlgButtonChecked(hWnd, IDC_CHECK1) == BST_CHECKED)
				p.fConvertSecondLayer = true;

			p.ConvertAllMtls(p.fInterface->GetRootNode(), &bar);

			p.fInterface->RedrawViews(p.fInterface->GetTime());
			p.fDoneMaterials.clear();
			return TRUE;
		}
		break;
	}

	return FALSE;
}

void plMaterialUpdate::BeginEditParams(Interface *ip, IUtil *iu) 
{
	fInterface = ip;
	fhPanel = fInterface->AddRollupPage(
		hInstance,
		MAKEINTRESOURCE(IDD_PANEL),
		DlgProc,
		GetString(IDS_PARAMS),
		0);
}
	
void plMaterialUpdate::EndEditParams(Interface *ip, IUtil *iu) 
{
	fInterface->DeleteRollupPage(fhPanel);
	fInterface = NULL;
	fhPanel = NULL;
}

bool plMaterialUpdate::ConvertAllMtls(INode *node, plExportProgressBar *bar)
{
	IConvert(node);

	bool cancel = bar->Update();
	if (cancel)
		return false;

	for (int i = 0; i < node->NumChildren(); i++)
	{
		if (!ConvertAllMtls(node->GetChildNode(i), bar))
			return false;
	}
	
	return true;
}

void plMaterialUpdate::IConvert(INode *node)
{
	Mtl *mtl = node->GetMtl();
	if (!mtl)
		return;

	if (mtl->ClassID() == hsMaxMtlClassID)
	{
		plPassMtl *pass = IConvertMtl(mtl);
		node->SetMtl(pass);
	}
	else if (mtl->ClassID() == Class_ID(MULTI_CLASS_ID,0))
	{
		for (int i = 0; i < mtl->NumSubMtls(); i++)
		{
			Mtl *subMtl = mtl->GetSubMtl(i);
			if (subMtl->ClassID() == hsMaxMtlClassID)
			{
				plPassMtl *pass = IConvertMtl(subMtl, mtl, i);
				mtl->SetSubMtl(i, pass);
			}
		}
	}
}

void plMaterialUpdate::IRenameMtls(MtlBase *oldMtl, MtlBase *newMtl)
{
	char buf[256];
	const char *name = oldMtl->GetName();
	newMtl->SetName(name);
	strcpy(buf, name);
	strcat(buf, " old");
	oldMtl->SetName(buf);
}

plPassMtl *plMaterialUpdate::IConvertMtl(Mtl *mtl, Mtl *multi, int subNum)
{
	// We've already converted this material, use the new one we already made
	if (fDoneMaterials.find(mtl) != fDoneMaterials.end())
		return (plPassMtl*)fDoneMaterials[mtl];

	plPassMtl *newMtl = (plPassMtl*)CreateInstance(MATERIAL_CLASS_ID, PASS_MTL_CLASS_ID);
	IParamBlock2 *layersPB = newMtl->GetParamBlockByID(plPassMtl::kBlkLayers);

	IRenameMtls(mtl, newMtl);

	if (mtl->NumSubTexmaps() > 0)
	{
		Texmap *map = mtl->GetSubTexmap(0);
		if (map->ClassID() == hsMaxLayerClassID)
		{
			plLayerTex *layer = IConvertLayer((hsMaxLayer*)map);

//			layer->SetMtlFlag(MTL_TEX_DISPLAY_ENABLED, TRUE);
//			layer->ActivateTexDisplay(TRUE);
			
			newMtl->SetSubTexmap(0, layer);
/*			newMtl->SetActiveTexmap(layer);
			newMtl->SetMtlFlag(MTL_TEX_DISPLAY_ENABLED, TRUE);

			if (multi)
				GetCOREInterface()->ActivateTexture(layer, multi, subNum);
			else
				GetCOREInterface()->ActivateTexture(layer, newMtl);
*/
			ICopyMtlParams(newMtl, (hsMaxLayer*)map);
		}
	}

	if (mtl->NumSubTexmaps() > 1 && fConvertSecondLayer && mtl->SubTexmapOn(1))
	{
		Texmap *map = mtl->GetSubTexmap(1);
		if (map->ClassID() == hsMaxLayerClassID)
		{
			hsMaxLayer *oldLayer = (hsMaxLayer*)map;
			plLayerTex *layer = IConvertLayer(oldLayer);
			newMtl->SetSubTexmap(1, layer);

			IParamBlock2 *layersPB = newMtl->GetParamBlockByID(plPassMtl::kBlkLayers);
			layersPB->SetValue(kPassLayTopOn, 0, TRUE);

			if (oldLayer->GetBlendFlags() & hsMaxLayerBase::kBlendAlpha)
				layersPB->SetValue(kPassLayBlend, 0, plPassMtlBase::kBlendAlpha);
			else if (oldLayer->GetBlendFlags() & hsMaxLayerBase::kBlendAdd)
				layersPB->SetValue(kPassLayBlend, 0, plPassMtlBase::kBlendAdd);
		}
	}

//MtlBaseLib& Interface::GetMaterialLibrary()
	
	// Add this to our converted materials
	fDoneMaterials[mtl] = newMtl;
		
	return newMtl;
}

plLayerTex *plMaterialUpdate::IConvertLayer(hsMaxLayer *layer)
{
	plLayerTex *newLayer = (plLayerTex*)CreateInstance(TEXMAP_CLASS_ID, LAYER_TEX_CLASS_ID);
	IParamBlock2 *bitmapPB = newLayer->GetParamBlockByID(plLayerTex::kBlkBitmap);

	IRenameMtls(layer, newLayer);

	// Copy the bitmap
	if (layer->GetMiscFlags() & hsMaxLayerBase::kMiscUseBitmap)
	{
		bitmapPB->SetValue(kBmpUseBitmap, 0, 1);

		const char *name = layer->GetMapName();
		PBBitmap pbb;
		pbb.bi.SetName(name);

		// Disable annoying missing texture warning
		BOOL bmmSilentMode = TheManager->SilentMode();
		TheManager->SetSilentMode(TRUE);

		bitmapPB->SetValue(kBmpBitmap, 0, &pbb);
	
		TheManager->SetSilentMode(bmmSilentMode);
	}

	// Copy the UVGen
	newLayer->ReplaceReference(plLayerTex::kRefUVGen, layer->GetUVGen());

	// Copy the cropping
	if (layer->GetApplyCrop())
	{
		bitmapPB->SetValue(kBmpApply, 0, TRUE);
		bitmapPB->SetValue(kBmpCropPlace, 0, layer->GetPlaceImage());

		bitmapPB->SetValue(kBmpClipU, 0, layer->GetClipU(0));
		bitmapPB->SetValue(kBmpClipV, 0, layer->GetClipV(0));
		bitmapPB->SetValue(kBmpClipW, 0, layer->GetClipW(0));
		bitmapPB->SetValue(kBmpClipH, 0, layer->GetClipH(0));
	}

	// Misc
	if (layer->GetBlendFlags() & hsMaxLayerBase::kBlendNoColor)
		bitmapPB->SetValue(kBmpDiscardColor, 0, TRUE);
	if (layer->GetBlendFlags() & hsMaxLayerBase::kBlendInvertColor)
		bitmapPB->SetValue(kBmpInvertColor, 0, TRUE);
	if (layer->GetAlphaSource() == 2)
		bitmapPB->SetValue(kBmpDiscardAlpha, 0, TRUE);
	if (layer->GetBlendFlags() & hsMaxLayerBase::kBlendInvertAlpha)
		bitmapPB->SetValue(kBmpInvertAlpha, 0, TRUE);

/*
	// Texture quality
	kBmpNonCompressed,
	kBmpScaling,
*/

	// Mipmap
	if (layer->GetFilterType() == 2)
		bitmapPB->SetValue(kBmpNoFilter, 0, TRUE);

	float blur = layer->GetMipMapBlur(TimeValue(0));
	bitmapPB->SetValue(kBmpMipBlur, 0, blur);
	
	if (layer->GetZFlags() & hsMaxLayerBase::kZLODBias)
	{
		bitmapPB->SetValue(kBmpMipBias, 0, TRUE);
		bitmapPB->SetValue(kBmpMipBiasAmt, 0, layer->GetLODBias(TimeValue(0)));
	}
	
	// Detail
	if (layer->GetBlendFlags() & hsMaxLayerBase::kBlendDetail ||
		layer->GetBlendFlags() & hsMaxLayerBase::kBlendDetailAdd)
	{
		bitmapPB->SetValue(kBmpUseDetail, 0, TRUE);

		bitmapPB->SetValue(kBmpDetailStartSize, 0, layer->GetDetailDropoffStart(0));
		bitmapPB->SetValue(kBmpDetailStopSize, 0, layer->GetDetailDropoffStop(0));
		bitmapPB->SetValue(kBmpDetailStartOpac, 0, layer->GetDetailMax(0));
		bitmapPB->SetValue(kBmpDetailStopOpac, 0, layer->GetDetailMin(0));
	}

	return newLayer;
}

void plMaterialUpdate::ICopyMtlParams(plPassMtl *mtl, hsMaxLayer *layer)
{
	IParamBlock2 *basicPB = mtl->GetParamBlockByID(plPassMtl::kBlkBasic);
	IParamBlock2 *layersPB = mtl->GetParamBlockByID(plPassMtl::kBlkLayers);

	basicPB->SetValue(kPassBasColorAmb, 0, layer->GetAmbient());
	basicPB->SetValue(kPassBasColor, 0, layer->GetColor());

	basicPB->SetValue(kPassBasOpacity, 0, int(layer->GetOpacity(0)*100.f));

	if (layer->GetBlendFlags() & hsMaxLayerBase::kBlendAlpha)
		layersPB->SetValue(kPassLayOutputBlend, 0, plPassMtlBase::kBlendAlpha);
	else if (layer->GetBlendFlags() & hsMaxLayerBase::kBlendAdd)
		layersPB->SetValue(kPassLayOutputBlend, 0, plPassMtlBase::kBlendAdd);

/*
	kPassAdvUseSpec,
	kPassAdvSpecType,
	kPassAdvShine,
	kPassAdvShineStr,

	// Misc
	kPassAdvWire,
	kPassAdvMeshOutlines,
	kPassAdvTwoSided,

	// Shading
	kPassAdvSoftShadow,
	kPassAdvNoProj,
	kPassAdvVertexShade,
	kPassAdvNoShade,
	kPassAdvNoFog,
	kPassAdvWhite,

	// Z
	kPassAdvZOnly,
	kPassAdvZClear,
	kPassAdvZNoRead,
	kPassAdvZNoWrite,
	kPassAdvZInc,
*/
}
