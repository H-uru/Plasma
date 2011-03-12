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
#include "plMtlCollector.h"

#include "../MaxPlasmaMtls/Layers/plPlasmaMAXLayer.h"

#include "../MaxPlasmaMtls/Materials/plCompositeMtl.h"
#include "../MaxPlasmaMtls/Materials/plDecalMtl.h"
#include "../MaxPlasmaMtls/Materials/plMultipassMtl.h"
#include "../MaxPlasmaMtls/Materials/plParticleMtl.h"
#include "../MaxPlasmaMtls/Materials/plPassMtl.h"
#include "../MaxPlasmaMtls/Materials/plClothingMtl.h"

#include "../MaxComponent/plGUIComponents.h"
#include "../MaxComponent/pfGUISkinComp.h"
#include "../MaxComponent/plMiscComponents.h"

#include "../MaxMain/plMaxNodeBase.h"

static bool IsPlasmaMtl(Mtl *mtl)
{
	if (mtl->ClassID() == COMP_MTL_CLASS_ID ||
		mtl->ClassID() == DECAL_MTL_CLASS_ID ||
		mtl->ClassID() == MULTIMTL_CLASS_ID ||
		mtl->ClassID() == PARTICLE_MTL_CLASS_ID ||
		mtl->ClassID() == PASS_MTL_CLASS_ID ||
		mtl->ClassID() == CLOTHING_MTL_CLASS_ID)
		return true;
	return false;
}

static bool IsTexmapOK(Texmap *tex, UInt8 flags)
{
	if (flags & plMtlCollector::kPlasmaOnly && !plPlasmaMAXLayer::GetPlasmaMAXLayer(tex))
		return false;

	return true;
}

static bool IsMtlOK(Mtl *mtl, UInt8 flags)
{
	if (flags & plMtlCollector::kPlasmaOnly && !IsPlasmaMtl(mtl))
		return false;

	if (flags & plMtlCollector::kNoMultiMtl && mtl->ClassID() == MULTIMTL_CLASS_ID)
		return false;

	if (flags & plMtlCollector::kClothingMtlOnly && mtl->ClassID() != CLOTHING_MTL_CLASS_ID)
		return false;

	return true;
}

void GetMtlsRecur(MtlBase *mtlBase, MtlSet* mtls, TexSet* texmaps, UInt32 flags)
{
	if (!mtlBase)
		return;

	if (mtlBase->SuperClassID() == TEXMAP_CLASS_ID)
	{
		Texmap* tex = (Texmap*)mtlBase;
		if (texmaps && IsTexmapOK(tex, flags))
			texmaps->insert(tex);
	}
	else if(mtlBase->SuperClassID() == MATERIAL_CLASS_ID)
	{
		Mtl* mtl = (Mtl*)mtlBase;

		if (mtls && IsMtlOK(mtl, flags))
			mtls->insert(mtl);

		// Get the bitmaps from all the textures this material contains
		int i;
		int numTex = mtl->NumSubTexmaps();
		for (i = 0; i < numTex; i++)
		{
			Texmap *tex = mtl->GetSubTexmap(i);
			if (tex)
			{
				if (texmaps && IsTexmapOK(tex, flags))
					texmaps->insert(tex);
			}
		}

		// Do the same for any submtls
		if (!(flags & plMtlCollector::kNoSubMtls))
		{
			int numMtl = mtl->NumSubMtls();
			for (i = 0; i < numMtl; i++)
				GetMtlsRecur(mtl->GetSubMtl(i), mtls, texmaps, flags);
		}
	}
	else
	{
		hsAssert(0, "What kind of material is this?");
	}
}

static void GetTexmapPBs(Texmap* tex, PBSet& pbs)
{
	if (!tex)
		return;

	plPlasmaMAXLayer *plasma = plPlasmaMAXLayer::GetPlasmaMAXLayer( tex );
	if( plasma != nil )
	{
		int i;
		for( i = 0; i < plasma->GetNumBitmaps(); i++ )
		{
			PBBitmap *pbbm = plasma->GetPBBitmap( i );
			if( pbbm != nil )
				pbs.insert( pbbm );
		}
	}
	else
	{
		for (int i = 0; i < tex->NumRefs(); i++)
		{
			ReferenceTarget* r = tex->GetReference(i);
			if (r && r->SuperClassID() == PARAMETER_BLOCK2_CLASS_ID)
			{
				IParamBlock2* pb = (IParamBlock2*)r;
				for (int j = 0; j < pb->NumParams(); j++)
				{
					if (pb->GetParameterType(pb->IndextoID(j)) == TYPE_BITMAP)
					{
						PBBitmap *pbbm = pb->GetBitmap(j);
						
						if (pbbm)
							pbs.insert(pbbm);
					}
				}
			}
		}
	}
}

#include "../MaxPlasmaLights/plRealTimeLightBase.h"

static void GetNodeMtlsRecur(INode *node, MtlSet* mtls, TexSet* texmaps, UInt32 flags)
{
	Mtl *mtl = node->GetMtl();
	GetMtlsRecur(mtl, mtls, texmaps, flags);

	Object* obj = node->GetObjectRef();
	if (obj && (obj->ClassID() == RTSPOT_LIGHT_CLASSID || obj->ClassID() == RTPDIR_LIGHT_CLASSID))
	{
		Texmap* texmap = ((plRTLightBase*)obj)->GetProjMap();
		GetMtlsRecur(texmap, mtls, texmaps, flags);
	}

	plGUIControlBase *gui = plGUIControlBase::GetGUIComp( node );
	if( gui != nil )
	{
		UInt32 i;
		for( i = 0; i < gui->GetNumMtls(); i++ )
			GetMtlsRecur( gui->GetMtl( i ), mtls, texmaps, flags );
	}
	else
	{
		// Skins aren't controls
		plGUISkinComp *guiSkin = plGUISkinComp::GetGUIComp( node );
		if( guiSkin != nil )
		{
			UInt32 i;
			for( i = 0; i < guiSkin->GetNumMtls(); i++ )
				GetMtlsRecur( guiSkin->GetMtl( i ), mtls, texmaps, flags );
		}
		else
		{
			// Um, other components
			plComponentBase *base = ( ( plMaxNodeBase *)node )->ConvertToComponent();
			if( base != nil )
			{
				if( base->ClassID() == IMAGE_LIB_CID )
				{
					pfImageLibComponent *iLib = (pfImageLibComponent *)base;
					UInt32 i;
					for( i = 0; i < iLib->GetNumBitmaps(); i++ )
						GetMtlsRecur( iLib->GetBitmap( i ), mtls, texmaps, flags );
				}
			}
		}
	}

	for (int i = 0; i < node->NumberOfChildren(); i++)
		GetNodeMtlsRecur(node->GetChildNode(i), mtls, texmaps, flags);
}

static void GetEditorMtls(MtlSet* mtls, TexSet* texmaps, UInt32 flags)
{
	static const int kNumEditorSlots = 24;

	Interface *ip = GetCOREInterface();
	for (int i = 0; i < kNumEditorSlots; i++)
	{
		MtlBase *mtlBase = ip->GetMtlSlot(i);
		GetMtlsRecur(mtlBase, mtls, texmaps, flags);
	}
}

void plMtlCollector::GetMtls(MtlSet* mtls, TexSet* texmaps, UInt32 flags)
{
	Interface *ip = GetCOREInterface();

	// Make a list of all the textures from the GetSceneMtls() func
	MtlBaseLib* sceneMtls = ip->GetSceneMtls();
	for(int i = 0; i < sceneMtls->Count(); i++)
	{
		GetMtlsRecur((*sceneMtls)[i], mtls, texmaps, flags);
	}

	// Add any more we find traversing the node hierarchy
	INode *root = ip->GetRootNode();
	GetNodeMtlsRecur(root, mtls, texmaps, flags);

	if (!(flags & kUsedOnly))
		GetEditorMtls(mtls, texmaps, flags);
}

void plMtlCollector::GetMtlLayers(Mtl *mtl, LayerSet& layers)
{
	TexSet tex;
	GetMtlsRecur(mtl, nil, &tex, kPlasmaOnly);

	TexSet::iterator it = tex.begin();
	for (; it != tex.end(); it++)
	{
		layers.insert((plPlasmaMAXLayer*)*it);
	}
}

void plMtlCollector::GetAllTextures(TexNameSet& texNames)
{
	TexSet tex;
	GetMtls(nil, &tex);

	PBSet pbs;
	TexSet::iterator it = tex.begin();
	for (; it != tex.end(); it++)
		GetTexmapPBs(*it, pbs);

	PBSet::iterator pbIt = pbs.begin();
	for (; pbIt != pbs.end(); pbIt++)
	{
		PBBitmap* pbbm = *pbIt;
		texNames.insert(pbbm->bi.Name());
	}
}
