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
#include "hsWindows.h"
#include <commdlg.h>
#include <math.h>
#include <float.h>

#include "Max.h"
#include "stdmat.h"
#include "bmmlib.h"
#include "istdplug.h"
#include "texutil.h"

#include "hsMaterialConverter.h"
#include "plLayerConverter.h"
#include "../MaxComponent/plMaxAnimUtils.h"
#include "../plResMgr/plKeyFinder.h"
#include "hsResMgr.h"
#include "../pnKeyedObject/plUoid.h"
#include "hsUtils.h"
#include "hsMaxLayerBase.h"
#include "../MaxExport/plErrorMsg.h"
#include "../plSurface/hsGMaterial.h"
#include "../pnSceneObject/plSceneObject.h"
#include "UserPropMgr.h"
#include "../plFile/plFileUtils.h"

#include "hsConverterUtils.h"
#include "hsControlConverter.h"
#include "../MaxMain/plMaxNode.h"

#include "../plInterp/plController.h"
#include "hsExceptionStack.h"
#include "hsStringTokenizer.h"
#include "../plSurface/plLayerInterface.h"
#include "../plSurface/plLayer.h"
#include "../plSurface/plLayerAnimation.h"
#include "../plGImage/plMipmap.h"

#include "plgDispatch.h"

#include "../pnMessage/plRefMsg.h"
#include "../pnKeyedObject/plKey.h"
#include "../pnKeyedObject/plKeyImp.h"

#include "plBitmapCreator.h"

#include "../plMessage/plMatRefMsg.h"
#include "../plMessage/plLayRefMsg.h"
#include "../pnMessage/plObjRefMsg.h"
#include "../pnMessage/plNodeRefMsg.h"
#include "../pfMessage/plClothingMsg.h"

#include "../MaxPlasmaMtls/Materials/plMultipassMtlPB.h"
#include "../MaxPlasmaMtls/Materials/plCompositeMtlPB.h"
#include "../MaxPlasmaMtls/Materials/plPassMtl.h"
#include "../MaxPlasmaMtls/Materials/plMultipassMtl.h"
#include "../MaxPlasmaMtls/Materials/plDecalMtl.h"
#include "../MaxPlasmaMtls/Materials/plCompositeMtl.h"
#include "../MaxPlasmaMtls/Materials/plParticleMtl.h"
#include "../MaxPlasmaMtls/Materials/plBumpMtl.h"
#include "../MaxPlasmaMtls/Materials/plPassMtlBase.h"
#include "../MaxPlasmaMtls/Materials/plAnimStealthNode.h"
#include "../MaxPlasmaMtls/Layers/plPlasmaMAXLayer.h"
#include "../MaxPlasmaMtls/Layers/plLayerTex.h"
#include "../MaxPlasmaMtls/Layers/plLayerTexBitmapPB.h"

#include "../pfSurface/plLayerAVI.h"
#include "../pfSurface/plLayerBink.h"

#include "../MaxComponent/plLightMapComponent.h"
#include "../plDrawable/plGeometrySpan.h"

#include "../MaxPlasmaMtls/Materials/plClothingMtl.h"
#include "../plAvatar/plAvatarClothing.h"
#include "../plAvatar/plClothingLayout.h"
#include "../plSDL/plSDL.h"
#include "../plSDL/plSDLDescriptor.h"

extern UserPropMgr gUserPropMgr;

static const char* kSecretBumpSign = "~~~";

namespace {
	const int kDefaultDetailBias=5;

	void CopyMaterialLODToTextures(hsGMaterial* mat)
	{
		Int32 i;
		for (i = 0; i < mat->GetNumLayers(); ++i)
		{
			plLayerInterface* layer = mat->GetLayer(i);

/*			Textures? What textures?
			if (layer->GetTexture())
			{
				hsGTexture* texture = layer->GetTexture();
				texture->SetLOD(texture->GetLOD() | mat->GetLOD());
			}
*/
		}
	}

	const char	sWarnBaseTextureMissing[] = "The object \"%s\"'s material has a base layer that is assigned texture \"%s\", but the texture file is missing. "
										"This can cause unwanted effects during runtime."; 
	const char	sWarnUpperTextureMissing[] = "The object \"%s\"'s material has an upper layer that is assigned texture \"%s\", but the texture file is missing. "
										"This is not supported in the engine, so the upper layer will be ignored."; 
	const char	sWarnNoUpperTexture[] = "The object \"%s\"'s material has an uppper layer that is not assigned a texture. "
										"This is not supported in the engine, so the upper layer will be disabled."; 
}

static UInt32 MakeUInt32Color(float r, float g, float b, float a)
{
	return (UInt32(a * 255.9f) << 24)
			|(UInt32(r * 255.9f) << 16)
			|(UInt32(g * 255.9f) << 8)
			|(UInt32(b * 255.9f) << 0);
}

static bool failedRT = false;
static bool failedNumUV = false;
static bool failedAlphaLayer = false;
static bool failedFade = false;
static int dupCuzRT = 0;
static int dupCuzNumUV = 0;
static int dupCuzAlphaLayer = 0;
static int dupCuzFade = 0;

hsMaterialConverter& hsMaterialConverter::Instance()
{
	hsGuardBegin("hsMaterialConverter::Instance");
	static hsMaterialConverter the_instance;

	return the_instance;
	hsGuardEnd; 
}

hsMaterialConverter::hsMaterialConverter() :
fSave(true),
fNodeName(nil),
fWarned(0),
fInterface(nil),
fConverterUtils(hsConverterUtils::Instance()),
fChangedTimes(false)
{
	hsGuardBegin("hsMaterialConverter::hsMaterialConverter");

	hsGuardEnd;
}

hsMaterialConverter::~hsMaterialConverter()
{
	hsGuardBegin("hsMaterialConverter::~hsMaterialConverter");
	hsAssert(fDoneMaterials.Count() == 0, "FreeMaterialCache not called");
	hsGuardEnd;
}

void hsMaterialConverter::Init(hsBool save, plErrorMsg *msg)
{
	hsGuardBegin("hsMaterialConverter::Init");

	fInterface = GetCOREInterface();
	fSave = save;
	fErrorMsg = msg;

	fSubIndex = -1;
	fNodeName = nil;

	fWarned = true;

	fLastMaterial.fHsMaterial = nil;
	fLastMaterial.fMaxMaterial = nil;
	fLastMaterial.fMaxMaterial = nil;
	fLastMaterial.fSubMultiMat = false;
	fLastMaterial.fOwnedCopy = false;

	failedRT = false;
	failedNumUV = false;
	failedAlphaLayer = false;
	failedFade = false;
	dupCuzRT = 0;
	dupCuzNumUV = 0;
	dupCuzAlphaLayer = 0;
	dupCuzFade = 0;

	hsGuardEnd;
}

void hsMaterialConverter::FreeMaterialCache(const char* path)
{
	if( path && *path )
		IGenMaterialReport(path);

	for (int i = 0; i < fDoneMaterials.Count(); i++)
		fDoneMaterials[i].fHsMaterial->GetKey()->UnRefObject();

	fDoneMaterials.Reset();
}

hsBool hsMaterialConverter::ForceNoUvsFlatten(plMaxNode* node)
{
	hsGuardBegin("hsMaterialConverter::ForceNoUvsFlatten");
	return IsMultiMat(GetBaseMtl(node));
	hsGuardEnd; 
}

hsBool hsMaterialConverter::PreserveUVOffset(Mtl* mtl)
{
	hsGuardBegin("hsMaterialConverter::PreserveUVOffset");

	if (!mtl || (!IsHsMaxMat(mtl) && !IsDecalMat(mtl)))
		return true;

	//IParamBlock2 *pblock = mtl->GetParamBlockByID(plPassMtl::kBlkLayers);
	//if (!pblock)
	//	return true;

	plPassMtlBase *currMtl = (plPassMtlBase *)mtl;
	for (int i = 0; i < mtl->NumSubTexmaps(); i++) 
	{
		//if (i == 1 && !pblock->GetInt(kPassLayTopOn))
		if (i == 1 && !currMtl->GetTopLayerOn())
			continue;

		Texmap *texMap = mtl->GetSubTexmap(i);

		if (!texMap || texMap->ClassID() != LAYER_TEX_CLASS_ID)
			continue;
		
		StdUVGen* uvGen = (StdUVGen*)((plLayerTex*)texMap)->GetTheUVGen();
		int tiling = uvGen->GetTextureTiling();
		if( !(tiling & U_WRAP) || !(tiling & V_WRAP) )
			return true;

		if (IHasAnimatedControllers(uvGen))
			return true;
//		if (ITextureTransformIsAnimated(texMap))
//			return true;
		
	}

	return false;

	hsGuardEnd; 
}

void AttachLinkMtlAnims(plMaxNode *node, hsGMaterial *mat)
{
	const int numKeys = 2;
	hsScalar times[] = {0.f, 1.5f};
	hsScalar values[numKeys] = {100.f, 0.f};
	hsBool leaving[] = {true, false};
	char *animName = "_link_anim";

	int k;
	for (k = 0; k < mat->GetNumLayers(); k++)
	{
		plLayerInterface *oldLayer, *currLayer;
		oldLayer = currLayer = mat->GetLayer(k);
		plLeafController *opaCtl;
		plLayerLinkAnimation* animLayer;

		char suff[10];
		sprintf(suff, "%d", k);
		
		opaCtl = TRACKED_NEW plLeafController;
		opaCtl->QuickScalarController(numKeys, times, values, sizeof(hsScalar));
		animLayer = TRACKED_NEW plLayerLinkAnimation;
		animLayer->SetLinkKey(node->GetAvatarSO()->GetKey());
		//animLayer->fLeavingAge = leaving[x];
		TSTR fullAnimName = TSTR(oldLayer->GetKeyName()) + TSTR("_") + TSTR(animName) + TSTR("_") + TSTR(suff);
		hsgResMgr::ResMgr()->NewKey(fullAnimName, animLayer, node->GetLocation());
		animLayer->SetOpacityCtl(opaCtl);
		animLayer->GetTimeConvert().SetBegin(times[0]);
		animLayer->GetTimeConvert().SetEnd(times[1]);
		animLayer->GetTimeConvert().Stop(true);
		animLayer->AttachViaNotify(currLayer);
		currLayer = animLayer;
		
		plMatRefMsg* msg = TRACKED_NEW plMatRefMsg(mat->GetKey(), plRefMsg::kOnReplace, k, plMatRefMsg::kLayer);
		msg->SetOldRef(oldLayer);
		msg->SetRef(currLayer);
		hsgResMgr::ResMgr()->AddViaNotify(msg, plRefFlags::kActiveRef);
	}
}

UInt32 hsMaterialConverter::ColorChannelsUseMask(plMaxNode* node, int iSubMtl)
{
	UInt32 usedChan = 0;
	hsBool deleteIt = false;

	TriObject* triObj = node->GetTriObject(deleteIt);
	if( triObj )
	{
		Mesh* mesh = &(triObj->mesh);
	
		UVVert* alphaMap = mesh->mapVerts(MAP_ALPHA);	
		int numAlphaVerts = mesh->getNumMapVerts(MAP_ALPHA);

		UVVert* illumMap = mesh->mapVerts(MAP_SHADING);
		int numIllumVerts = mesh->getNumMapVerts(MAP_SHADING);

		Point3* colorMap = mesh->vertCol;
		int numColorVerts = mesh->numCVerts;

		TVFace* colorFaces = mesh->vcFace;
		TVFace* illumFaces = mesh->mapFaces(MAP_SHADING);
		TVFace* alphaFaces = mesh->mapFaces(MAP_ALPHA);

		int numFaces = mesh->getNumFaces();
		int i;
		for( i = 0; i < numFaces; i++ )
		{
			Face		*face = &mesh->faces[ i ];

			if( (iSubMtl >= 0) && (face->getMatID() != iSubMtl) )
				continue;

			if( colorFaces && colorMap )
			{
				TVFace* colorFace = colorFaces + i;
				usedChan |= ICheckPoints(colorMap[colorFace->getTVert(0)],
										colorMap[colorFace->getTVert(1)],
										colorMap[colorFace->getTVert(2)],
										0,
										kColorRedBlack, kColorRedGrey, kColorRedWhite);
				usedChan |= ICheckPoints(colorMap[colorFace->getTVert(0)],
										colorMap[colorFace->getTVert(1)],
										colorMap[colorFace->getTVert(2)],
										1,
										kColorGreenBlack, kColorGreenGrey, kColorGreenWhite);
				usedChan |= ICheckPoints(colorMap[colorFace->getTVert(0)],
										colorMap[colorFace->getTVert(1)],
										colorMap[colorFace->getTVert(2)],
										2,
										kColorBlueBlack, kColorBlueGrey, kColorBlueWhite);
			}
			if( illumFaces && illumMap )
			{
				TVFace* illumFace = illumFaces + i;
				usedChan |= ICheckPoints(illumMap[illumFace->getTVert(0)],
										illumMap[illumFace->getTVert(1)],
										illumMap[illumFace->getTVert(2)],
										0,
										kIllumRedBlack, kIllumRedGrey, kIllumRedWhite);
				usedChan |= ICheckPoints(illumMap[illumFace->getTVert(0)],
										illumMap[illumFace->getTVert(1)],
										illumMap[illumFace->getTVert(2)],
										1,
										kIllumGreenBlack, kIllumGreenGrey, kIllumGreenWhite);
				usedChan |= ICheckPoints(illumMap[illumFace->getTVert(0)],
										illumMap[illumFace->getTVert(1)],
										illumMap[illumFace->getTVert(2)],
										2,
										kIllumBlueBlack, kIllumBlueGrey, kIllumBlueWhite);
			}
			if( alphaFaces && alphaMap )
			{
				TVFace* alphaFace = alphaFaces + i;
				usedChan |= ICheckPoints(alphaMap[alphaFace->getTVert(0)],
										alphaMap[alphaFace->getTVert(1)],
										alphaMap[alphaFace->getTVert(2)],
										0,
										kAlphaBlack, kAlphaGrey, kAlphaWhite);
			}
		}

		if( deleteIt )
			triObj->DeleteThis();
	}

	return usedChan;
}

UInt32 hsMaterialConverter::ICheckPoints(const Point3& p0, const Point3& p1, const Point3& p2,
										 int chan,
										 UInt32 mBlack, UInt32 mGrey, UInt32 mWhite)
{
	const float kSmall = 1.e-3f;
	if( (p0[chan] < kSmall) && (p1[chan] < kSmall) && (p2[chan] < kSmall) )
		return mBlack;

	if( (1.f - p0[chan] < kSmall) && (1.f - p1[chan] < kSmall) && (1.f - p2[chan] < kSmall) )
		return mWhite;

	return mGrey;
}

UInt32 hsMaterialConverter::ICheckPoints(const Point3& p0, const Point3& p1, const Point3& p2, const Point3& p3,
										 int chan,
										 UInt32 mBlack, UInt32 mGrey, UInt32 mWhite)
{
	const float kSmall = 1.e-3f;
	if( (p0[chan] < kSmall) && (p1[chan] < kSmall) && (p2[chan] < kSmall) && (p3[chan] < kSmall) )
		return mBlack;

	if( (1.f - p0[chan] < kSmall) && (1.f - p1[chan] < kSmall) && (1.f - p2[chan] < kSmall) && (1.f - p3[chan] < kSmall) )
		return mWhite;

	return mGrey;
}

int hsMaterialConverter::NumVertexOpacityChannelsRequired(plMaxNode* node, int iSubMtl)
{

	UInt32 vtxChanMask = VertexChannelsRequiredMask(node, iSubMtl);

	// Now count the bits.
	int numChan;
	for( numChan = 0; vtxChanMask; vtxChanMask >>= 1 )
	{
		numChan += (vtxChanMask & 0x1);
	}
	return numChan;
}

UInt32 hsMaterialConverter::VertexChannelsRequiredMask(plMaxNode* node, int iSubMtl)
{
	Mtl* mtl = node->GetMtl();
	if( !mtl )
		return 0;

	// We use iSubMtl < 0 here as a request for all submtls used by node.
	if( IsMultiMat(mtl) && (iSubMtl >= 0) )
	{
		if( iSubMtl < 0 )
			iSubMtl = 0;
		while( IsMultiMat(mtl) )
		{
			if( iSubMtl >= mtl->NumSubMtls() )
				iSubMtl = mtl->NumSubMtls() - 1;
			mtl = mtl->GetSubMtl(iSubMtl);
		}
	}
	else
	{
		iSubMtl = -1;
	}

	// Get the channels our materials will look at. These should all
	// be of the grey variety, since a solid opaque can be drawn as is,
	// and a solid transparent can be pitched.
	UInt32 vtxChanReq = VertexChannelsRequestMask(node, iSubMtl, mtl);

	// Or in vtx alpha. If it's been set, we need to respect it, if it hasn't,
	// it'll and out to zero anyway when we & with the vtxChanUsed.
	vtxChanReq |= kAlphaGrey;

	// Now figure out what's in all the channels, which are solid black, which
	// actually have interesting values etc.
	UInt32 vtxChanUsed = ColorChannelsUseMask(node, iSubMtl);

	UInt32 vtxChanMask = vtxChanReq & vtxChanUsed;

	return vtxChanMask;
}

UInt32 hsMaterialConverter::VertexChannelsRequestMask(plMaxNode* node, int iSubMtl, Mtl* mtl)
{
	if( !mtl )
		return 0;

	UInt32 vtxChanMask = 0;

	if( IsMultiMat(mtl) && (iSubMtl >= 0) )
	{
		while( IsMultiMat(mtl) )
		{
			if( iSubMtl >= mtl->NumSubMtls() )
				iSubMtl = mtl->NumSubMtls() - 1;
			mtl = mtl->GetSubMtl(iSubMtl);
		}
	}

	if( IsMultiMat(mtl) || IsMultipassMat(mtl) )
	{
		int i;

		for( i = 0; i < mtl->NumSubMtls(); i++ )
			vtxChanMask |= VertexChannelsRequestMask(node, iSubMtl, mtl->GetSubMtl(i));

		return vtxChanMask;
	}

	if( IsCompositeMat(mtl) )
	{
		vtxChanMask |= VertexChannelsRequestMask(node, iSubMtl, mtl->GetSubMtl(0));

		int i;
		for( i = 1; i < mtl->NumSubMtls(); i++ )
		{
			plCompositeMtl* comp = (plCompositeMtl*)mtl;
			int myBlend = comp->GetBlendStyle(i);
			switch( myBlend )
			{
			case plCompositeMtl::kCompBlendVertexAlpha:
			case plCompositeMtl::kCompBlendInverseVtxAlpha:
				vtxChanMask |= kAlphaGrey;
				break;
			case plCompositeMtl::kCompBlendVertexIllumRed:
			case plCompositeMtl::kCompBlendInverseVtxIllumRed:
				vtxChanMask |= kIllumRedGrey;
				break;
			case plCompositeMtl::kCompBlendVertexIllumGreen:
			case plCompositeMtl::kCompBlendInverseVtxIllumGreen:
				vtxChanMask |= kIllumGreenGrey;
				break;
			case plCompositeMtl::kCompBlendVertexIllumBlue:
			case plCompositeMtl::kCompBlendInverseVtxIllumBlue:
				vtxChanMask |= kIllumBlueGrey;
				break;
			default:
				hsAssert(false, "Ooops, new composite blends?");
				break;
			}
		}
	}

	if( IsHsMaxMat(mtl) )
	{
		plPassMtl* passMtl = (plPassMtl*)mtl;
		if( plPassMtlBase::kBlendAlpha == passMtl->GetOutputBlend() )
			vtxChanMask |= kAlphaGrey;
	}

	return vtxChanMask;
}


//
// What kind of material am I??
//
hsBool hsMaterialConverter::IsMultiMat(Mtl *m) 
{
	hsGuardBegin("hsMaterialConverter::IsMultiMat");

	if (m == NULL)
	{
		return false;
	}

	return (m->ClassID() == Class_ID(MULTI_CLASS_ID,0));
	hsGuardEnd; 
}

hsBool hsMaterialConverter::IsHsMaxMat(Mtl *m)
{
	hsGuardBegin("hsMaterialConverter::IsHsMaxMat");

	if (m==NULL)
	{
		return false;
	}

	return (m->ClassID() == PASS_MTL_CLASS_ID);
	hsGuardEnd; 
}

hsBool hsMaterialConverter::IsMultipassMat(Mtl *m)
{
	hsGuardBegin("hsMaterialConverter::IsHsMaxMat");

	if (m==NULL)
	{
		return false;
	}

	return (m->ClassID() == MULTIMTL_CLASS_ID);
	hsGuardEnd; 
}

hsBool hsMaterialConverter::IsDecalMat(Mtl *m)
{
	hsGuardBegin("hsMaterialConverter::IsDecalMat");

	if (m==NULL)
	{
		return false;
	}

	return (m->ClassID() == DECAL_MTL_CLASS_ID);
	hsGuardEnd; 
}

hsBool hsMaterialConverter::IsCompositeMat(Mtl *m)
{
	hsGuardBegin("hsMaterialConverter::IsCompositeMat");

	if (m==NULL)
	{
		return false;
	}

	return (m->ClassID() == COMP_MTL_CLASS_ID);
	hsGuardEnd; 
}

hsBool hsMaterialConverter::IsParticleMat(Mtl *m)
{
	hsGuardBegin("hsMaterialConverter::IsParticleMat");

	if (m==NULL)
	{
		return false;
	}

	return (m->ClassID() == PARTICLE_MTL_CLASS_ID);
	hsGuardEnd;
}

hsBool hsMaterialConverter::IsClothingMat(Mtl *m)
{
	hsGuardBegin("hsMaterialConverter::IsClothingMat");

	if (m==NULL)
	{
		return false;
	}

	return (m->ClassID() == CLOTHING_MTL_CLASS_ID);
	hsGuardEnd;
}

hsBool hsMaterialConverter::IsTwoSided(Mtl* m, int iSubMtl)
{
	if( !m )
		return false;

	return 0 != (m->Requirements(iSubMtl) & MTLREQ_2SIDE);
}

hsBool hsMaterialConverter::HasAnimatedTextures(Texmap* texMap)
{
	hsGuardBegin("hsMaterialConverter::HasAnimatedTextures");
	return false;
	hsGuardEnd; 
}

hsBool hsMaterialConverter::IsAVILayer(Texmap*  texMap)
{
	hsGuardBegin("hsMaterialConverter::IsAVILayer");
	return false;
	hsGuardEnd; 
}

hsBool hsMaterialConverter::IsQTLayer(Texmap*  texMap)
{
	hsGuardBegin("hsMaterialConverter::IsQTLayer");
	return false;
	hsGuardEnd; 
}

hsBool hsMaterialConverter::IsBinkLayer(Texmap*  texMap)
{
	hsGuardBegin("hsMaterialConverter::IsBinkLayer");
	return false;
	hsGuardEnd; 
}



// MAXR3 broke GetCoordMapping, here's a work around which
// works for everything except "Texture - Planar from Object XYZ"
int hsMaterialConverter::GetCoordMapping(StdUVGen *uvgen)
{
	hsGuardBegin("hsMaterialConverter::GetCoordMapping");

	// Try Requirements, as StdUV does override this...
	ULONG reqs = 0l;
	reqs = uvgen->Requirements(0);
	if (reqs & MTLREQ_VIEW_DEP || reqs == 0) 
	{
		return (uvgen->GetCoordMapping(0));
	} 
	else 
	{
		return (UVMAP_EXPLICIT);
	}

	hsGuardEnd; 
}

static void IGetNodesByMaterialRecur(plMaxNode* node, Mtl *mtl, hsTArray<plMaxNode*> &out)
{
	if (node)
	{
		if (node->GetMtl() == mtl)
			out.Append(node);

		int numChildren = node->NumberOfChildren();
		for (int i = 0; i < numChildren; i++)
		{
			IGetNodesByMaterialRecur((plMaxNode*)node->GetChildNode(i), mtl, out);
		}
	}
}

void hsMaterialConverter::GetNodesByMaterial(Mtl *mtl, hsTArray<plMaxNode*> &out)
{
	IGetNodesByMaterialRecur((plMaxNode*)GetCOREInterface()->GetRootNode(), mtl, out);
}

//	   Ok, Max gives us Multi-materials which it knows are just collections of independent materials. This function
// should never be called with a multi-mat. Instead, the multi-mat should call this for each one of its 
// sub-materials, and build up an array of these arrays.
//     This function should be called on anything that Max thinks is a single material, even if the exporter
// wants to generate several materials from it (like composites).

hsTArray<plExportMaterialData> *
hsMaterialConverter::CreateMaterialArray(Mtl *maxMaterial, plMaxNode *node, UInt32 multiIndex)
{
	hsTArray<plExportMaterialData> *ourMaterials = TRACKED_NEW hsTArray<plExportMaterialData>;

	const char* dbgNodeName = node->GetName();

	int numUVChannels = node->NumUVWChannels();
	hsBool makeAlphaLayer = node->AlphaHackLayersNeeded(multiIndex) > 0;

	hsBool enviro = fConverterUtils.IsEnvironHolder(node);
	
	TSTR name;
	if (maxMaterial)
		name = maxMaterial->GetName();
	else
		name = "nil";
	
	/// Get the material
	hsBool isMultiMat = IsMultiMat( maxMaterial );
	if (isMultiMat)
	{
		if (fErrorMsg->Set(!(fWarned & kWarnedSubMulti), node->GetName(), "Multi-material in CreateMaterialArray (Multi child of multi?) on mat %s. Using the first sub-material instead.", 
			maxMaterial->GetName()).CheckAskOrCancel() )
			fWarned |= kWarnedSubMulti;
		maxMaterial = maxMaterial->GetSubMtl(0);
	}
		
	hsBool isMultipassMat = IsMultipassMat( maxMaterial );
	hsBool isComposite = IsCompositeMat( maxMaterial );
	
	int i;
	int numMaterials = 1;
	int numBlendChannels = 0;
	if (isComposite)
	{
		int numBlendMaterials = maxMaterial->NumSubMtls() - 1; // no blend for the base Mtl

		// This will be one or zero, depending on blend modes, and whether we can use the lighting equation
		// with vertex alpha.
		int maxBlendChannel = plGeometrySpan::kMaxNumUVChannels - 1 - numUVChannels;

		IParamBlock2 *pb = maxMaterial->GetParamBlockByID(plCompositeMtl::kBlkPasses);
		for (i = 0; i < numBlendMaterials; i++)
		{
			int curr = 0;
			if (curr > maxBlendChannel)
			{
				curr = maxBlendChannel;
			}
			pb->SetValue(kCompUVChannels, 0, (int)(numUVChannels + curr), i); // label each layer's opacity UV mapping
		}
		numMaterials = (1 << maxMaterial->NumSubMtls()) - 1; // 2^n - 1 possibilites

		ourMaterials->Reset();
		for( i = 0; i < numMaterials; i++ ) // would be smarter to only create the materials we'll actually use
		{
			plExportMaterialData emd;
			emd.fMaterial = ICreateMaterial(maxMaterial, node, name, i + 1, numUVChannels, makeAlphaLayer);
			emd.fNumBlendChannels = (emd.fMaterial != nil && emd.fMaterial->NeedsBlendChannel() ? 1 : 0);

			// A bump layer requires 2 generated uv channels
			if( HasBumpLayer(node, maxMaterial) )
				emd.fNumBlendChannels += 2;

			if (numBlendChannels < emd.fNumBlendChannels)
				numBlendChannels = emd.fNumBlendChannels;
			ourMaterials->Append(emd);
		}
	}
	else // plPassMtl, plDecalMat, plMultiPassMtl, plParticleMtl
	{
		hsGMaterial *mat = ICreateMaterial(maxMaterial, node, name, -1, numUVChannels, makeAlphaLayer);
		int maxLayer = (mat == nil ? 0 : mat->GetNumLayers());

		numBlendChannels = (mat != nil && mat->NeedsBlendChannel() ? 1 : 0);

		// A bump layer requires 2 generated uv channels
		if( HasBumpLayer(node, maxMaterial) )
			numBlendChannels += 2;

		plExportMaterialData emd;
		emd.fMaterial = mat;
		emd.fNumBlendChannels = numBlendChannels;
		ourMaterials->Reset();
		ourMaterials->Append(emd);
	}

	// We've already handled it... just letting them know.
	if (fErrorMsg->Set( (numUVChannels + numBlendChannels > plGeometrySpan::kMaxNumUVChannels) && !(fWarned & kWarnedTooManyUVs), node->GetName(), 
						"Material wants %d UV channels for textures and blending, but only %d are available."
						" Some layers will have incorrect channels assigned.",
						numUVChannels + numBlendChannels, plGeometrySpan::kMaxNumUVChannels).CheckAskOrCancel() )
		fWarned |= kWarnedTooManyUVs; 

	return ourMaterials;
}

int hsMaterialConverter::MaxUsedUVWSrc(plMaxNode* node, Mtl* mtl)
{
	const char* dbgNodeName = node->GetName();

	if( !mtl )
		return 0;

	if( IsMultiMat(mtl) || IsMultipassMat(mtl) || IsCompositeMat(mtl) )
	{
		int i;

		int numUVWs = 0;
		for( i = 0; i < mtl->NumSubMtls(); i++ )
		{
			int num = MaxUsedUVWSrc(node, mtl->GetSubMtl(i));
			if( num > numUVWs )
				numUVWs = num;
		}
		return numUVWs;
	}

	if( IsParticleMat(mtl) )
	{
		return 1;
	}

	if( IsHsMaxMat(mtl) || IsDecalMat(mtl) || IsBumpMtl(mtl) )
	{
		plPassMtlBase* passMtl = (plPassMtlBase*)mtl;

		int numUVWs = 0;

		Texmap* tex = passMtl->GetBaseLayer();
		if( tex )
		{
			if( tex->GetUVWSource() == UVWSRC_EXPLICIT )
			{
				if( tex->GetMapChannel() > numUVWs )
					numUVWs = tex->GetMapChannel();

				if( passMtl->GetTopLayerOn() && passMtl->GetTopLayer() )
				{
					tex = passMtl->GetTopLayer();
					if( tex->GetMapChannel() > numUVWs )
						numUVWs = tex->GetMapChannel();
				}
			}

		}
		return numUVWs;
	}

	return 0;
}

hsGMaterial* hsMaterialConverter::NonAlphaHackPrint(plMaxNode* node, Texmap* baseTex, UInt32 blendFlags)
{
	// Bogus input, I hope they choke on the nil pointer I'm returning.
	if( !(baseTex && node) )
		return nil;

	char name[256];
	sprintf(name, "%s_%s_%d", node->GetName(), baseTex->GetName(), 0);

	// Search done materials for it

	hsGMaterial* mat = TRACKED_NEW hsGMaterial;
	hsgResMgr::ResMgr()->NewKey(name, mat, node->GetLocation());

	// If plasmaLayer is nil, the artist has some other wierd (unsupported) layer type in the slot.
	// Should warn them here.
	plPlasmaMAXLayer* plasmaLayer = plPlasmaMAXLayer::GetPlasmaMAXLayer(baseTex);
	if( !plasmaLayer )
		return nil;

	plLayerInterface *layerIFace = plLayerConverter::Instance().ConvertTexmap(baseTex, node, 0, false, false); 

	plLayer* baseLay = plLayer::ConvertNoRef(layerIFace->BottomOfStack());
	if( !baseLay )
		return nil;

	baseLay->SetTransform(hsMatrix44::IdentityMatrix());
	baseLay->SetUVWSrc(0);
	baseLay->SetBlendFlags(blendFlags);
	baseLay->SetZFlags(hsGMatState::kZNoZWrite | hsGMatState::kZIncLayer);
	baseLay->SetShadeFlags(0);
	baseLay->SetClampFlags(hsGMatState::kClampTexture);
	baseLay->SetMiscFlags(0);

	baseLay->SetAmbientColor(hsColorRGBA().Set(0.f, 0.f, 0.f, 1.f));
	baseLay->SetRuntimeColor(hsColorRGBA().Set(1.f, 1.f, 1.f, 1.f));
	baseLay->SetPreshadeColor(hsColorRGBA().Set(0.f, 0.f, 0.f, 1.f));

	IAddLayerToMaterial(mat, layerIFace);

	return mat;
}

hsGMaterial* hsMaterialConverter::AlphaHackPrint(plMaxNode* node, Texmap* baseTex, UInt32 blendFlags)
{
	// Bogus input, I hope they choke on the nil pointer I'm returning.
	if( !(baseTex && node) )
		return nil;

	char name[256];
	sprintf(name, "%s_%s_%d_AH", node->GetName(), baseTex->GetName(), 0);

	// Search done materials for it

	hsGMaterial* mat = TRACKED_NEW hsGMaterial;
	hsgResMgr::ResMgr()->NewKey(name, mat, node->GetLocation());

	// If plasmaLayer is nil, the artist has some other wierd (unsupported) layer type in the slot.
	// Should warn them here.
	plPlasmaMAXLayer* plasmaLayer = plPlasmaMAXLayer::GetPlasmaMAXLayer(baseTex);
	if( !plasmaLayer )
		return nil;

	plLayerInterface *layerIFace = plLayerConverter::Instance().ConvertTexmap(baseTex, node, 0, false, false); 

	plLayer* baseLay = plLayer::ConvertNoRef(layerIFace->BottomOfStack());
	if( !baseLay )
		return nil;

	baseLay->SetTransform(hsMatrix44::IdentityMatrix());
	baseLay->SetUVWSrc(0);
	baseLay->SetBlendFlags(blendFlags);
	baseLay->SetZFlags(hsGMatState::kZNoZWrite | hsGMatState::kZIncLayer);
	baseLay->SetShadeFlags(0);
	baseLay->SetClampFlags(hsGMatState::kClampTexture);
	baseLay->SetMiscFlags(0);

	baseLay->SetAmbientColor(hsColorRGBA().Set(0.f, 0.f, 0.f, 1.f));
	baseLay->SetRuntimeColor(hsColorRGBA().Set(1.f, 1.f, 1.f, 1.f));
	baseLay->SetPreshadeColor(hsColorRGBA().Set(1.f, 1.f, 1.f, 1.f));

	IAddLayerToMaterial(mat, layerIFace);

	plMipmap *texture = IGetUVTransTexture(node);
	IInsertSingleBlendLayer(texture, mat, node, 1, 1);

	return mat;
}

hsGMaterial* hsMaterialConverter::NonAlphaHackVersion(plMaxNode* node, Mtl* mtl, int subIndex)
{
	if( !mtl )
		return nil;

	if( IsMultiMat(mtl) )
	{
		return nil;
	}

	char name[256];
	sprintf(name, "%s_%s_%d", node->GetName(), mtl->GetName(), subIndex);

	return ICreateMaterial(mtl, node, name, subIndex, 1, false);
}

hsGMaterial* hsMaterialConverter::AlphaHackVersion(plMaxNode* node, Mtl* mtl, int subIndex)
{
	if( !mtl )
		return nil;

	if( IsMultiMat(mtl) )
	{
		return nil;
	}

	char name[256];
	sprintf(name, "%s_%s_%d_AH", node->GetName(), mtl->GetName(), subIndex);

	return ICreateMaterial(mtl, node, name, subIndex, 1, true);
}

//
// Big kahuna converter function
// (Though meshConverter should be calling CreateMaterialArray instead)
//
hsGMaterial *hsMaterialConverter::ICreateMaterial(Mtl *mtl, plMaxNode *node, const char *name, int subIndex, 
												  int numUVChannels, hsBool makeAlphaLayer)
{
	hsGuardBegin("hsMaterialConverter::ICreateMaterial");

	char *nodeName = node->GetName();
	fSubIndex = subIndex;
	fNodeName = nodeName;
	Object *obj = node->EvalWorldState(fConverterUtils.GetTime(fInterface)).obj;
	if (obj && (obj->SuperClassID() == SClass_ID(LIGHT_CLASS_ID)))
	{
		return ICheckForProjectedTexture(node);
	}
	else
	{
		hsBool forceCopy = hsControlConverter::Instance().OwnsMaterialCopy(node);
		if( !forceCopy )
			forceCopy = node->GetForceMaterialCopy();
		if( !forceCopy )
			forceCopy = IMustBeUniqueMaterial( mtl );

		hsBool runtimeLit = node->GetRunTimeLight();

		Mtl *bMtl = GetBaseMtl(node);
		hsBool isMultiMat = IsMultiMat(bMtl);// || IsPortalMat(bMtl);
		
		hsGMaterial *mat;
		if (mtl)
		{
			if (IsMatchingDoneMaterial(&fLastMaterial, mtl, isMultiMat, subIndex, forceCopy, runtimeLit,
									   node, numUVChannels, makeAlphaLayer))
			{
				mat = fLastMaterial.fHsMaterial;
				CopyMaterialLODToTextures(mat);
				//hsRefCnt_SafeRef(mat);
				return mat;
			}

			Int32 i;
			Int32 index(-1);
			for (i = 0; i < fDoneMaterials.Count(); i++)
			{
				if (IsMatchingDoneMaterial(&fDoneMaterials[i], mtl,  isMultiMat, subIndex, forceCopy, runtimeLit,
										   node, numUVChannels, makeAlphaLayer))
				{
					index = i;
					break;
				}				
			}

			if (index != -1)
			{
				mat = fDoneMaterials[index].fHsMaterial;
			
				fLastMaterial = fDoneMaterials[index];
	
				CopyMaterialLODToTextures(mat);
				//hsRefCnt_SafeRef(mat);
				return mat;
			}
		}
		// Guess we haven't converted this one before.	Bummer.
		
		// Get material
		/* // CreateMaterial is not called on MultiMats (if CreateMaterialArray is doing its job)
		if (IsMultiMat(mtl))
		{
			mat = IProcessMaterial(mtl->GetSubMtl(subIndex), node, numUVChannels);
		}
		*/
		if (IsCompositeMat(mtl))
		{
			mat = IProcessMaterial(mtl, node, name, numUVChannels, subIndex);
			IInsertCompBlendingLayers(mtl, node, mat, subIndex, numUVChannels, makeAlphaLayer);
		}
		else if (IsMultipassMat(mtl))
		{			
			mat = IProcessMaterial(mtl, node, name, numUVChannels);
			IInsertMultipassBlendingLayers(mtl, node, mat, numUVChannels, makeAlphaLayer);
		}
		else if (IsHsMaxMat(mtl) || IsDecalMat(mtl))
		{			
			mat = IProcessMaterial(mtl, node, name, numUVChannels);
			IInsertAlphaBlendingLayers(mtl, node, mat, numUVChannels, makeAlphaLayer);
		}
		else if (IsClothingMat(mtl))
		{
			mat = nil; // clothing materials do not generate an hsGMaterial object
		}
		else { // Particle materials hit this. No need for blending layers, ever.
			mat = IProcessMaterial(mtl, node, name, numUVChannels);
		}
		if( mat && HasBumpLayer(node, mtl) )
			IInsertBumpLayers(node, mat);

		if (mat)
			mat = IInsertDoneMaterial(mtl, mat, node, isMultiMat, forceCopy, runtimeLit, subIndex, numUVChannels, makeAlphaLayer);

		return mat;
	}
	return nil;
	hsGuardEnd; 
}

#include "../MaxPlasmaMtls/Materials/plMultipassMtl.h"

//
// Handle materials for normal non-light, non-particle nodes.
//
hsGMaterial *hsMaterialConverter::IProcessMaterial(Mtl *mtl, plMaxNode *node, const char *name, 
												   int UVChan, int subMtlFlags /* = 0 */)
{
	hsGuardBegin("hsMaterialConverter::IProcessMaterial");

	plLocation nodeLoc = node->GetLocation();

	char *dbgNodeName = node->GetName();
	hsGMaterial *hMat = nil;
	fChangedTimes = false;

	if (IsMultiMat(mtl))
	{
		if (fErrorMsg->Set(!(fWarned & kWarnedSubMulti), dbgNodeName, "Multi-material in ProcessMaterial (Multi child of multi?) on mat %s.", 
			mtl->GetName()).CheckAskOrCancel() )
			fWarned |= kWarnedSubMulti;
		hMat = IProcessMaterial(mtl->GetSubMtl(0), node, name, UVChan);
	}
	else if (IsHsMaxMat(mtl) || IsDecalMat(mtl) || IsBumpMtl( mtl ) ) 
	{
		hMat = TRACKED_NEW hsGMaterial;
		hsgResMgr::ResMgr()->NewKey(name, hMat,nodeLoc);
		IProcessPlasmaMaterial(mtl, node, hMat, hMat->GetKey()->GetName());
	}
	else if (mtl && mtl->ClassID() == MULTIMTL_CLASS_ID)
	{
		hMat = IProcessMultipassMtl(mtl, node, name, UVChan);
	}
	else if (IsCompositeMat(mtl))
	{
		hMat = IProcessCompositeMtl(mtl, node, name, UVChan, subMtlFlags);
	}
	else if (IsParticleMat(mtl))
	{
		hMat = IProcessParticleMtl(mtl, node, name);
	}
	else
	{
		hMat = IAddDefaultMaterial(node);
	}

	if (hMat)
	{
		if (node->GetAvatarSO() != nil)
		{
			AttachLinkMtlAnims(node, hMat);
		}
		if (hMat->GetNumLayers() == 0)
		{
			if (fErrorMsg->Set((fWarned & kWarnedNoLayers) == 0, node->GetName(), "Material has no layers. (%s)", mtl->GetName()).CheckAndAsk())
				fWarned |= kWarnedNoLayers;
			
			plLayer* hLay = TRACKED_NEW plLayer;
			hLay->InitToDefault();
			hsgResMgr::ResMgr()->NewKey(TSTR(name) + TSTR("_DefLay"), hLay, nodeLoc);
			IAddLayerToMaterial(hMat, hLay);
		}

		if( node->UserPropExists("WetMe") && !strstr(hMat->GetKey()->GetName(), "Wet(*)") )
			IAppendWetLayer(node, hMat);
//		hsgResMgr::ResMgr()->NewKey(name, hMat,nodeLoc);
	}
	else
		return nil;

	if (hMat->IsDynamic())
	{
		fChangedTimes = true;
	}

	CopyMaterialLODToTextures(hMat);

	return hMat;
	hsGuardEnd; 
}

hsBool hsMaterialConverter::IsMatchingDoneMaterial(DoneMaterialData *dmd, 
												   Mtl *mtl, hsBool isMultiMat, UInt32 subMtlFlags, hsBool forceCopy, hsBool runtimeLit,
												   plMaxNode *node, int numUVChannels, hsBool makeAlphaLayer)
{
	if (!((dmd->fMaxMaterial == mtl) && 
		  (dmd->fSubMultiMat == isMultiMat) &&
		  (dmd->fSubMtlFlags == subMtlFlags) &&
		  (dmd->fRuntimeLit == runtimeLit) && 
		  (dmd->fNumUVChannels == numUVChannels) &&
		  (dmd->fMakeAlphaLayer == makeAlphaLayer)))
	{
		if( dmd->fMaxMaterial == mtl )
		{
			if( dmd->fRuntimeLit != runtimeLit )
				failedRT = true;
			if( dmd->fNumUVChannels != numUVChannels )
				failedNumUV = true;
			if( dmd->fMakeAlphaLayer != makeAlphaLayer )
				failedAlphaLayer = true;
		}

		return false;
	}
	if( dmd->fNode->HasFade() != node->HasFade() )
	{
		bool realFade = false;
		if( dmd->fNode->HasFade() )
		{
			Box3 fade = dmd->fNode->GetFade();
			if( (fade.Min()[0] != fade.Min()[1]) || (fade.Max()[0] != fade.Max()[1]) )
				realFade = true;
		}
		else if( node->HasFade() )
		{
			Box3 fade = node->GetFade();
			if( (fade.Min()[0] != fade.Min()[1]) || (fade.Max()[0] != fade.Max()[1]) )
				realFade = true;
		}

		if( realFade )
		{
			failedFade = true;
			return false;
		}
	}
	if( dmd->fNode->HasFade() && node->HasFade() )
	{
		Box3 dmdFade = dmd->fNode->GetFade();
		Box3 nodeFade = node->GetFade();
		if( dmdFade.Min() != nodeFade.Min() )
		{
			if( (dmdFade.Min()[0] != dmdFade.Min()[1]) || (nodeFade.Min()[0] != nodeFade.Min()[1]) )
			{
				failedFade = true;
				return false;
			}
		}
		if( dmdFade.Max() != nodeFade.Max() )
		{
			if( (dmdFade.Max()[0] != dmdFade.Max()[1]) || (nodeFade.Max()[0] != nodeFade.Max()[1]) )
			{
				failedFade = true;
				return false;
			}
		}
	}
				
	return !(forceCopy || dmd->fOwnedCopy) || (dmd->fNode == node);
}
		

hsGMaterial* hsMaterialConverter::IInsertDoneMaterial(Mtl *mtl, hsGMaterial *hMat, plMaxNode *node, hsBool isMultiMat, 
											  hsBool forceCopy, hsBool runtimeLit, UInt32 subMtlFlags, int numUVChannels, 
											  hsBool makeAlphaLayer)
{
	if( failedRT )
		dupCuzRT++;
	if( failedNumUV )
		dupCuzNumUV++;
	if( failedAlphaLayer )
		dupCuzAlphaLayer++;
	if( failedFade )
		dupCuzFade++;
	failedRT = failedNumUV = failedAlphaLayer = failedFade = false;

	DoneMaterialData done;
	done.fHsMaterial = hMat;
//	hsRefCnt_SafeAssign(done.fHsMaterial, hMat);
	done.fHsMaterial = hMat;
	done.fMaxMaterial = mtl;
	done.fNode = node;
	done.fSubMultiMat = isMultiMat;
	done.fOwnedCopy = forceCopy;
	done.fRuntimeLit = runtimeLit;
	done.fSubMtlFlags = subMtlFlags;
	done.fNumUVChannels = numUVChannels;
	done.fMakeAlphaLayer = makeAlphaLayer;

	DoneMaterialData* equivalent = IFindDoneMaterial(done);
	
	if( equivalent )
	{
		plKey matKey = hMat->GetKey();
		matKey->RefObject();
		matKey->UnRefObject();
		((plKeyImp *)matKey)->SetObjectPtr(nil);
		matKey = nil;

		hMat = equivalent->fHsMaterial;
	}
	else
	{
		hMat->GetKey()->RefObject(); // Matching unref in hsMaterialConverter::DeInit();

		fDoneMaterials.Append(done);
	}

	return hMat;
}

hsGMaterial *hsMaterialConverter::IAddDefaultMaterial(plMaxNode *node)
{
	if (!node)
		return nil;

	plLocation loc = node->GetLocation();

	hsGMaterial *hMat = TRACKED_NEW hsGMaterial;
	hsgResMgr::ResMgr()->NewKey(TSTR(node->GetName()) + TSTR("_DefMat"), hMat, loc);
	
	plLayer *layer = TRACKED_NEW plLayer;
	layer->InitToDefault();
	hsgResMgr::ResMgr()->NewKey(TSTR(hMat->GetKeyName()) + TSTR("_DefLay"), layer, loc);

	DWORD color = node->GetWireColor();
	hsScalar r = hsScalar(GetRValue(color)) / 255.f;
	hsScalar g = hsScalar(GetGValue(color)) / 255.f;
	hsScalar b = hsScalar(GetBValue(color)) / 255.f;
	layer->SetRuntimeColor(hsColorRGBA().Set(r, g, b, 1.f));
	layer->SetPreshadeColor(hsColorRGBA().Set(r, g, b, 1.f));
	layer->SetOpacity(1.f);
	IAddLayerToMaterial(hMat, layer);

	return hMat;
}

plMipmap *hsMaterialConverter::IGetUVTransTexture(plMaxNode *node, hsBool useU /* = true */)
{
	char* texName = (useU ? "ALPHA_BLEND_FILTER_U2ALPHA_TRANS_64x4" : "ALPHA_BLEND_FILTER_V2ALPHA_TRANS_4x64" );

	int w = (useU ? 64 : 4);
	int h = (useU ? 4 : 64);

	// NOTE: CreateBlankMipmap might return an old mipmap if it was already created, so in those
	// cases we wouldn't really need to re-write the texture. However, there's no harm in doing so,
	// and since we're close to Alpha, I don't want to shake up the code any more than absolutely 
	// necessary. -mcn
	plMipmap *texture = plBitmapCreator::Instance().CreateBlankMipmap( w, h, plMipmap::kARGB32Config, 1, texName, node->GetLocation() );

	// set the color data
	UInt32* pix = (UInt32*)texture->GetImage();
	int x, y;
	for (y = 0; y < h; y++)
	{
		for (x = 0; x < w; x++)
		{
			// Filter either the U or V coordinate
			float shade = (useU ? (float)x / (w - 1) : (float)y / (h - 1));

			// These colors MUST be all white even though we're usuing the kBlendNoColor flag... 
			// Rad huh? Hooray for nVidia!!!
			*pix++ = MakeUInt32Color(1.0, 1.0, 1.0, shade);
		}
	}

	return texture;
}

//// IInsertSingleBlendLayer //////////////////////////////////////////////////
//	10.24.01 mcn - Added kMiscRestartPassHere to the layer below the inserted
//				   one, to guarantee that the hacked alpha layers draw in their
//				   own passes on the GF3. Keeping this here until we can properly
//				   handle the hack layers as piggybacks in the pipeline.

void hsMaterialConverter::IInsertSingleBlendLayer(plMipmap *texture, hsGMaterial *mat, 
												  plMaxNode *node, int layerIdx, int UVChan)
{
	// Need to tweak a few flags on its corresponding layer
	plLayer* underLay = plLayer::ConvertNoRef(mat->GetLayer(layerIdx - 1)->BottomOfStack());
	if( !underLay )
		return;

	// This error means a GeForce 1 or 2 won't do the vertex alpha correctly, and every other card
	// won't care. We've been ignoring this warning for years anyway, might as well just take it
	// out and stop interrupting the export.
//	fErrorMsg->Set((underLay->GetMiscFlags() & hsGMatState::kMiscBindNext) != 0, node->GetName(),
//		"Layer %s has its BindNext flag set, which can't be done with a vertex alpha blend. The "
//		"resulting material may not blend correctly.", underLay->GetKeyName()).Show();
	underLay->SetMiscFlags(underLay->GetMiscFlags() | hsGMatState::kMiscBindNext
							| hsGMatState::kMiscRestartPassHere );
	underLay->SetBlendFlags(underLay->GetBlendFlags() | hsGMatState::kBlendAlpha);
	mat->SetCompositeFlags(mat->GetCompositeFlags() | hsGMaterial::kCompNeedsBlendChannel);

	
	plLayer* layer = TRACKED_NEW plLayer;
	layer->InitToDefault();
	hsgResMgr::ResMgr()->NewKey(TSTR(underLay->GetKeyName()) + TSTR("_AlphaBlend"), layer, node->GetLocation());
	hsgResMgr::ResMgr()->AddViaNotify(texture->GetKey(), TRACKED_NEW plLayRefMsg(layer->GetKey(), plRefMsg::kOnCreate, 0, plLayRefMsg::kTexture), plRefFlags::kActiveRef);
	layer->SetAmbientColor(hsColorRGBA().Set(1.f, 1.f, 1.f, 1.f));
//	layer->SetZFlags(hsGMatState::kZNoZWrite | hsGMatState::kZIncLayer);
	// The inclayer prop probably wouldn't hurt here, because this layer should only get drawn as
	// an upper layer, but I'm nuking it out for consistency. mf
	layer->SetZFlags(hsGMatState::kZNoZWrite);
	UInt32 blendFlags = hsGMatState::kBlendNoTexColor | hsGMatState::kBlendAlphaMult | hsGMatState::kBlendAlpha;
	layer->SetBlendFlags(blendFlags);
	layer->SetClampFlags(hsGMatState::kClampTexture);
	layer->SetUVWSrc(UVChan);
	layer->SetMiscFlags(0);

	// Insert it in the right spot.
	hsgResMgr::ResMgr()->AddViaNotify(layer->GetKey(), TRACKED_NEW plMatRefMsg(mat->GetKey(), plRefMsg::kOnCreate, 
 																       layerIdx, plMatRefMsg::kLayer | plMatRefMsg::kInsert), plRefFlags::kActiveRef);
}

void hsMaterialConverter::IInsertAlphaBlendingLayers(Mtl *mtl, plMaxNode *node, hsGMaterial *mat, int UVChan,
													hsBool makeAlphaLayer)
{
	if ((mat->GetLayer( 0 )->GetState().fBlendFlags & hsGMatState::kBlendAlpha) == 0 ||	UVChan < 0 || !makeAlphaLayer)
		return; // Not blending... false alarm... my bad... enjoy the buffet!

	if (!(UVChan < plGeometrySpan::kMaxNumUVChannels))
	{
		if (fErrorMsg->Set(!(fWarned & kWarnedTooManyUVs), node->GetName(), 
						"Material is already using all available UV channels and thus doesn't have one for alpha "
						"blending. Some layers will have incorrect channels assigned. (%s)", 
						mtl->GetName()).CheckAskOrCancel() )
		fWarned |= kWarnedTooManyUVs; 
		UVChan = plGeometrySpan::kMaxNumUVChannels - 1;
	}
	int i;	

	hsGMaterial *objMat = mat;
	plMipmap *texture = IGetUVTransTexture(node);
	int origLayers = objMat->GetNumLayers();
	for (i = 0; i < origLayers; i++)
	{
		IInsertSingleBlendLayer(texture, objMat, node, 2 * i + 1, UVChan);
	}
}

void hsMaterialConverter::IInsertMultipassBlendingLayers(Mtl *mtl, plMaxNode *node, hsGMaterial *mat, int UVChan,
														hsBool makeAlphaLayer)
{
	if (UVChan < 0 || !makeAlphaLayer)
		return; // Not blending, no layers to insert.

	if (!(UVChan < plGeometrySpan::kMaxNumUVChannels))
	{
		if (fErrorMsg->Set(!(fWarned & kWarnedTooManyUVs), node->GetName(), 
						"Material is already using all available UV channels and thus doesn't have one for alpha "
						"blending. Some layers will have incorrect channels assigned. (%s)", 
						mtl->GetName()).CheckAskOrCancel() )
		fWarned |= kWarnedTooManyUVs; 
		UVChan = plGeometrySpan::kMaxNumUVChannels - 1;
	}

	int i;
	hsGMaterial *objMat = mat;
	IParamBlock2 *pb = mtl->GetParamBlockByID(plMultipassMtl::kBlkPasses);
	plMipmap *texture = IGetUVTransTexture(node);
	int currLayerNum = 0;
	for (i = 0; i < mtl->NumSubMtls(); i++)
	{
		if (!pb->GetInt(kMultOn, 0, i)) // Is the box for this submtl checked?
			continue; // No, skip it!

		if ((objMat->GetLayer(currLayerNum)->GetBlendFlags() & hsGMatState::kBlendAlpha) == 0) 
		{   // not alpha blending... skip!
			currLayerNum += pb->GetInt(kMultLayerCounts, 0, i);
			continue;
		}
		int j;

		for (j = 0; j < pb->GetInt(kMultLayerCounts, 0, i); j++)
		{
			IInsertSingleBlendLayer(texture, objMat, node, currLayerNum + 1, UVChan);
			currLayerNum += 2;
		}
	}
	return;
}

void hsMaterialConverter::IInsertCompBlendingLayers(Mtl *mtl, plMaxNode *node, hsGMaterial *mat, int subMtlFlags,
												   int UVChan, hsBool makeAlphaLayer)
{
	if (mat == nil)
		return;

	if (!IsCompositeMat(mtl))
	{
		hsAssert(false, "IInsertCompBlendingLayers() called on a non-composite material.");
		return;
	}

	int i;
	hsGMaterial *objMat = mat;
	plMipmap *textureU = IGetUVTransTexture(node, true);
	plMipmap *textureV = IGetUVTransTexture(node, false);
	IParamBlock2 *pb = mtl->GetParamBlockByID(plCompositeMtl::kBlkPasses);
	int currLayerNum = 0;
	int bitmask;
	hsBool firstUsedLayer = true;
	hsBool canWriteAlpha = ((plCompositeMtl *)mtl)->CanWriteAlpha() >= 0;
	plMipmap *currTexture;
	for (i = 0, bitmask = 0x1; i < mtl->NumSubMtls(); i++, bitmask <<= 1)
	{
		if ((bitmask & subMtlFlags) == 0) // skip it!
			continue;

		if (firstUsedLayer)
		{
			firstUsedLayer = false;
			if (i != 0) // it's not the base layer, so it must be covering the base layer up. Thus, it's
			{			// fully opaque, and we shouldn't blend.
				plLayer* underLay = plLayer::ConvertNoRef(objMat->GetLayer(currLayerNum)->BottomOfStack());
				if( underLay )
					underLay->SetBlendFlags(underLay->GetBlendFlags() & ~hsGMatState::kBlendMask);
				currLayerNum += pb->GetInt(kCompLayerCounts, 0, i);
				continue;
			}
		}

		if (i == 0 && ((mat->GetLayer( 0 )->GetState().fBlendFlags & hsGMatState::kBlendAlpha) == 0))
		{
			currLayerNum += pb->GetInt(kCompLayerCounts, 0, i);
			continue;
		}
		
		int method = (i <= 0 ? kCompBlendVertexAlpha : pb->GetInt(kCompBlend, 0, i - 1));
		currTexture = (i == 1 ? textureU : textureV);
		int j;
		for (j = 0; j < pb->GetInt(kCompLayerCounts, 0, i); j++)
		{		
			if (!makeAlphaLayer && canWriteAlpha)
			{
				if (((plCompositeMtl *)mtl)->IsInverseBlend(method))
				{
					plLayer* underLay = plLayer::ConvertNoRef(objMat->GetLayer(currLayerNum)->BottomOfStack());
					underLay->SetBlendFlags(underLay->GetBlendFlags() | hsGMatState::kBlendInvertVtxAlpha);
				}
				currLayerNum++;
				continue;
			}
			
			IInsertSingleBlendLayer(currTexture, objMat, node, currLayerNum + 1, UVChan);
			currLayerNum += 2;
		}
	}
	return;
}

hsGMaterial *hsMaterialConverter::IProcessCompositeMtl(Mtl *mtl, plMaxNode *node, const char *name, int UVChan, int subMtlFlags)
{
	if (!mtl || mtl->ClassID() != COMP_MTL_CLASS_ID)
		return nil;
	UInt32 *layerCounts = TRACKED_NEW UInt32[mtl->NumSubMtls()];
	IParamBlock2 *pb = mtl->GetParamBlockByID(plCompositeMtl::kBlkPasses);
	char suff[10];
	sprintf(suff, "_%d", subMtlFlags);
	hsGMaterial *mat = TRACKED_NEW hsGMaterial;
	hsgResMgr::ResMgr()->NewKey(TSTR(name) + TSTR(suff), mat, node->GetLocation());
	
	int multiIndex = IFindSubIndex(node, mtl);
	hsBool needAlphaHack = node->AlphaHackLayersNeeded(multiIndex) > 0;
	hsBool canWriteAlpha = ((plCompositeMtl *)mtl)->CanWriteAlpha() >= 0;	

	int bitMask = 1;
	int i;
	for (i = 0; i < mtl->NumSubMtls(); i++)
	{
		Mtl *subMtl = nil;
		hsBool usingSubMtl = (i == 0 || pb->GetInt(kCompOn, 0, i - 1));
		if ((bitMask & subMtlFlags) != 0 && usingSubMtl)
		{
			char pref[256];
			sprintf(pref, "%s_%d", mat->GetKey()->GetName(), i);

			subMtl = mtl->GetSubMtl(i);
			if (subMtl != nil && subMtl->ClassID() == PASS_MTL_CLASS_ID)
				IProcessPlasmaMaterial(subMtl, node, mat, pref);
		}
		bitMask <<= 1;
		layerCounts[i] = mat->GetNumLayers();
		if (i > 0 && mat->GetNumLayers() > 0)
		{
			hsBool materialIsBad = false;
			int j;
			if (usingSubMtl)
			{
				for (j = mat->GetNumLayers() - 1; j >= (int)layerCounts[i - 1]; j--) 
				{
					UInt32 blendFlags = mat->GetLayer(j)->GetBlendFlags();
					if ((blendFlags & hsGMatState::kBlendMask) != hsGMatState::kBlendAlpha)
					{
						hsBool ignore = fErrorMsg->Set(!(fWarned & kWarnedCompMtlBadBlend), node->GetName(),
							"For composite materials, all multi-layered submaterials (except the base)"
							" must choose 'alpha' for 'output blending'. %s, which is a sub-material"
							" of %s, doesn't and will not be included in the composite material.",
							subMtl->GetName(), mtl->GetName()).CheckAskOrCancel();
						
						materialIsBad = true;
						if (ignore)
							fWarned |= kWarnedCompMtlBadBlend;
					}

					if ((needAlphaHack || !canWriteAlpha) &&
						((blendFlags & hsGMatState::kBlendAlphaMult) || (blendFlags & hsGMatState::kBlendAlphaAdd)))
					{
						hsBool ignore = fErrorMsg->Set(!(fWarned & kWarnedCompMtlBadBlend), node->GetName(),
							"Composite material %s has a submaterial, %s, that requires too many textures in a single pass "
							"(for blending effects). To cut this down, try some of the following:\n"
							"1. Make sure all multi-layered submaterials (except the base)"
							" choose 'alpha' for 'layer blending', and 'base alpha only' for 'layer alpha blending'\n"
							"2. Force the object to be runtime lit.\n"
							"3. Remove other components on this node that may add extra blending layers.\n"
							"For now, the submaterial will not be included in the composite.",
							mtl->GetName(), subMtl->GetName()).CheckAskOrCancel();

						materialIsBad = true;
						if (ignore)
							fWarned |= kWarnedCompMtlBadBlend;
					}
				}
			}
			if (materialIsBad) // Nuke all the layers of this sub material, so the artists don't just ignore the warnings
			{
				int min = (int)layerCounts[i - 1];
				for (j = mat->GetNumLayers() - 1; j >= min; j--)
				{
					mat->GetKey()->Release(mat->GetLayer(j)->GetKey());
				}
				layerCounts[i] = mat->GetNumLayers();
			}
		}
	}

	for (i = mtl->NumSubMtls() - 1; i > 0; i--)
		layerCounts[i] -= layerCounts[i - 1];

	for (i = 0; i < mtl->NumSubMtls(); i++)
		pb->SetValue(kCompLayerCounts, 0, (int)layerCounts[i], i);

	delete [] layerCounts;
	if (mat->GetNumLayers() == 0)
	{
		// No one ever got a ref, so we ref and unref to make it go away.
		mat->GetKey()->RefObject();
		mat->GetKey()->UnRefObject();
		return IAddDefaultMaterial(node); 
	}

	return mat;
}

hsGMaterial *hsMaterialConverter::IProcessMultipassMtl(Mtl *mtl, plMaxNode *node, const char *name, int UVChan)
{
	if (!mtl || mtl->ClassID() != MULTIMTL_CLASS_ID)
		return nil;

	hsGMaterial *mat = TRACKED_NEW hsGMaterial;
	UInt32 *layerCounts = TRACKED_NEW UInt32[mtl->NumSubMtls()];
	hsgResMgr::ResMgr()->NewKey(name, mat, node->GetLocation());

	IParamBlock2 *pb = mtl->GetParamBlockByID(plMultipassMtl::kBlkPasses);
	for (int i = 0; i < mtl->NumSubMtls(); i++)
	{
		Mtl *subMtl = mtl->GetSubMtl(i);
		int check = pb->GetInt(kMultOn, 0, i);
		if ( ( subMtl->ClassID() == PASS_MTL_CLASS_ID ||
			   subMtl->ClassID() == BUMP_MTL_CLASS_ID ) && check != 0)
		{
			IProcessPlasmaMaterial(subMtl, node, mat, mat->GetKey()->GetName());
		}
		layerCounts[i] = mat->GetNumLayers();
	}

	for (i = mtl->NumSubMtls() - 1; i > 0; i--)
		layerCounts[i] -= layerCounts[i - 1];

	for (i = 0; i < mtl->NumSubMtls(); i++)
		pb->SetValue(kMultLayerCounts, 0, (int)layerCounts[i], i);

	delete [] layerCounts;
	return mat;
}

hsGMaterial *hsMaterialConverter::IProcessParticleMtl(Mtl *mtl, plMaxNode *node, const char *name)
{
	hsGuardBegin("hsMaterialConverter::IProcessParticleMaterial");

	plLocation nodeLoc = node->GetLocation(); 
	char* dbgNodeName = node->GetName();

	hsGMaterial *mat = TRACKED_NEW hsGMaterial;
	hsgResMgr::ResMgr()->NewKey(name, mat,nodeLoc);

	
	if(!mtl)
	{
		fErrorMsg->Set(!mtl, "Material Loading Error", "No material in ProcessParticleMaterial!").Show();
		fErrorMsg->Set();
		return nil;
	}

	plParticleMtl *particleMtl = (plParticleMtl *)mtl;
	IParamBlock2 *basicPB = mtl->GetParamBlockByID(plParticleMtl::kBlkBasic);
	//
	// Color / Opacity
	//
	Color amb  = basicPB->GetColor(plParticleMtl::kColorAmb);;
	Color dif  = basicPB->GetColor(plParticleMtl::kColor);
	float opac = float(basicPB->GetInt(plParticleMtl::kOpacity)) / 100.0f;

	fErrorMsg->Set(opac  < 0.0 || opac  > 1.0, dbgNodeName, "Bad opacity on mat %s", name).CheckAndAsk();
	fErrorMsg->Set(dif.r < 0.0 || dif.r > 1.0, dbgNodeName, "Bad color (r) on mat %s", name).CheckAndAsk();
	fErrorMsg->Set(dif.g < 0.0 || dif.g > 1.0, dbgNodeName, "Bad color (g) on mat %s", name).CheckAndAsk();
	fErrorMsg->Set(dif.b < 0.0 || dif.b > 1.0, dbgNodeName, "Bad color (b) on mat %s", name).CheckAndAsk();

	Color col = dif;

	//
	// Blend flags
	//
	UInt32 blendFlags = 0;

	//
	// Shade flags
	//
	UInt32 shadeFlags = 0;

	if (basicPB->GetInt(plParticleMtl::kNormal) == plParticleMtl::kEmissive)
		shadeFlags |= hsGMatState::kShadeEmissive;
	if( node->GetIsGUI() )
	{
		// We don't want materials on any GUI objects to *ever* be fogged 
		// (just in case we have some weird GUI particle system...)...4.25.2002 mcn
		shadeFlags |= hsGMatState::kShadeReallyNoFog;
	}

	//
	// Misc flags...
	//
	UInt32 miscFlags = 0;

	//
	// Z flags
	//
	UInt32 zFlags = 0;
	zFlags |= hsGMatState::kZNoZWrite;

	hsBool preserveUVOffset = false;//PreserveUVOffset(mtl);

	//
	// Process base layer
	//
	plLayer* baseLay = nil;
	Texmap *baseTex = basicPB->GetTexmap(plParticleMtl::kTexmap);
	if (baseTex && baseTex->ClassID() == LAYER_TEX_CLASS_ID)
	{
		plLayerInterface *layerIFace = plLayerConverter::Instance().ConvertTexmap( baseTex, node, 0, preserveUVOffset, false );

		baseLay = plLayer::ConvertNoRef(layerIFace->BottomOfStack());
		
		fErrorMsg->Set(!baseLay, node->GetName(), "Found no layer at end of IProcessPlasmaLayer()").Check();
			
		baseLay->SetBlendFlags(baseLay->GetBlendFlags() | blendFlags);
		baseLay->SetShadeFlags(baseLay->GetShadeFlags() | shadeFlags);
		baseLay->SetZFlags(baseLay->GetZFlags() | zFlags);
		baseLay->SetMiscFlags(baseLay->GetMiscFlags() | miscFlags);	// 3.28.2001 mcn - Who didn't put this in?

		// No specular on particles. Spoot.
		baseLay->SetSpecularColor( hsColorRGBA().Set(0,0,0,1.f) );
		baseLay->SetSpecularPower( 0 );

		baseLay->SetAmbientColor(hsColorRGBA().Set(amb.r, amb.g, amb.b, hsScalar1));
		baseLay->SetRuntimeColor(hsColorRGBA().Set(col.r, col.g, col.b, hsScalar1));
		baseLay->SetPreshadeColor(hsColorRGBA().Set(0.f,0.f,0.f,1.f));
		baseLay->SetOpacity( opac );		// Don't scale the material color by this if we're add blending; do that later

		UInt32 blendType = 0;
		switch (basicPB->GetInt(plParticleMtl::kBlend))
		{
		case plParticleMtl::kBlendAlpha:		blendType |= hsGMatState::kBlendAlpha;	break;
		case plParticleMtl::kBlendAdd:			blendType |= hsGMatState::kBlendAdd;	break;
		}
		baseLay->SetBlendFlags(baseLay->GetBlendFlags() | blendType);

		plLayerInterface *layerAnim = layerIFace;

		IAddLayerToMaterial(mat, layerAnim);
	}


	return mat;
	hsGuardEnd; 
}

plLayerAnimation *IConvertNoteTrackAnims(plLayerAnimation *animLayer, SegmentMap *segMap, plMaxNode *node, 
										 const char *name)
{
	plLayerAnimation *prev = animLayer;
	
	for (SegmentMap::iterator it = segMap->begin(); it != segMap->end(); it++)
	{
		SegmentSpec *spec = it->second;
		if (spec->fType == SegmentSpec::kAnim)
		{
			plLayerAnimation *noteAnim = TRACKED_NEW plLayerAnimation;
			TSTR animName = TSTR(name) + TSTR("_anim_") + TSTR(spec->fName);
			hsgResMgr::ResMgr()->NewKey(animName, noteAnim, node->GetLocation());

			if (animLayer->GetPreshadeColorCtl())
				noteAnim->SetPreshadeColorCtl(animLayer->GetPreshadeColorCtl());
			if (animLayer->GetRuntimeColorCtl())
				noteAnim->SetRuntimeColorCtl(animLayer->GetRuntimeColorCtl());
			if (animLayer->GetAmbientColorCtl())
				noteAnim->SetAmbientColorCtl(animLayer->GetAmbientColorCtl());
			if (animLayer->GetSpecularColorCtl())
				noteAnim->SetSpecularColorCtl(animLayer->GetSpecularColorCtl());
			if (animLayer->GetOpacityCtl())
				noteAnim->SetOpacityCtl(animLayer->GetOpacityCtl());

			noteAnim->GetTimeConvert().SetBegin(spec->fStart);
			noteAnim->GetTimeConvert().SetEnd(spec->fEnd);

			noteAnim->AttachViaNotify(prev);
			prev = noteAnim;
		}
	}

	return prev;
}

void ISetDefaultAnim(plPassMtlBase* mtl, plAnimTimeConvert& tc, SegmentMap* segMap)
{
	const char *animName = mtl->GetAnimName();
	if( segMap && animName && (segMap->find(animName) != segMap->end()) )
	{
		SegmentSpec *spec = (*segMap)[animName];
		tc.SetBegin(spec->fStart);
		tc.SetEnd(spec->fEnd);
	}

	if (mtl->GetAutoStart())
		tc.Start(0);
	else
		tc.Stop(true);

	if (mtl->GetLoop())
	{
		// Default to the entire anim
		hsScalar loopStart = tc.GetBegin();
		hsScalar loopEnd = tc.GetEnd();

		// If there's a loop, use it
		const char *loopName = mtl->GetAnimLoopName();
		if (loopName && *loopName && segMap)
			GetSegMapAnimTime(loopName, segMap, SegmentSpec::kLoop, loopStart, loopEnd);

		tc.SetLoopPoints(loopStart, loopEnd);
		tc.Loop(true);
	}
	else
		tc.Loop(false);
}

void StuffStopPoints(SegmentMap *segMap, hsTArray<hsScalar> &out)
{
	if (segMap == nil)
		return;

	for (SegmentMap::iterator it = segMap->begin(); it != segMap->end(); it++)
	{
		SegmentSpec *spec = it->second;
		if (spec->fType == SegmentSpec::kStopPoint)
		{
			out.Append(spec->fStart);
		}
	}
}

void IProcessMtlAnimEase(plPassMtlBase *mtl, plAnimTimeConvert &tc)
{
	if (mtl->GetEaseInType() != plAnimEaseTypes::kNoEase)
	{
		tc.SetEase(true, mtl->GetEaseInType(), mtl->GetEaseInMinLength(),
				   mtl->GetEaseInMaxLength(), mtl->GetEaseInNormLength());
	}
	if (mtl->GetEaseOutType() != plAnimEaseTypes::kNoEase)
	{
		tc.SetEase(false, mtl->GetEaseOutType(), mtl->GetEaseOutMinLength(),
				   mtl->GetEaseOutMaxLength(), mtl->GetEaseOutNormLength());
	}
}
/*
hsBool hsMaterialConverter::CheckValidityOfSDLVarAnim(plPassMtlBase *mtl, char *varName, plMaxNode *node)
{
	plStateDescriptor *sd = nil;
	char *ageName;
	ageName = node->GetAgeName();
	if (ageName)
		sd = plSDLMgr::GetInstance()->FindDescriptor(ageName, plSDL::kLatestVersion);
	if (sd)
	{
		plVarDescriptor *var = sd->FindVar(varName);
		if (var == nil)
		{
			char buff[512];
			sprintf(buff, "Cannot find variable named \"%s\" used to animate the material "
					"\"%s\". This material won't be animated.", varName, mtl->GetName());
			if (fErrorMsg->Set(!(fWarned & kWarnedBadAnimSDLVarName), node->GetName(), buff).CheckAskOrCancel() )
				fWarned |= kWarnedBadAnimSDLVarName;
			return false;
		}
		if (!(var->GetType() == plVarDescriptor::kFloat ||
			  var->GetType() == plVarDescriptor::kDouble ||
			  var->GetType() == plVarDescriptor::kTime ||
			  var->GetType() == plVarDescriptor::kAgeTimeOfDay))
		{
			char buff[512];
			sprintf(buff, "Material \"%s\" animates on the age variable \"%s\", which is "
					"of type \"%s\". Only the types float/double/time/ageTimeOfDay may be used. "
					"Ignoring the animation of this material.", mtl->GetName(), varName, var->GetTypeString());
			if (fErrorMsg->Set(!(fWarned & kWarnedBadAnimSDLVarName), node->GetName(), buff).CheckAskOrCancel())
				fWarned |= kWarnedBadAnimSDLVarName;
			return false;
		}
		return true;
	}
	return false;
}
*/

static plAnimStealthNode* IGetEntireAnimation(plPassMtlBase* mtl)
{
	const int count = mtl->GetNumStealths();
	int i;
	for( i = 0; i < count; i++ )
	{
		plAnimStealthNode* stealth = mtl->GetStealth(i);

		const char *segName = stealth->GetSegmentName();
		if( !segName || !*segName || !stricmp(segName, ENTIRE_ANIMATION_NAME) )
			return stealth;

	}
	return nil;
}

static plLayerInterface* IProcessLayerMovie(plPassMtlBase* mtl, plLayerTex* layTex, plMaxNode* node, 
										 plLayerInterface* layerIFace)
{
	IParamBlock2* bitmapPB = layTex->GetParamBlockByID( plLayerTex::kBlkBitmap );
	if( !bitmapPB )
		return layerIFace;

	PBBitmap	*pbbm = nil;

	if( !bitmapPB->GetInt(kBmpUseBitmap) || !(pbbm = bitmapPB->GetBitmap(kBmpBitmap)) )
		return layerIFace;

	BitmapInfo	*bi = &pbbm->bi;
	if( !bi || !bi->Name() || !*bi->Name() )
		return layerIFace;

	const char* fileName = bi->Name();

	plAnimStealthNode* stealth = IGetEntireAnimation(mtl);

	const char* ext = plFileUtils::GetFileExt(fileName);
	bool isBink = ext && (stricmp(ext, "bik") == 0);
	bool isAvi  = ext &&(stricmp(ext, "avi") == 0);

	if (isBink || isAvi)
	{
		char movieName[256];
		sprintf(movieName, "avi/%s", plFileUtils::GetFileName(fileName));

		plLayerMovie* movieLayer = nil;
		const char* moviePostfix = nil;

		if (isBink)
		{
			movieLayer = TRACKED_NEW plLayerBink;
			moviePostfix = "_bink";
		}
		else if (isAvi)
		{
			movieLayer = TRACKED_NEW plLayerAVI;
			moviePostfix = "_avi";
		}

		TSTR movieKeyName = TSTR(layerIFace->GetKeyName()) + moviePostfix;
		hsgResMgr::ResMgr()->NewKey(movieKeyName, movieLayer, node->GetLocation());

		movieLayer->SetMovieName(movieName);
		movieLayer->Eval(0,0,0);

		plAnimTimeConvert& tc = movieLayer->GetTimeConvert();

		if (stealth)
		{
			if (stealth->GetAutoStart())
				tc.Start(0);
			else
				tc.Stop(true);

			tc.Loop(stealth->GetLoop());
		}
		tc.SetLoopPoints(0, movieLayer->GetLength());
		tc.SetBegin(0);
		tc.SetEnd(movieLayer->GetLength());

		movieLayer->AttachViaNotify(layerIFace);
		layerIFace = movieLayer;
	}

	return layerIFace;
}

plLayerInterface* IProcessLayerAnimation(plPassMtlBase* mtl, plLayerTex* layTex, plMaxNode* node, 
										 const char *name, plLayerInterface* layerIFace)
{
	hsControlConverter& cc = hsControlConverter::Instance();
	
	//
	// Look for animations. These will get tacked onto the base pass layer
	StdUVGen *uvGen = (StdUVGen*)layTex->GetTheUVGen();
	plLeafController* xfmCtl = cc.MakeMatrix44Controller(uvGen, node->GetName());

	if( !xfmCtl )
		return layerIFace;

	if( mtl->GetUseGlobal() )
	{
		plLayerSDLAnimation *SDLLayer = TRACKED_NEW plLayerSDLAnimation;
		TSTR animName = TSTR(name) + TSTR("_anim_") + TSTR(mtl->GetGlobalVarName());
		hsgResMgr::ResMgr()->NewKey(animName, SDLLayer, node->GetLocation());

		SDLLayer->SetVarName(mtl->GetGlobalVarName());
		SDLLayer->SetTransformCtl(xfmCtl);
		SDLLayer->AttachViaNotify(layerIFace);
		node->CheckSynchOptions(SDLLayer);
		
		return SDLLayer;
	}	

	// Don't need anymore, really was just for validation above
	delete xfmCtl;

	// If no valid animation track is chosen, return entire animation.
	int i, count = mtl->GetNumStealths();
	for( i = count - 1; i >= 0; i-- )
	{
		plAnimStealthNode *stealth = mtl->GetStealth( i );

		plLayerAnimation *noteAnim = TRACKED_NEW plLayerAnimation;
		node->CheckSynchOptions(noteAnim);

		const char *segName = stealth->GetSegmentName();
		bool isDefault = ( segName == nil || strcmp( segName, ENTIRE_ANIMATION_NAME ) == 0 ) ? true : false;

		TSTR animName = TSTR( name ) + ( ( isDefault ) ? TSTR( "_LayerAnim_" ) : ( TSTR("_LayerAnim") + TSTR( segName ) ) );
		hsgResMgr::ResMgr()->NewKey( animName, noteAnim, node->GetLocation() );

		StdUVGen *uvGen = (StdUVGen *)layTex->GetTheUVGen();
		plLeafController *segXfmCtl = cc.MakeMatrix44Controller( uvGen, node->GetName() );
		noteAnim->SetTransformCtl( segXfmCtl );

		// ATC conversion stuff
		stealth->StuffToTimeConvert( noteAnim->GetTimeConvert(), segXfmCtl->GetLength() );
		
		// Set segment name if we're not the default
		if( !isDefault )
			noteAnim->SetSegmentID( (char *)segName );

		// And attach!
		noteAnim->AttachViaNotify( layerIFace );

		// So the next time will attach to this layer...
		layerIFace = noteAnim;
	}

	return layerIFace;

}

plLayerInterface* IProcessAnimation(plPassMtlBase *mtl, plMaxNode *node, const char *name,
									plLayerInterface *layerIFace)
{
	hsControlConverter& cc = hsControlConverter::Instance();

	hsScalar maxLength = 0;
	//
	// Look for animations. These will get tacked onto the base pass layer
	Control *maxColCtl = mtl->GetPreshadeColorController();
	plController *colCtl = cc.MakeColorController(maxColCtl, node);
	
	Control *maxRunColCtl = nil;
	plController *runColCtl = nil;
	if( mtl->GetDiffuseColorLock() )
		maxRunColCtl = maxColCtl;
	else
		maxRunColCtl = mtl->GetRuntimeColorController();

	runColCtl = cc.MakeColorController(maxRunColCtl, node);

	Control *maxAmbCtl = mtl->GetAmbColorController();
	plController *ambCtl = cc.MakeColorController(maxAmbCtl, node);

	Control *maxOpaCtl = mtl->GetOpacityController();
	plLeafController *opaCtl = cc.MakeScalarController(maxOpaCtl, node);

	Control *maxSpecCtl = mtl->GetSpecularColorController();
	plController *specCtl = cc.MakeColorController(maxSpecCtl, node);

	// If there are no animations, return
	if (!colCtl && !ambCtl && !opaCtl && !runColCtl && !specCtl)
		return layerIFace;
	if( colCtl )
		maxLength = colCtl->GetLength();
	if( ambCtl && (ambCtl->GetLength() > maxLength) )
		maxLength = ambCtl->GetLength();
	if( opaCtl && (opaCtl->GetLength() > maxLength) )
		maxLength = opaCtl->GetLength();
	if( runColCtl && (runColCtl->GetLength() > maxLength) )
		maxLength = runColCtl->GetLength();
	if( specCtl && (specCtl->GetLength() > maxLength) )
		maxLength = specCtl->GetLength();

	if( mtl->GetUseGlobal() )
	{
		//if (!hsMaterialConverter::Instance().CheckValidityOfSDLVarAnim(mtl, mtl->GetGlobalVarName(), node))
		//	return layerIFace;
		
		plLayerSDLAnimation *SDLLayer = TRACKED_NEW plLayerSDLAnimation;
		TSTR animName = TSTR(name) + TSTR("_anim_") + TSTR(mtl->GetGlobalVarName());
		hsgResMgr::ResMgr()->NewKey(animName, SDLLayer, node->GetLocation());

		SDLLayer->SetVarName(mtl->GetGlobalVarName());
		node->CheckSynchOptions(SDLLayer);

		if (colCtl)
			SDLLayer->SetPreshadeColorCtl(colCtl);
		if (ambCtl)
			SDLLayer->SetAmbientColorCtl(ambCtl);
		if (opaCtl)
			SDLLayer->SetOpacityCtl(opaCtl);
		if( runColCtl )
			SDLLayer->SetRuntimeColorCtl( runColCtl );
		if( specCtl )
			SDLLayer->SetSpecularColorCtl( specCtl );

		SDLLayer->AttachViaNotify(layerIFace);

		return SDLLayer;
	}

	// Delete these, since they're no longer needed
	delete colCtl;
	delete ambCtl;
	delete opaCtl;
	delete runColCtl;
	delete specCtl;

	// Loop through the stealths, since each one represents a segment
	int i, count = mtl->GetNumStealths();
	for( i = count - 1; i >= 0; i-- )
	{
		plAnimStealthNode *stealth = mtl->GetStealth( i );

		plLayerAnimation *noteAnim = TRACKED_NEW plLayerAnimation;
		node->CheckSynchOptions(noteAnim);

		const char *segName = stealth->GetSegmentName();
		bool isDefault = ( segName == nil || strcmp( segName, ENTIRE_ANIMATION_NAME ) == 0 ) ? true : false;

		TSTR animName = TSTR( name ) + ( ( isDefault ) ? TSTR( "_anim" ) : ( TSTR("_anim_") + TSTR( segName ) ) );
		hsgResMgr::ResMgr()->NewKey( animName, noteAnim, node->GetLocation() );

		plController *noteColCtl = cc.MakeColorController( maxColCtl, node );
		plController *noteAmbCtl = cc.MakeColorController( maxAmbCtl, node );
		plController *noteOpaCtl = cc.MakeScalarController( maxOpaCtl, node );
		plController *noteSpecCtl = cc.MakeColorController( maxSpecCtl, node );
		plController *noteRunColCtl = cc.MakeColorController( maxColCtl, node );

		if( noteColCtl )
			noteAnim->SetPreshadeColorCtl( noteColCtl );
		if( noteAmbCtl )
			noteAnim->SetAmbientColorCtl( noteAmbCtl );
		if( noteOpaCtl )
			noteAnim->SetOpacityCtl( noteOpaCtl );
		if( noteRunColCtl )
			noteAnim->SetRuntimeColorCtl( noteRunColCtl );
		if( noteSpecCtl )
			noteAnim->SetSpecularColorCtl( noteSpecCtl );

		// ATC conversion stuff
		stealth->StuffToTimeConvert( noteAnim->GetTimeConvert(), maxLength );
		
		// Set segment name if we're not the default
		if( !isDefault )
			noteAnim->SetSegmentID( (char *)segName );

		// And attach!
		noteAnim->AttachViaNotify( layerIFace );

		// So the next time will attach to this layer...
		layerIFace = noteAnim;
	}

	// Cleanup
	return layerIFace;

}


hsBool hsMaterialConverter::IHasSubMtl(Mtl* base, Mtl* sub)
{
	if( !(base && sub) )
		return false;

	if( base == sub )
		return true;

	int nSub = base->NumSubMtls();
	int i;
	for( i = 0; i < nSub; i++ )
	{
		if( IHasSubMtl(base->GetSubMtl(i), sub) )
			return true;
	}
	return false;
}

int hsMaterialConverter::IFindSubIndex(plMaxNode* node, Mtl* mtl)
{
	Mtl* baseMtl = node->GetMtl();
	if( !IsMultiMat(baseMtl) )
		return 0;

	int i;
	for( i = 0; i < mtl->NumSubMtls(); i++ )
	{
		if( IHasSubMtl(mtl->GetSubMtl(i), mtl) )
			return i;
	}
	return -1;
}

//
// Converters for different kinds of materials (Plasma, Environ, Standard)
//

// Now handles both plPassMtl and plDecalMtl which derive from plPassMtlBase
hsBool hsMaterialConverter::IProcessPlasmaMaterial(Mtl *mtl, plMaxNode *node, hsGMaterial *mat, const char* name)
{
	hsGuardBegin("hsMaterialConverter::IProcessPlasmaMaterial");

	plLocation nodeLoc= node->GetLocation(); 
	char* dbgNodeName = node->GetName();
	
	int initNumLayers = mat->GetNumLayers();

	if(!mtl)
	{
		fErrorMsg->Set(!mtl, "Material Loading Error", "No material in ProcessPlasmaMaterial!").Show();
		//hsAssert(mtl, "No material in ProcessPlasmaMaterial!");

		fErrorMsg->Set();
		return false;
	}

	int multiIndex = IFindSubIndex(node, mtl);
	hsBool needAlphaHack = node->AlphaHackLayersNeeded(multiIndex) > 0;

	plPassMtlBase *passBase = (plPassMtlBase *)mtl;
	//
	// Color / Opacity
	//
	Color amb  = passBase->GetAmbColor();
	Color dif  = passBase->GetColor();
	Color runDif = passBase->GetRuntimeColor();
	if( passBase->GetDiffuseColorLock() )
		runDif = dif;

	float opac = float(passBase->GetOpacity()) / 100.0f;

	fErrorMsg->Set(opac  < 0.0 || opac  > 1.0, dbgNodeName, "Bad opacity on mat %s", name).CheckAndAsk();
	fErrorMsg->Set(dif.r < 0.0 || dif.r > 1.0, dbgNodeName, "Bad color (r) on mat %s", name).CheckAndAsk();
	fErrorMsg->Set(dif.g < 0.0 || dif.g > 1.0, dbgNodeName, "Bad color (g) on mat %s", name).CheckAndAsk();
	fErrorMsg->Set(dif.b < 0.0 || dif.b > 1.0, dbgNodeName, "Bad color (b) on mat %s", name).CheckAndAsk();
	fErrorMsg->Set(runDif.r < 0.0 || runDif.r > 1.0, dbgNodeName, "Bad runtime color (r) on mat %s", name).CheckAndAsk();
	fErrorMsg->Set(runDif.g < 0.0 || runDif.g > 1.0, dbgNodeName, "Bad runtime color (g) on mat %s", name).CheckAndAsk();
	fErrorMsg->Set(runDif.b < 0.0 || runDif.b > 1.0, dbgNodeName, "Bad runtime color (b) on mat %s", name).CheckAndAsk();

//	Color col = dif - amb;
	Color col = dif;

	if( passBase->GetDiffuseColorLock() )
		runDif = dif;

	// If we've got No Preshading explicitly set, then set the 
	// PreshadeColor to black (so anything in the vertex color
	// will get ignored at runtime).
	if( node->GetRunTimeLight() )
		col.Black();
	//
	// Blend flags
	//
	UInt32 blendFlags = 0;
	if( node->GetAlphaTestHigh() || passBase->GetAlphaTestHigh() )
		blendFlags |= hsGMatState::kBlendAlphaTestHigh;

	// Z Flag
	if (passBase->GetZOnly())
		blendFlags |= hsGMatState::kBlendNoColor;

	//
	// Shade flags
	//
	UInt32 shadeFlags = 0;
	if (passBase->GetSoftShadow())
		shadeFlags |= hsGMatState::kShadeSoftShadow;
	if (passBase->GetNoProj())
		shadeFlags |= hsGMatState::kShadeNoProjectors;
	if (passBase->GetVertexShade())
		shadeFlags |= hsGMatState::kShadeVertexShade;
	if (passBase->GetNoShade())
		shadeFlags |= hsGMatState::kShadeNoShade;
	if (passBase->GetNoFog())
		shadeFlags |= hsGMatState::kShadeReallyNoFog;
	if (passBase->GetWhite())
		shadeFlags |= hsGMatState::kShadeWhite;
	/// Emissive flag, from basic parameters
	if( passBase->GetEmissive() )
	{
		shadeFlags |= hsGMatState::kShadeEmissive;
		col.r = col.g = col.b = 0.0f;
		runDif = col;
	}
	if( node->GetIsGUI() )
	{
		// We don't want materials on any GUI objects to *ever* be fogged...4.25.2002 mcn
		shadeFlags |= hsGMatState::kShadeReallyNoFog;
	}

	// Specular flags
	float	shine	= 0.0;
	Color	specColor = Color(0,0,0);

	if (passBase->GetUseSpec())
	{
		shadeFlags |= hsGMatState::kShadeSpecular;
		shine = passBase->GetShine();
		specColor = passBase->GetSpecularColor();
	}

	//
	// Misc flags...
	//
	UInt32 miscFlags = 0;
	if (passBase->GetBasicWire())
		miscFlags |= hsGMatState::kMiscWireFrame;
	if (passBase->GetMeshOutlines())
		miscFlags |= hsGMatState::kMiscDrawMeshOutlines;
	if( !node->GetDup2Sided() && passBase->GetTwoSided() )
		miscFlags |= hsGMatState::kMiscTwoSided;

	//
	// Z flags
	//
	UInt32 zFlags = 0;
	if (passBase->GetZClear())
		zFlags |= hsGMatState::kZClearZ;
	if (passBase->GetZNoRead())
		zFlags |= hsGMatState::kZNoZRead;
	if (passBase->GetZNoWrite())
		zFlags |= hsGMatState::kZNoZWrite;
	if (passBase->GetZInc())
		zFlags |= hsGMatState::kZIncLayer;
	
	// Decal flag settings
	if (IsDecalMat(mtl)) 
	{
		zFlags |= hsGMatState::kZIncLayer | hsGMatState::kZNoZWrite;
		mat->SetCompositeFlags(mat->GetCompositeFlags() | hsGMaterial::kCompDecal);
	}	


	// We're not using specular highlights?
	if (shadeFlags & hsGMatState::kShadeSpecularHighlight)
	{
		shadeFlags &= ~(hsGMatState::kShadeSpecular 
						| hsGMatState::kShadeSpecularHighlight 
						| hsGMatState::kShadeSpecularAlpha 
						| hsGMatState::kShadeSpecularColor);
	}

	hsBool preserveUVOffset = PreserveUVOffset(mtl);

	//
	// Process base layer
	//
	plLayer* baseLay = nil;
	Texmap *baseTex = passBase->GetBaseLayer();
	plPlasmaMAXLayer *plasmaLayer;
	if (baseTex && ( plasmaLayer = plPlasmaMAXLayer::GetPlasmaMAXLayer( baseTex ) ) != nil )
	{
		plLayerInterface *layerIFace = plLayerConverter::Instance().ConvertTexmap( baseTex, node, 0, preserveUVOffset, false ); 

		baseLay = plLayer::ConvertNoRef(layerIFace->BottomOfStack());
		
		fErrorMsg->Set(!baseLay, node->GetName(), "Found no layer at end of IProcessPlasmaLayer()").Check();
			
		// This sucks. If we're adding a base layer on top of layers whose emissive flags don't match,
		// we have to set kMiscRestartPassHere on this layer
		if( mat->GetNumLayers() > 0 )
		{
			UInt32	lastShades = mat->GetLayer( mat->GetNumLayers() - 1 )->GetShadeFlags();

			if( ( lastShades ^ shadeFlags ) & hsGMatState::kShadeEmissive )
				baseLay->SetMiscFlags( baseLay->GetMiscFlags() | hsGMatState::kMiscRestartPassHere );
		}

		baseLay->SetBlendFlags(baseLay->GetBlendFlags() | blendFlags);
		baseLay->SetShadeFlags(baseLay->GetShadeFlags() | shadeFlags);
		baseLay->SetZFlags(baseLay->GetZFlags() | zFlags);
		baseLay->SetMiscFlags(baseLay->GetMiscFlags() | miscFlags);	// 3.28.2001 mcn - Who didn't put this in?

		if (baseLay->GetShadeFlags() & hsGMatState::kShadeSpecular) 
		{
			baseLay->SetSpecularColor( hsColorRGBA().Set( specColor.r, specColor.g, specColor.b, hsScalar1 ) );
			baseLay->SetSpecularPower(shine);
		}

		baseLay->SetAmbientColor(hsColorRGBA().Set(amb.r, amb.g, amb.b, hsScalar1));
		baseLay->SetPreshadeColor(hsColorRGBA().Set(col.r, col.g, col.b, hsScalar1));
		baseLay->SetRuntimeColor(hsColorRGBA().Set(runDif.r, runDif.g, runDif.b, hsScalar1));
		baseLay->SetOpacity( opac );		// Don't scale the material color by this if we're add blending; do that later

		UInt32 blendType = 0;
		switch (passBase->GetOutputBlend())
		{
		case plPassMtlBase::kBlendAlpha:		blendType |= hsGMatState::kBlendAlpha;	break;
		case plPassMtlBase::kBlendAdd:			blendType |= hsGMatState::kBlendAdd;	break;
		}
		baseLay->SetBlendFlags(baseLay->GetBlendFlags() | blendType);
		if( (opac < 1.f) && (blendType & hsGMatState::kBlendAlpha) )
			baseLay->SetMiscFlags( baseLay->GetMiscFlags() | hsGMatState::kMiscRestartPassHere );

		plLayerInterface *layerAnim = IProcessAnimation(passBase, node, layerIFace->GetKeyName(), layerIFace);

		layerAnim = IProcessLayerAnimation(passBase, (plLayerTex*)baseTex, node, layerIFace->GetKeyName(), layerAnim);

		layerAnim = IProcessLayerMovie(passBase, (plLayerTex*)baseTex, node, layerAnim);

		layerAnim->Eval(0,0,0);

		IAddLayerToMaterial(mat, layerAnim);

	}

	// If we don't have a base layer, there's not much left to talk about.
	if( !baseLay )
		return true;

	if( baseLay && ( IsBumpLayer(baseTex) || IsBumpMtl( mtl ) ) )
		baseLay->SetMiscFlags(baseLay->GetMiscFlags() | hsGMatState::kMiscBumpLayer);
	if( baseLay && node->GetWaterDecEnv() )
		baseLay->SetShadeFlags(baseLay->GetShadeFlags() | hsGMatState::kShadeEnvironMap);

	// If the top layer is on, and there is a valid base layer
	if (passBase->GetTopLayerOn() && baseLay)
	{
		Texmap *texMap = passBase->GetTopLayer();
		plPlasmaMAXLayer *plasmaTopLayer;
		if (texMap && ( plasmaTopLayer = plPlasmaMAXLayer::GetPlasmaMAXLayer( baseTex ) ) != nil )
		{
			// Blend flags (do this first so we can pass it to IProcessPlasmaLayer())
			UInt32 blendIndex = passBase->GetLayerBlend();
			if( needAlphaHack
				&&(passBase->GetOutputBlend() == plPassMtlBase::kBlendAlpha)
				&&(blendIndex == plPassMtlBase::kBlendAdd || blendIndex == plPassMtlBase::kBlendMult) )
			{
				blendIndex = plPassMtlBase::kBlendAlpha;
				if (fErrorMsg->Set(!(fWarned & kWarnedAlphaAddCombo), node->GetName(), 
								   "Material \"%s\" has Output Blending set to Alpha, but Layer Blending is not. "
								   "This combination requires RunTime Lighting or give up vertex alpha and opacity animation."
								   "Using a layer blend of alpha for now.", 
								   passBase->GetName()).CheckAskOrCancel() )
					fWarned |= kWarnedAlphaAddCombo;
			}

			UInt32 blendType = 0;
			switch (blendIndex)
			{
			case plPassMtlBase::kBlendAlpha:		
				blendType |= hsGMatState::kBlendAlpha;	
				break;
			case plPassMtlBase::kBlendAdd:			
				blendType |= hsGMatState::kBlendAdd;	
				break;
			case plPassMtlBase::kBlendMult:		
				blendType |= hsGMatState::kBlendMult;	
				break;
			}

			plLayerInterface *layerIFace = plLayerConverter::Instance().ConvertTexmap( texMap, node, blendType, preserveUVOffset, true ); 

			if( layerIFace != nil )
			{
				if( (baseLay->GetBlendFlags() | layerIFace->GetBlendFlags()) & (hsGMatState::kBlendNoTexColor | hsGMatState::kBlendNoTexAlpha) )
					baseLay->SetMiscFlags(baseLay->GetMiscFlags() | hsGMatState::kMiscBindNext);

				if( blendIndex == plPassMtlBase::kBlendMult )
					baseLay->SetMiscFlags(baseLay->GetMiscFlags() | hsGMatState::kMiscBindNext);

				plLayer* hLay = plLayer::ConvertNoRef(layerIFace->BottomOfStack());

				fErrorMsg->Set(!hLay, node->GetName(), "Found no layer at end of IProcessPlasmaLayer()").Check();

				hLay->SetBlendFlags(hLay->GetBlendFlags() | blendType | blendFlags );
				hLay->SetShadeFlags(hLay->GetShadeFlags() | shadeFlags);
				// 5.29.2001 mcn - Upper layers are *supposed* to have this set on them
//				hLay->SetZFlags( hLay->GetZFlags() | zFlags | hsGMatState::kZNoZWrite | hsGMatState::kZIncLayer );
				// 2/28 2002 - mf - upper layers don't need ZIncLayer (because we no longer clip them).
				hLay->SetZFlags( hLay->GetZFlags() | zFlags | hsGMatState::kZNoZWrite );

				if (hLay->GetShadeFlags() & hsGMatState::kShadeSpecular) 
				{
					hLay->SetSpecularColor( hsColorRGBA().Set( specColor.r, specColor.g, specColor.b, hsScalar1 ) );
					hLay->SetSpecularPower(shine);
				}

				hLay->SetAmbientColor(hsColorRGBA().Set(amb.r, amb.g, amb.b, hsScalar1));
				hLay->SetPreshadeColor(hsColorRGBA().Set(col.r, col.g, col.b, hsScalar1));
				hLay->SetRuntimeColor(hsColorRGBA().Set(runDif.r, runDif.g, runDif.b, hsScalar1));
				hLay->SetOpacity( opac );

				if( IsBumpLayer(texMap) || IsBumpMtl( mtl ) )
					hLay->SetMiscFlags(hLay->GetMiscFlags() | hsGMatState::kMiscBumpLayer);

				hsBool canFunk = !needAlphaHack;
				// Base layer fixups
				// Yet another attempt to protect the artists from themselves. Setting the 
				// alpha combine to mult or add, when the output blend is opaque is at best meaningless
				// and can be dangerous. Just ignore them.
				if( baseLay->GetBlendFlags() & hsGMatState::kBlendAlpha )
				{
					switch (passBase->GetOutputAlpha())
					{
					case plPassMtlBase::kAlphaMultiply:
						hLay->SetBlendFlags(hLay->GetBlendFlags() | hsGMatState::kBlendAlphaMult);
						baseLay->SetMiscFlags(baseLay->GetMiscFlags() | hsGMatState::kMiscBindNext);
						canFunk = false;
						break;
					case plPassMtlBase::kAlphaAdd:
						hLay->SetBlendFlags(hLay->GetBlendFlags() | hsGMatState::kBlendAlphaAdd);
						baseLay->SetMiscFlags(baseLay->GetMiscFlags() | hsGMatState::kMiscBindNext);
						canFunk = false;
						break;
					}
				}

				if( canFunk && IHasFunkyOpacity(node, baseTex) )
				{
					IAppendFunkyLayer(node, baseTex, mat);
					// warn if requested but can't?
				}

				layerIFace = IProcessLayerAnimation(passBase, (plLayerTex*)texMap, node, layerIFace->GetKeyName(), layerIFace);

				layerIFace = IProcessLayerMovie(passBase, (plLayerTex*)baseTex, node, layerIFace);

				layerIFace->Eval(0,0,0);

				IAddLayerToMaterial(mat, layerIFace);

				if( canFunk && IHasFunkyOpacity(node, texMap) )
				{
					IAppendFunkyLayer(node, texMap, mat);
					// warn if requested but can't?
				}
			}
		}
	}
	else
	{
		if( IHasFunkyOpacity(node, baseTex) )
		{
			if( !needAlphaHack )
			{
				IAppendFunkyLayer(node, baseTex, mat);
			}
			else
			{
				// warn?
			}
		}
	}

	return true;
	hsGuardEnd; 
}

void hsMaterialConverter::IAddLayerToMaterial(hsGMaterial *mat, plLayerInterface *layer)
{
	plMatRefMsg* msg = TRACKED_NEW plMatRefMsg(mat->GetKey(), plRefMsg::kOnCreate, -1, plMatRefMsg::kLayer);
	hsgResMgr::ResMgr()->AddViaNotify(layer->GetKey(), msg, plRefFlags::kActiveRef);
}

//
// Functions called by the converters up above...
//
#include "../MaxPlasmaMtls/Layers/plLayerTex.h"

//// IMustBeUniqueMaterial ////////////////////////////////////////////////////
//	Fun stuff here. If one of the layers of the material is a dynamic EnvMap,
//	and its anchor is set to nil (i.e. "self"), then the layer must be unique
//	for each object its applied to. However, that means the material must
//	*ALSO* be unique. So this function will tell us whether we have to make
//	a unique material.

hsBool	hsMaterialConverter::IMustBeUniqueMaterial( Mtl *mtl )
{
	hsGuardBegin( "hsMaterialConverter::IMustBeUniqueMaterial" );

	if( !mtl )
		return false;

	const char	*dbgName = mtl->GetName();

	if( IsMultiMat( mtl ) || IsMultipassMat( mtl ) || IsCompositeMat( mtl ) )
	{
		int iMtl;
		for( iMtl = 0; iMtl < mtl->NumSubMtls(); iMtl++ )
		{
			if( IMustBeUniqueMaterial( mtl->GetSubMtl( iMtl ) ) ) 
				return true;
		}
		return false;
	}
	else if( IsParticleMat( mtl ) )
		return false;
	else if( mtl->ClassID() == PASS_MTL_CLASS_ID )
	{ 
		// It is std material.	does not have any submaterials
		plPassMtlBase		*passMtl = (plPassMtlBase *)mtl;

		if( IMustBeUniqueLayer( passMtl->GetBaseLayer() ) )
			return true;

		if( passMtl->GetTopLayerOn() && passMtl->GetTopLayer() && 
			IMustBeUniqueLayer( passMtl->GetTopLayer() ) )
			return true;

		return false;
	}
	
	return false;

	hsGuardEnd; 
}

//// IMustBeUniqueLayer ///////////////////////////////////////////////////////
//	Check for a single layer (see IMustBeUniqueMaterial())

hsBool	hsMaterialConverter::IMustBeUniqueLayer( Texmap *layer )
{
	plPlasmaMAXLayer	*plasmaLayer = plPlasmaMAXLayer::GetPlasmaMAXLayer( layer );
	if( plasmaLayer != nil )
		return plasmaLayer->MustBeUnique();

	return false;
}

hsBool hsMaterialConverter::IUVGenHasDynamicScale(plMaxNode* node, StdUVGen *uvGen)
{
	hsGuardBegin("hsMaterialConverter::IUVGenHasDynamicScale");

	Control *ctl = nil;

	hsControlConverter::Instance().GetControllerByName(uvGen, TSTR("U Tiling"), ctl);
	if( ctl && (ctl->NumKeys() > 1) )
		return true;
	hsControlConverter::Instance().GetControllerByName(uvGen, TSTR("V Tiling"), ctl);
	if( ctl && (ctl->NumKeys() > 1) )
		return true;

	return false;
	hsGuardEnd; 
}

void hsMaterialConverter::IScaleLayerOpacity(plLayer* hLay, hsScalar scale)
{
	hsGuardBegin("hsMaterialConverter::IScaleLayerOpacity");

	if( scale < 1.f )
	{
		if( !(hLay->GetBlendFlags() & ~(hsGMatState::kBlendAlpha | hsGMatState::kBlendAntiAlias)) )
		{
			hLay->SetBlendFlags(hLay->GetBlendFlags() | hsGMatState::kBlendAlpha);
		}

		hsScalar opac = hLay->GetOpacity();
		opac *= scale;
		hLay->SetOpacity(scale);
	}
	hsGuardEnd;
}

hsGMaterial *hsMaterialConverter::ICheckForProjectedTexture(plMaxNode *node)
{
	hsGuardBegin("hsMaterialConverter::ICheckForProjectedTexture");

	char *nodeName = node->GetName();
	Object *obj = node->EvalWorldState(fConverterUtils.GetTime(fInterface)).obj;
	LightObject *light = (LightObject*)obj->ConvertToType(fConverterUtils.GetTime(fInterface), 
			Class_ID(SPOT_LIGHT_CLASS_ID,0));
	if( !light )
		light = (LightObject*)obj->ConvertToType(fConverterUtils.GetTime(fInterface), 
			Class_ID(FSPOT_LIGHT_CLASS_ID,0));
	if( !light )
		light = (LightObject*)obj->ConvertToType(fConverterUtils.GetTime(fInterface), 
			Class_ID(OMNI_LIGHT_CLASS_ID,0));

	if( !light )
		light = (LightObject*)obj->ConvertToType(fConverterUtils.GetTime(fInterface), 
			Class_ID(DIR_LIGHT_CLASS_ID,0));

	if( !light )
		light = (LightObject*)obj->ConvertToType(fConverterUtils.GetTime(fInterface), 
			Class_ID(TDIR_LIGHT_CLASS_ID,0));

	if( light && light->GetProjector() )
	{
		Texmap *projMap;
		projMap = light->GetProjMap();
		return IWrapTextureInMaterial(projMap, node);
	}

	return nil;
	hsGuardEnd; 
}

hsGMaterial *hsMaterialConverter::IWrapTextureInMaterial(Texmap *texMap, plMaxNode *node)
{
	hsGuardBegin("hsMaterialConverter::IWrapTextureInMaterial");
	plLocation nodeLoc = node->GetLocation();
	//
	// Add material to list
	//
	int found=FALSE;
	char *nodeName = node->GetName();
	CStr className;
	texMap->GetClassName(className);

	// We want to keep it.	Handle appropriately.
	BitmapTex *bitmapTex = (BitmapTex *)texMap; 	   
	char txtFileName[256];
	strcpy(txtFileName, bitmapTex->GetMapName());

//	hsRegistryKey* key = hsgResMgr::ResMgr()->FindKey(txtFileName, hsGMaterial::Index());
	plKey key = node->FindPageKey( hsGMaterial::Index(), txtFileName );

	hsGMaterial *hMat = key ? hsGMaterial::ConvertNoRef(key->GetObjectPtr()) : nil;
	if( hMat )
	{
		CopyMaterialLODToTextures(hMat);
		return hMat;
	}

	hMat = TRACKED_NEW hsGMaterial;

	plLayer* hLay = TRACKED_NEW plLayer;
	hLay->InitToDefault();

	hsgResMgr::ResMgr()->NewKey(txtFileName, hLay,nodeLoc);

	if (texMap->ClassID() == hsMaxLayerClassID) 
	{
//		IProcessPlasmaLayer(hLay, nil, texMap, node, 0);
	}
	else 
	{
		//
		// Create hsGBitmap texture file from texture.
		// Add texture name to list if not already there.
		//
		
		// fill in benign values for our material
		hLay->SetAmbientColor(hsColorRGBA().Set(0.f, 0.f, 0.f, 1.f));
		hLay->SetRuntimeColor(hsColorRGBA().Set(1.f, 1.f, 1.f, 1.f));

		// If we've got No Preshading explicitly set, then set the 
		// PreshadeColor to black (so anything in the vertex color
		// will get ignored at runtime).
		if( node->GetRunTimeLight() )
			hLay->SetPreshadeColor(hsColorRGBA().Set(0.f, 0.f, 0.f, 1.f));
		else
			hLay->SetPreshadeColor(hsColorRGBA().Set(1.f, 1.f, 1.f, 1.f));
		hLay->SetOpacity(1.f);
		
		UInt32 autoStart = 0;
		char *nodeName = node->GetName();
		char *texName = bitmapTex->GetName();
		// BEGIN OVERRIDE
		if (ALPHA_NONE == bitmapTex->GetAlphaSource())
		{
			// TEMP
			// This should be a blend modifier, not a load flag - mf
			// hLay->SetLoadFlags(hLay->GetLoadFlags() | hsGLayer::kLoadNoAlpha);
		}
		else if (!(hLay->GetBlendFlags() & (hsGMatState::kBlendMask & ~hsGMatState::kBlendAntiAlias)))
		{
			if (ALPHA_FILE == bitmapTex->GetAlphaSource())
				hLay->SetBlendFlags(hLay->GetBlendFlags() | hsGMatState::kBlendAlpha);
		}
#if 0		
		// Texture map info
		if (texMap != NULL)
		{
			IInitAttrTexture(node, nil, hLay, texMap, nodeName);
		}
#endif
	}

	key = hsgResMgr::ResMgr()->NewKey(txtFileName, hMat,nodeLoc);
	
	IAddLayerToMaterial(hMat, hLay);

	CopyMaterialLODToTextures(hMat);

	return hMat;
	hsGuardEnd; 
}

Mtl* hsMaterialConverter::GetBaseMtl(Mtl* mtl)
{
	hsGuardBegin("hsMaterialConverter::GetBaseMtl");

	return (mtl);
	hsGuardEnd; 
}

Mtl* hsMaterialConverter::GetBaseMtl(plMaxNode* node)
{ 
	return GetBaseMtl(node->GetMtl()); 
}

enum plFunkyNess
{
	kFunkyNone			= 0x0,
	kFunkyDistance		= 0x1,
	kFunkyNormal		= 0x2,
	kFunkyReflect		= 0x4,
	kFunkyUp			= 0x8,
	kFunkyMask			= kFunkyDistance
						| kFunkyNormal
						| kFunkyReflect
						| kFunkyUp,
	kFunkyAdd
};

static float IClampToRange(float v, float lo, float hi)
{
	if( v < lo )
		v = lo;
	else if( v > hi )
		v = hi;

	return v;
}

UInt32 hsMaterialConverter::IGetOpacityRanges(plMaxNode* node, Texmap* texMap, hsScalar& tr0, hsScalar& op0, hsScalar& op1, hsScalar& tr1)
{
	if( node->HasFade() )
	{
		Box3 fade = node->GetFade();
		tr0 = fade.Min()[0];
		op0 = fade.Min()[1];
		op1 = fade.Max()[1];
		tr1 = fade.Max()[0];

		UInt32 funkyType = kFunkyDistance;

		if( (tr0 == op0) && (op1 == tr1) )
		{
			funkyType = kFunkyNone;
		}
		// See if we're going opaque to transparent to opaque. Then we need the Additive LUT
		else
		if( (tr0 > op0) && (op1 > tr1) )
		{
			funkyType |= kFunkyAdd;
		}
		return funkyType;
	}


	if( !(texMap && texMap->GetName() && (strlen(texMap->GetName()) > 3)) )
		return kFunkyNone;

	const char* field = texMap->GetName()+3;
	float f0, f1, f2, f3;
	int code = sscanf(field, "%g,%g,%g,%g", &f0, &f1, &f2, &f3);
	if( !code || (code == EOF) )
	{
		return kFunkyNone;
	}
	switch( code )
	{
	case 1:
		tr1 = f0;
		tr0 = op0 = op1 = 0;
		break;
	case 2:
		op1 = f0;
		tr1 = f1;
		tr0 = op0 = 0;
		break;
	case 3:
		tr0 = 0;
		op0 = f0;
		op1 = f1;
		tr1 = f2;
		break;
	case 4:
		tr0 = f0;
		op0 = f1;
		op1 = f2;
		tr1 = f3;
		break;

	default:
		hsAssert(false, "Whoa, am i confused!!!");
		return kFunkyNone;
	}

	UInt32 funkyType = IGetFunkyType(texMap);
	switch( funkyType & kFunkyMask )
	{
	case kFunkyDistance:
		tr0 = IClampToRange(tr0, 0, 1.e33f);
		op0 = IClampToRange(op0, 0, 1.e33f);
		op1 = IClampToRange(op1, 0, 1.e33f);
		tr1 = IClampToRange(tr1, 0, 1.e33f);
		break;
	case kFunkyNormal:
	case kFunkyUp:
		tr0 = IClampToRange(tr0, 0, 180.f);
		op0 = IClampToRange(op0, 0, 180.f);
		op1 = IClampToRange(op1, 0, 180.f);
		tr1 = IClampToRange(tr1, 0, 180.f);
		break;
	case kFunkyReflect:
		tr0 = IClampToRange(tr0, 0, 180.f);
		op0 = IClampToRange(op0, 0, 180.f);
		op1 = IClampToRange(op1, 0, 180.f);
		tr1 = IClampToRange(tr1, 0, 180.f);
		break;
	}
	if( tr0 > tr1 )
	{
		hsScalar t;
		t = tr0;
		tr0 = tr1;
		tr1 = t;

		t = op0;
		op0 = op1;
		op1 = t;
	}
	// Check for degenerate ranges.
	if( (tr0 == op0) && (op1 == tr1) )
	{
		funkyType = kFunkyNone;
	}
	// See if we're going opaque to transparent to opaque. Then we need the Additive LUT
	else
	if( (tr0 > op0) && (op1 > tr1) )
	{
		funkyType |= kFunkyAdd;
	}
	switch( funkyType & kFunkyMask )
	{
	case kFunkyDistance:
		break;
	case kFunkyNormal:
	case kFunkyUp:
		tr0 = hsCosine(hsScalarDegToRad(tr0));
		op0 = hsCosine(hsScalarDegToRad(op0));
		op1 = hsCosine(hsScalarDegToRad(op1));
		tr1 = hsCosine(hsScalarDegToRad(tr1));
		break;
	case kFunkyReflect:
		tr0 = hsCosine(hsScalarDegToRad(tr0));
		op0 = hsCosine(hsScalarDegToRad(op0));
		op1 = hsCosine(hsScalarDegToRad(op1));
		tr1 = hsCosine(hsScalarDegToRad(tr1));
		break;
	}
	return funkyType;
}

UInt32 hsMaterialConverter::IGetFunkyType(Texmap* texMap)
{
	if( texMap && texMap->GetName() && *texMap->GetName() )
	{
		// Distance opacity
		if( !_strnicmp(texMap->GetName(), "%%%", 3) )
			return kFunkyDistance;
		// Angle opacity - normal
		if( !_strnicmp(texMap->GetName(), "@@@", 3) )
			return kFunkyNormal;
		// Angle opacity - reflection
		if( !_strnicmp(texMap->GetName(), "$$$", 3) )
			return kFunkyReflect;
		if( !_strnicmp(texMap->GetName(), "!!!", 3) )
			return kFunkyUp;
	}
	return kFunkyNone;
}

hsBool hsMaterialConverter::IHasFunkyOpacity(plMaxNode* node, Texmap* texMap)
{
	float tr0, cp0, cp1, tr1;
	return IGetOpacityRanges(node, texMap, tr0, cp0, cp1, tr1) != kFunkyNone;
}

void hsMaterialConverter::IAppendFunkyLayer(plMaxNode* node, Texmap* texMap, hsGMaterial* mat)
{
	plLayer* prevLay = plLayer::ConvertNoRef(mat->GetLayer(mat->GetNumLayers()-1)->BottomOfStack());
	hsAssert(prevLay, "Lost our base layer");
	if( prevLay )
		prevLay->SetMiscFlags(prevLay->GetMiscFlags() | hsGMatState::kMiscBindNext);

	float tr0, op0, op1, tr1;
	UInt32 funkyType = IGetOpacityRanges(node, texMap, tr0, op0, op1, tr1);

	if( funkyType == kFunkyNone )
		return;

	hsMatrix44 uvwXfm;
	uvwXfm.Reset();
	uvwXfm.fMap[0][0] = uvwXfm.fMap[1][1] = uvwXfm.fMap[2][2] = 0;
	uvwXfm.NotIdentity();

	if( op0 != tr0 )
	{
		uvwXfm.fMap[0][2] = -1.f / (tr0 - op0);
		uvwXfm.fMap[0][3] = uvwXfm.fMap[0][2] * -tr0;
	}
	else
	{
		uvwXfm.fMap[0][3] = 1.f;
	}

	if( op1 != tr1 )
	{
		uvwXfm.fMap[1][2] = -1.f / (tr1 - op1);
		uvwXfm.fMap[1][3] = uvwXfm.fMap[1][2] * -tr1;
	}
	else
	{
		uvwXfm.fMap[1][3] = 1.f;
	}

	plBitmap* funkRamp = IGetFunkyRamp(node, funkyType);

	char name[512];
	sprintf(name, "%s_funkRamp", prevLay->GetKey()->GetName());

	plLayer* layer = TRACKED_NEW plLayer;
	layer->InitToDefault();
	hsgResMgr::ResMgr()->NewKey(name, layer, node->GetLocation());
	hsgResMgr::ResMgr()->AddViaNotify(funkRamp->GetKey(), TRACKED_NEW plLayRefMsg(layer->GetKey(), plRefMsg::kOnCreate, 0, plLayRefMsg::kTexture), plRefFlags::kActiveRef);

	layer->SetAmbientColor(hsColorRGBA().Set(1.f, 1.f, 1.f, 1.f));
	layer->SetPreshadeColor(hsColorRGBA().Set(0, 0, 0, 1.f));
	layer->SetRuntimeColor(hsColorRGBA().Set(0, 0, 0, 1.f));

	layer->SetZFlags(hsGMatState::kZNoZWrite);
	UInt32 blendFlags = hsGMatState::kBlendAlpha | hsGMatState::kBlendNoTexColor | hsGMatState::kBlendAlphaMult;
	layer->SetBlendFlags(blendFlags);
	layer->SetClampFlags(hsGMatState::kClampTexture);

	layer->SetTransform(uvwXfm);

	switch( funkyType & kFunkyMask )
	{
	case kFunkyDistance:
		layer->SetUVWSrc(plLayerInterface::kUVWPosition);
		layer->SetMiscFlags(layer->GetMiscFlags() | hsGMatState::kMiscNoShadowAlpha);
		break;
	case kFunkyNormal:
		layer->SetUVWSrc(plLayerInterface::kUVWNormal);
		break;
	case kFunkyUp:
		layer->SetUVWSrc(plLayerInterface::kUVWNormal);
		layer->SetMiscFlags(layer->GetMiscFlags() | hsGMatState::kMiscOrthoProjection);
		break;
	case kFunkyReflect:
		layer->SetUVWSrc(plLayerInterface::kUVWReflect);
		break;
	}

	hsgResMgr::ResMgr()->AddViaNotify(layer->GetKey(), TRACKED_NEW plMatRefMsg(mat->GetKey(), plRefMsg::kOnCreate, 
 																       -1, plMatRefMsg::kLayer), plRefFlags::kActiveRef);

}

plBitmap* hsMaterialConverter::IGetFunkyRamp(plMaxNode* node, UInt32 funkyType)
{
	const char* funkName = funkyType & kFunkyAdd ? "FunkyRampAdd" : "FunkyRampMult";

	const int kLUTWidth = 16;
	const int kLUTHeight = 16;

	// NOTE: CreateBlankMipmap might return an old mipmap if it was already created, so in those
	// cases we wouldn't really need to re-write the texture. However, there's no harm in doing so,
	// and since we're close to Alpha, I don't want to shake up the code any more than absolutely 
	// necessary. -mcn
	plMipmap *texture = plBitmapCreator::Instance().CreateBlankMipmap( kLUTWidth, kLUTHeight, plMipmap::kARGB32Config, 1, funkName, node->GetLocation() );

	UInt32* pix = (UInt32*)texture->GetImage();

	if( funkyType & kFunkyAdd )
	{
		int i;
		for( i = 0; i < kLUTHeight; i++ )
		{
			int j;
			for( j = 0; j < kLUTWidth; j++ )
			{
				float x = float(j) / (kLUTWidth-1);
				float y = float(i) / (kLUTHeight-1);
				if( x < y )
					x = y;
				*pix++ = MakeUInt32Color(1.f, 1.f, 1.f, x);
			}
		}
	}
	else
	{
		int i;
		for( i = 0; i < kLUTHeight; i++ )
		{
			int j;
			for( j = 0; j < kLUTWidth; j++ )
			{
				float x = float(j) / (kLUTWidth-1);
				float y = float(i) / (kLUTHeight-1);
				*pix++ = MakeUInt32Color(1.f, 1.f, 1.f, x*y);
			}
		}
	}

	return texture;
}

void hsMaterialConverter::IAppendWetLayer(plMaxNode* node, hsGMaterial* mat)
{
	// Find the "wet" environment map
	int i;
	for( i = 0; i < 24; i++ )
	{
		MtlBase* mtl = fInterface->GetMtlSlot(i);
		if( mtl && (mtl->GetName() == TSTR("Wet(*)")) )
		{
			if( mtl->SuperClassID() != MATERIAL_CLASS_ID )
				continue;

			hsTArray<hsGMaterial*> matList;
			if( !GetMaterialArray((Mtl*)mtl, node, matList, 0) )
				return; // oh well, thanks for playing...

			// Okay, got one (at least one, hopefully just one, but this is trash code anyway, right?
			plLayerInterface* envLay = matList[0]->GetLayer(0);

			// Append it to the material
			hsgResMgr::ResMgr()->AddViaNotify(envLay->GetKey(), 
					new plMatRefMsg(mat->GetKey(), plRefMsg::kOnCreate, -1, plMatRefMsg::kLayer), plRefFlags::kActiveRef);

			break;
		}
	}
	// Badly set up here
	if( i == 24 )
		return;

	// Now throw on the FunkyUp map. Wait here while I pull some parameters out of my

	plLayer* prevLay = plLayer::ConvertNoRef(mat->GetLayer(mat->GetNumLayers()-1)->BottomOfStack());
	hsAssert(prevLay, "Lost our base layer");
	if( prevLay )
		prevLay->SetMiscFlags(prevLay->GetMiscFlags() | hsGMatState::kMiscBindNext | hsGMatState::kMiscRestartPassHere);

	hsMatrix44 uvwXfm;
	uvwXfm.Reset();
	uvwXfm.fMap[0][0] = uvwXfm.fMap[1][1] = uvwXfm.fMap[2][2] = 0;
	uvwXfm.NotIdentity();

	uvwXfm.fMap[0][3] = 1.f;

	float op = 0.8f;
	float tr = 0.5f;
	uvwXfm.fMap[1][2] = -1.f / (tr - op);
	uvwXfm.fMap[1][3] = uvwXfm.fMap[1][2] * -tr;

	char name[512];
	sprintf(name, "%s_funkRamp", prevLay->GetKey()->GetName());

	plLayer* layer = nil;
	plKey key = node->FindPageKey( plLayer::Index(), name );
	if( key )
		layer = plLayer::ConvertNoRef(key->GetObjectPtr());
	if( !layer )
	{
		layer = TRACKED_NEW plLayer;
		layer->InitToDefault();

		hsgResMgr::ResMgr()->NewKey(name, layer, node->GetLocation());

		plBitmap* funkRamp = IGetFunkyRamp(node, kFunkyUp);
		hsgResMgr::ResMgr()->AddViaNotify(funkRamp->GetKey(), TRACKED_NEW plLayRefMsg(layer->GetKey(), plRefMsg::kOnCreate, 0, plLayRefMsg::kTexture), plRefFlags::kActiveRef);
	}

	layer->SetAmbientColor(hsColorRGBA().Set(1.f, 1.f, 1.f, 1.f));
	layer->SetPreshadeColor(hsColorRGBA().Set(0, 0, 0, 1.f));
	layer->SetRuntimeColor(hsColorRGBA().Set(0, 0, 0, 1.f));

	layer->SetZFlags(hsGMatState::kZNoZWrite);
	UInt32 blendFlags = hsGMatState::kBlendAlpha | hsGMatState::kBlendNoTexColor | hsGMatState::kBlendAlphaMult;
	layer->SetBlendFlags(blendFlags);
	layer->SetClampFlags(hsGMatState::kClampTexture);

	layer->SetTransform(uvwXfm);

	layer->SetUVWSrc(plLayerInterface::kUVWNormal);
	layer->SetMiscFlags(layer->GetMiscFlags() | hsGMatState::kMiscOrthoProjection);

	hsgResMgr::ResMgr()->AddViaNotify(layer->GetKey(), TRACKED_NEW plMatRefMsg(mat->GetKey(), plRefMsg::kOnCreate, 
 																       -1, plMatRefMsg::kLayer), plRefFlags::kActiveRef);

}

hsBool hsMaterialConverter::HasVisDists(plMaxNode* node, int iSubMtl, hsScalar& minDist, hsScalar& maxDist)
{
	const char* dbgNodeName = node->GetName();
	const char* dbgMatName = node->GetMtl() ? node->GetMtl()->GetName() : "Dunno";

	if( node->HasFade() )
	{
		const hsScalar kMaxMaxDist = 1.e10f;
		Box3 fade = node->GetFade();
		minDist = maxDist = 0;
		if( fade.Min()[2] < 0 )
		{
			minDist = fade.Min()[0];
			maxDist = kMaxMaxDist;
		}

		if( fade.Max()[2] > 0 )
			maxDist = fade.Max()[0];

		return maxDist > minDist;
	}

	Mtl* mtl = node->GetMtl();
	if( IsMultiMat(mtl) )
	{
		if( iSubMtl >= mtl->NumSubMtls() )
			iSubMtl = 0;
		mtl = mtl->GetSubMtl(iSubMtl);
	}

	return HasVisDists(node, mtl, minDist, maxDist);
}

hsBool hsMaterialConverter::HasVisDists(plMaxNode* node, Mtl* mtl, hsScalar& minDist, hsScalar& maxDist)
{
	const char* dbgNodeName = node->GetName();
	const char* dbgMatName = node->GetMtl() ? node->GetMtl()->GetName() : "Dunno";

	if( node->HasFade() )
	{
		Box3 fade = node->GetFade();
		if( fade.Min()[2] < 0 )
			minDist = fade.Min()[0];
		else
			minDist = 0;

		if( fade.Max()[2] > 0 )
			maxDist = fade.Max()[0];
		else
			maxDist = 0;
		return maxDist > minDist;
	}

	minDist = -1.f;
	maxDist = 1.e33f;

	if( IsHsMaxMat(mtl) )
	{
		hsBool baseFunky = false;
		hsBool topFunky = true;
		plPassMtl* passMtl = (plPassMtl*)mtl;
		hsScalar tr0, op0, op1, tr1;
		UInt32 funkyType = IGetOpacityRanges(node, mtl->GetSubTexmap(0), tr0, op0, op1, tr1);

		if( kFunkyDistance == (funkyType & kFunkyMask) )
		{
			if( tr0 < op0 )
			{
				minDist = tr0;
				baseFunky = true;
			}
			if( tr1 > op1 )
			{
				maxDist = tr1;
				baseFunky = true;
			}
		}

		if( passMtl->GetTopLayerOn() )
		{
			topFunky = false;
			funkyType = IGetOpacityRanges(node, mtl->GetSubTexmap(1), tr0, op0, op1, tr1);
			if( kFunkyDistance == (funkyType & kFunkyMask) )
			{
				if( tr0 < op0 )
				{
					if( minDist > tr0 )
						minDist = tr0;
					topFunky = true;
				}
				if( tr1 > op1 )
				{
					if( maxDist < tr1 )
						maxDist = tr1;
					topFunky = true;
				}
			}
		}
		return baseFunky && topFunky;
	}

	if( IsMultipassMat(mtl) )
	{
		minDist = 1.e33f;
		maxDist = -1.f;
		int i;
		for( i = 0; i < mtl->NumSubMtls(); i++ )
		{
			hsScalar minD, maxD;
			if( !HasVisDists(node, mtl->GetSubMtl(i), minD, maxD) )
				return false;

			if( minDist > minD )
				minDist = minD;
			if( maxDist < maxD )
				maxDist = maxD;
		}
		return true;
	}

	return false;
}


hsBool hsMaterialConverter::IsBumpLayer(Texmap* texMap)
{
	if( texMap 
		&& (texMap->ClassID() == LAYER_TEX_CLASS_ID) 
		&& !strncmp(texMap->GetName(), kSecretBumpSign, strlen(kSecretBumpSign)) )
	{
		return true;
	}
	return false;
}

hsBool hsMaterialConverter::IsBumpMtl(Mtl* mtl)
{
	if( mtl == nil )
		return false;		// Well, gee, I guess it can't be a bumpmap then...

	const char* dbgMtlName = mtl->GetName();

	if( mtl->ClassID() == BUMP_MTL_CLASS_ID )
		return true;

	return false;
}

hsBool hsMaterialConverter::HasBumpLayer(plMaxNode* node, Mtl* mtl)
{
	if( !mtl )
		return false;

	// Multi-sub or a multi-pass, recurse on down
	if( IsMultiMat(mtl) || IsMultipassMat(mtl) )
	{
		int i;
		for( i = 0; i < mtl->NumSubMtls(); i++ )
		{
			if( HasBumpLayer(node, mtl->GetSubMtl(i)) )
				return true;
		}
		return false;
	}

	// Composite, screw it, just say no!
	if( IsCompositeMat(mtl) )
		return false;

	// Particle? Give me a break.
	if( IsParticleMat(mtl) )
		return false;

#ifdef BUMP_EXCLUDE_DECAL
	// Decal? Maybe someday, but not today.
	if( IsDecalMat(mtl) )
		return false;
#endif // BUMP_EXCLUDE_DECAL

	// A closer look at the material.
	if( IsBumpMtl(mtl) )
		return true;

	if( IsHsMaxMat(mtl) || IsDecalMat(mtl) )
	{
		plPassMtlBase	*passMtl = (plPassMtlBase *)mtl;
#ifdef BUMP_EXCLUDE_MULTILAYER
		if( passMtl->GetTopLayerOn() )
			return false;
#else // BUMP_EXCLUDE_MULTILAYER
		if( passMtl->GetTopLayerOn() && IsBumpLayer(mtl->GetSubTexmap(1)) )
			return true;
#endif // BUMP_EXCLUDE_MULTILAYER
		
		if( IsBumpLayer(mtl->GetSubTexmap(0)) )
			return true;
	}
	return false;
}

BitmapTex* hsMaterialConverter::GetBumpLayer(plMaxNode* node, Mtl* mtl)
{
	hsAssert(!IsMultiMat(mtl), "Material passed in here should be per face");

	if( IsMultipassMat(mtl) )
	{
		int i;
		for( i = 0; i < mtl->NumSubMtls(); i++ )
		{
			BitmapTex* retVal = GetBumpLayer(node, mtl->GetSubMtl(i));
			if( retVal )
				return retVal;
		}
		return nil;
	}
	if( IsBumpMtl(mtl) )
	{
		Texmap* texMap = mtl->GetSubTexmap(0);

		if( texMap && (texMap->ClassID() == LAYER_TEX_CLASS_ID) )
		{
			return (BitmapTex*)texMap;
		}
		return nil;
	}
	if( HasBumpLayer(node, mtl) )
	{
		Texmap* texMap = nil;

#ifndef BUMP_EXCLUDE_MULTILAYER
		// Safe cast, because only PassMtl (and derivatives) are BumpMtls.
		plPassMtl* passMtl = (plPassMtl*)mtl;

		if( passMtl->GetTopLayerOn() && IsBumpLayer(mtl->GetSubTexmap(1)) )
			texMap = mtl->GetSubTexmap(1);
#endif // BUMP_EXCLUDE_MULTILAYER

		if( !texMap )
			texMap = mtl->GetSubTexmap(0);

		if( texMap && (texMap->ClassID() == LAYER_TEX_CLASS_ID) )
		{
			return (BitmapTex*)texMap;
		}
	}
	return nil;
}

plMipmap *hsMaterialConverter::IGetBumpLutTexture(plMaxNode *node)
{
	const char* texName = "BumpLutTexture";

//#define FUNKYBUMP
#ifndef FUNKYBUMP
	const int kLUTWidth = 16;
	const int kLUTHeight = 16;

	// NOTE: CreateBlankMipmap might return an old mipmap if it was already created, so in those
	// cases we wouldn't really need to re-write the texture. However, there's no harm in doing so,
	// and since we're close to Alpha, I don't want to shake up the code any more than absolutely 
	// necessary. -mcn
	plMipmap *texture = plBitmapCreator::Instance().CreateBlankMipmap( kLUTWidth, kLUTHeight, plMipmap::kARGB32Config, 1, texName, node->GetLocation() );

	// THIS IS UNTESTED FOR ANY HEIGHT OTHER THAN 16. BEWARE THE DREAD OFF-BY-ONE!!!
	int delH = (kLUTHeight-1) / 5;
	int startH = delH / 2 + 1;
	int doneH = 0;

	// set the color data
	UInt32* pix = (UInt32*)texture->GetImage();
	int i;

	// Red ramps, one with G,B = 0,0, one with G,B = 127,127
	for( i = 0; i < startH; i++ )
	{
		int j;
		for( j = 0; j < kLUTWidth; j++ )
		{
			float x = float(j) / (kLUTWidth-1);
			*pix++ = MakeUInt32Color(x, 0, 0, 1.f);
		}
	}
	doneH = i;
	for( i = i; i < doneH + delH; i++ )
	{
		int j;
		for( j = 0; j < kLUTWidth; j++ )
		{
			float x = float(j) / (kLUTWidth-1);
			*pix++ = MakeUInt32Color(x, 0.5f, 0.5f, 1.f);
		}
	}
	doneH = i;

	// Green ramps, one with R,B = 0,0, one with R,B = 127,127
	for( i = i; i < doneH + delH; i++ )
	{
		int j;
		for( j = 0; j < kLUTWidth; j++ )
		{
			float x = float(j) / (kLUTWidth-1);
			*pix++ = MakeUInt32Color(0, x, 0, 1.f);
		}
	}
	doneH = i;
	for( i = i; i < doneH + delH; i++ )
	{
		int j;
		for( j = 0; j < kLUTWidth; j++ )
		{
			float x = float(j) / (kLUTWidth-1);
			*pix++ = MakeUInt32Color(0.5f, x, 0.5f, 1.f);
		}
	}
	doneH = i;

	// Blue ramps, one with R,G = 0,0, one with R,G = 127,127
	for( i = i; i < doneH + delH; i++ )
	{
		int j;
		for( j = 0; j < kLUTWidth; j++ )
		{
			float x = float(j) / (kLUTWidth-1);
			*pix++ = MakeUInt32Color(0, 0, x, 1.f);
		}
	}
	for( i = i; i < kLUTHeight; i++ )
	{
		int j;
		for( j = 0; j < kLUTWidth; j++ )
		{
			float x = float(j) / (kLUTWidth-1);
//				*pix++ = MakeUInt32Color(0.25f, 0.25f, x, 1.f);
			*pix++ = MakeUInt32Color(0.5f, 0.5f, x, 1.f);
		}
	}
#else // FUNKYBUMP
	const int kLUTWidth = 16;
	const int kLUTHeight = 16;

	// NOTE: CreateBlankMipmap might return an old mipmap if it was already created, so in those
	// cases we wouldn't really need to re-write the texture. However, there's no harm in doing so,
	// and since we're close to Alpha, I don't want to shake up the code any more than absolutely 
	// necessary. -mcn
	plMipmap *texture = plBitmapCreator::CreateBlankMipmap( kLUTWidth, kLUTHeight, plMipmap::kARGB32Config, 1, texName, node->GetLocation() );

	// THIS IS UNTESTED FOR ANY HEIGHT OTHER THAN 16. BEWARE THE DREAD OFF-BY-ONE!!!
	int delH = (kLUTHeight-1) / 5;
	int startH = delH / 2 + 1;
	int doneH = 0;

	// set the color data
	UInt32* pix = (UInt32*)texture->GetImage();
	int i;

	const float kWScale = 1.f;
	// Red ramps, one with G,B = 0,0, one with G,B = 127,127
	for( i = 0; i < startH; i++ )
	{
		int j;
		for( j = 0; j < kLUTWidth; j++ )
		{
			float x = float(j) / (kLUTWidth-1);
			*pix++ = MakeUInt32Color(x, 0, 0, 1.f);
		}
	}
	doneH = i;
	for( i = i; i < doneH + delH; i++ )
	{
		int j;
		for( j = 0; j < kLUTWidth; j++ )
		{
			float x = float(j) / (kLUTWidth-1);
			if( x > 0.5f )
				*pix++ = MakeUInt32Color(x, 0.5f, (x-0.5f) * kWScale + 0.5f, 1.f);
			else
				*pix++ = MakeUInt32Color(x, 0.5f, (1.f - x - 0.5f) * kWScale + 0.5f, 1.f);
		}
	}
	doneH = i;

	// Green ramps, one with R,B = 0,0, one with R,B = 127,127
	for( i = i; i < doneH + delH; i++ )
	{
		int j;
		for( j = 0; j < kLUTWidth; j++ )
		{
			float x = float(j) / (kLUTWidth-1);
			*pix++ = MakeUInt32Color(0, x, 0, 1.f);
		}
	}
	doneH = i;
	for( i = i; i < doneH + delH; i++ )
	{
		int j;
		for( j = 0; j < kLUTWidth; j++ )
		{
			float x = float(j) / (kLUTWidth-1);
			if( x > 0.5f )
				*pix++ = MakeUInt32Color(0.5f, x, (x-0.5f) * kWScale + 0.5f, 1.f);
			else
				*pix++ = MakeUInt32Color(0.5f, x, (1.f - x - 0.5f) * kWScale + 0.5f, 1.f);
		}
	}
	doneH = i;

	// Blue ramps, one with R,G = 0,0, one with R,G = 127,127
	for( i = i; i < doneH + delH; i++ )
	{
		int j;
		for( j = 0; j < kLUTWidth; j++ )
		{
			float x = float(j) / (kLUTWidth-1);
			*pix++ = MakeUInt32Color(0, 0, x, 1.f);
		}
	}
	for( i = i; i < kLUTHeight; i++ )
	{
		int j;
		for( j = 0; j < kLUTWidth; j++ )
		{
			float x = float(j) / (kLUTWidth-1);
			*pix++ = MakeUInt32Color(0.25f, 0.25f, x, 1.f);
		}
	}
#endif // FUNKYBUMP

	return texture;
}

plLayer* hsMaterialConverter::IMakeBumpLayer(plMaxNode* node, const char* nameBase, hsGMaterial* mat, UInt32 miscFlag)
{
	char name[256];
	switch( miscFlag & hsGMatState::kMiscBumpChans )
	{
	case hsGMatState::kMiscBumpDu:
		sprintf(name, "%s_DU_BumpLut", nameBase);
		break;
	case hsGMatState::kMiscBumpDv:
		sprintf(name, "%s_DV_BumpLut", nameBase);
		break;
	case hsGMatState::kMiscBumpDw:
		sprintf(name, "%s_DW_BumpLut", nameBase);
		break;
	default:
		hsAssert(false, "Bogus flag input to MakeBumpLayer");
		return nil;
	}

	plMipmap* bumpLutTexture = IGetBumpLutTexture(node);

	plLayer* layer = TRACKED_NEW plLayer;
	layer->InitToDefault();
	hsgResMgr::ResMgr()->NewKey(name, layer, node->GetLocation());
	hsgResMgr::ResMgr()->AddViaNotify(bumpLutTexture->GetKey(), TRACKED_NEW plLayRefMsg(layer->GetKey(), plRefMsg::kOnCreate, 0, plLayRefMsg::kTexture), plRefFlags::kActiveRef);

	layer->SetAmbientColor(hsColorRGBA().Set(1.f, 1.f, 1.f, 1.f));
	layer->SetPreshadeColor(hsColorRGBA().Set(0, 0, 0, 1.f));
	layer->SetRuntimeColor(hsColorRGBA().Set(0, 0, 0, 1.f));

	layer->SetZFlags(hsGMatState::kZNoZWrite);
	UInt32 blendFlags = miscFlag & hsGMatState::kMiscBumpDu ? hsGMatState::kBlendMADD : hsGMatState::kBlendAdd;
	layer->SetBlendFlags(blendFlags);
	layer->SetClampFlags(hsGMatState::kClampTexture);
	layer->SetMiscFlags(miscFlag);
	layer->SetMiscFlags(layer->GetMiscFlags() | hsGMatState::kMiscBindNext);

	if( miscFlag & hsGMatState::kMiscBumpDu )
		layer->SetMiscFlags(layer->GetMiscFlags() | hsGMatState::kMiscRestartPassHere);

	int i;
	int uvChan = -1;

	// Find our UVW channel. If there's another layer wanting to use the same kind of
	// channel, just grab that. Otherwise we need to reserve one ourselves.
	for( i = 0; i < mat->GetNumLayers(); i++ )
	{
		if( (mat->GetLayer(i)->GetMiscFlags() & hsGMatState::kMiscBumpChans) == (miscFlag & hsGMatState::kMiscBumpChans) )
			uvChan = mat->GetLayer(i)->GetUVWSrc();
	}

	if( uvChan < 0 )
	{
		uvChan = 0;
		for( i = 0; i < mat->GetNumLayers(); i++ )
		{
			if( uvChan <= int(mat->GetLayer(i)->GetUVWSrc() & plLayerInterface::kUVWIdxMask) )
				uvChan = int(mat->GetLayer(i)->GetUVWSrc() & plLayerInterface::kUVWIdxMask) + 1;
		}
		// Lightmap layers haven't been created and inserted yet, but they have reserved their
		// UVW channel. Just don't take it.
		if( node->IsLegalDecal() )
			node = (plMaxNode*)node->GetParentNode();
		if( node->GetLightMapComponent() )
		{
			if( uvChan == node->GetLightMapComponent()->GetUVWSrc() )
				uvChan++;
		}
		if( miscFlag & hsGMatState::kMiscBumpDv )
			uvChan++;
		if( miscFlag & hsGMatState::kMiscBumpDw )
			uvChan |= plLayerInterface::kUVWNormal;
	}

	layer->SetUVWSrc(uvChan);

	return layer;
}

void hsMaterialConverter::IInsertBumpLayers(plMaxNode* node, hsGMaterial* mat, int bumpLayerIdx)
{
	// First let's monkey with the incoming layer, just to confuse things.
	// This doesn't actually do anything useful, it's just to spite anyone trying to read this code.
	hsBool isAdd = 0 != (mat->GetLayer(bumpLayerIdx)->GetBlendFlags() & hsGMatState::kBlendAdd);
	plLayer* bumpLay = plLayer::ConvertNoRef(mat->GetLayer(bumpLayerIdx)->BottomOfStack());
	if( bumpLay )
		bumpLay->SetBlendFlags(
						(bumpLay->GetBlendFlags() & ~hsGMatState::kBlendMask)
						| hsGMatState::kBlendDot3);

	const char* name = mat->GetLayer(bumpLayerIdx)->GetKey()->GetName();

	plLayer* layerDu = IMakeBumpLayer(node, name, mat, hsGMatState::kMiscBumpDu);
	plLayer* layerDv = IMakeBumpLayer(node, name, mat, hsGMatState::kMiscBumpDv);
	plLayer* layerDw = IMakeBumpLayer(node, name, mat, hsGMatState::kMiscBumpDw);

	if( isAdd )
		layerDu->SetBlendFlags((layerDu->GetBlendFlags() & ~hsGMatState::kBlendMask) | hsGMatState::kBlendAdd);

	// Insert it in the right spot.
	hsgResMgr::ResMgr()->AddViaNotify(layerDv->GetKey(), TRACKED_NEW plMatRefMsg(mat->GetKey(), plRefMsg::kOnCreate, 
 																       bumpLayerIdx, plMatRefMsg::kLayer | plMatRefMsg::kInsert), plRefFlags::kActiveRef);
	hsgResMgr::ResMgr()->AddViaNotify(layerDw->GetKey(), TRACKED_NEW plMatRefMsg(mat->GetKey(), plRefMsg::kOnCreate, 
 																       bumpLayerIdx, plMatRefMsg::kLayer | plMatRefMsg::kInsert), plRefFlags::kActiveRef);
	hsgResMgr::ResMgr()->AddViaNotify(layerDu->GetKey(), TRACKED_NEW plMatRefMsg(mat->GetKey(), plRefMsg::kOnCreate, 
 																       bumpLayerIdx, plMatRefMsg::kLayer | plMatRefMsg::kInsert), plRefFlags::kActiveRef);
}

void hsMaterialConverter::IInsertBumpLayers(plMaxNode* node, hsGMaterial* mat)
{
	int bumpLayerIdx = -1;

	// Look for a bump layer. If there aren't any, there's nothing to do, just return.
	// Currently only look for the first (and hopefully only) bump layer. 
	// BUMPTODO - Either correctly handle multiple bump layers (stupid) or detect and
	// slap the production wrist for trying it.
	// Okay, decided there actually are times it kind of makes sense to have multiple bump
	// layers (hint: what's wet and wavy).
	int i;
	for( i = 0; i < mat->GetNumLayers(); i++ )
	{
		if( mat->GetLayer(i)->GetMiscFlags() & hsGMatState::kMiscBumpLayer )
		{
			IInsertBumpLayers(node, mat, i);
			// Just inserted 3 layers before this one, adjust "i" to still be
			// pointed at this layer.
			i += 3;
		}
	}

}


//
//
//
Texmap *hsMaterialConverter::GetUVChannelBase(plMaxNode *node, Mtl* mtl, int which)
{
	hsGuardBegin("hsMaterialConverter::GetUVChannelBase");

	int i;

	// Forcing no flatten to support multiple uv channels
	return nil;

	// Forcing no flatten until flattening with MultiMat resolved.
	if( !GetBaseMtl(node->GetMtl())) // || MatWrite::IsMultiMat(node->GetMtl()) )
		return nil;

	if ( ForceNoUvsFlatten(node) )
		return nil;

	if ( !node )
		return nil;

	if ( !mtl )
		return nil;

	for (i = 0; i < mtl->NumSubTexmaps(); i++) {
		if (!((StdMat *)mtl)->MapEnabled(i))
			continue;

		Texmap *texMap = mtl->GetSubTexmap(i);

		if (!texMap)
			continue;
		
		if ( !(texMap->Requirements(-1) & MTLREQ_UV) &&
			!(texMap->Requirements(-1) & MTLREQ_UV2) )
			continue;

		if (which==0) {
			if (texMap->GetUVWSource() != UVWSRC_EXPLICIT)
				continue;
		} else {
			if (texMap->GetUVWSource() != UVWSRC_EXPLICIT2)
				continue;
		}
		
		if (ITextureTransformIsAnimated(texMap))
			continue;
		
		return (texMap);
	}

	return (0);
	hsGuardEnd; 
}

hsBool hsMaterialConverter::ClearDoneMaterials(plMaxNode* node)
{
	hsGuardBegin("hsMaterialConverter::ClearDoneMaterials");

	Mtl *mtl = GetBaseMtl(node);
	int isMultiMat = IsMultiMat(mtl);
	
	if (isMultiMat)
	{
		hsBool retVal = false;
		Int32 i;
		for (i = 0; i < mtl->NumSubMtls(); i++)
		{
			retVal = retVal || IClearDoneMaterial(mtl->GetSubMtl(i), node);
		}

		return (retVal);
	}
	else
	{
		return (IClearDoneMaterial(mtl, node));
	}

	hsGuardEnd; 
}

hsBool hsMaterialConverter::IClearDoneMaterial(Mtl* mtl, plMaxNode* node)
{
	hsGuardBegin("hsMaterialConverter::IClearDoneMaterial");

	hsBool doneSomething = false;
	if (fLastMaterial.fMaxMaterial == mtl)
	{
		fLastMaterial.fHsMaterial = nil;
		fLastMaterial.fMaxMaterial = nil;
		fLastMaterial.fMaxMaterial = nil;
		fLastMaterial.fSubMultiMat = false;
		fLastMaterial.fOwnedCopy = false;
	}

	Int32 i;
	for (i = fDoneMaterials.Count() - 1; i >= 0; --i)
	{
		if (fDoneMaterials[i].fMaxMaterial == mtl)
		{
			fDoneMaterials[i].fHsMaterial->GetKey()->UnRefObject();
			fDoneMaterials.Remove(i);
			doneSomething = true;
		}
	}

	return (doneSomething);
	hsGuardEnd; 
}

#define VIEW_UP 0
#define VIEW_DN 1
#define VIEW_LF 2
#define VIEW_RT 3
#define VIEW_FR 4
#define VIEW_BK 5


static BMM_Color_64 green64 = {0,(1<<16)-1,0,(1<<16)-1};

BMM_Color_64 hsMaterialConverter::ICubeSample(Bitmap *bitmap[6], double phi, double theta) 
{
	hsGuardBegin("hsMaterialConverter::ICubeSample");

	theta = fmod(theta, (double)TWOPI);
	if( theta < 0 )theta += TWOPI;
	if( phi < 0 )phi = 0;
	else if( phi > PI )phi = PI;

	Bitmap *map = nil;

	double sinPhi = sin(phi);
	double cosPhi = cos(phi);
	double sinThe = sin(theta);
	double cosThe = cos(theta);
	const double sqrt2 = sqrt(2.0);
	const double oo_sqrt2 = 1.0 / sqrt2;
	const double oo_sqrt3 = 1.0 / sqrt(3.0);
	double sinPhiSinThe = sinPhi * sinThe;
	double sinPhiCosThe = sinPhi * cosThe;

	double x, y, z;
	double xMap, yMap;

	x = sinPhiSinThe;
	y = sinPhiCosThe;
	z = cosPhi;
	if( (z*z > x*x)&&(z*z > y*y) )
	{
		if( z > 0 )
		{
			map = bitmap[VIEW_UP];
			xMap = -x / z;
			yMap = -y / z;
		}
		else
		{
			map = bitmap[VIEW_DN];
			xMap = x / z;
			yMap = -y / z;
		}
	}
	else
	{
		if( (theta <= (hsScalarPI / 2.0 - hsScalarPI/4.0))
			||(theta >= (hsScalarPI * 2.0 - hsScalarPI/4.0)) )
		{
			map = bitmap[VIEW_FR];
			xMap = x / y;
			yMap = -z / y;
		}
		else
		if( theta <= (hsScalarPI - hsScalarPI/4.0) )
		{
			map = bitmap[VIEW_LF];
			xMap = -y / x;
			yMap = -z / x;
		}
		else
		if( theta <= (hsScalarPI * 3.0/2.0 - hsScalarPI/4.0) )
		{
			map = bitmap[VIEW_BK];
			xMap = x / y;
			yMap = z / y;
		}
		else
		{
			map = bitmap[VIEW_RT];
			xMap = -y / x;
			yMap = z / x;
		}
	}
	xMap += 1.0;
	yMap += 1.0;
	xMap *= 0.5;
	yMap *= 0.5;
	int iMap, jMap;
	iMap = (int)(xMap * (map->Width()-1));
	jMap = (int)(yMap * (map->Height()-1));

	fErrorMsg->Set(!map, "CubeSample", "Bad fallthrough in spherefromcube").Check();
	BMM_Color_64 c;
	map->GetLinearPixels(iMap,jMap,1,&c);
	return c;

	hsGuardEnd; 
}

void hsMaterialConverter::IBuildSphereMap(Bitmap *bitmap[6], Bitmap *bm) 
{
	hsGuardBegin("hsMaterialConverter::IBuildSphereMap");

	int i, j;
	double delPhi = PI / bm->Height();
	double delThe = TWOPI / bm->Width();
	PixelBuf l64(bm->Width());
	BMM_Color_64 *pb=l64.Ptr();
	for( j = 0; j < bm->Height(); j++ )
	{
		for( i = 0; i < bm->Width(); i++ )
		{
			double phi, theta; // phi is up/down

			phi = (0.5 + j) * delPhi;
			theta = PI - (0.5 + i) * delThe;

			pb[i] = ICubeSample(bitmap, phi, theta);
		}
		bm->PutPixels(0,j, bm->Width(), pb);
	}

	hsGuardEnd;
}

hsBool hsMaterialConverter::ITextureTransformIsAnimated(Texmap *texmap)
{
	hsGuardBegin("hsMaterialConverter::IProcessAnimMaterial");

	if( !texmap )
		return false;

#if 0
	StdUVGen *uvGen = ((BitmapTex *)texmap)->GetUVGen();
	if( IsAnimatedByName(uvGen, TSTR("U Offset")) )
		return true;
	if( IsAnimatedByName(uvGen, TSTR("V Offset")) )
		return true;
	if( IsAnimatedByName(uvGen, TSTR("U Tiling")) )
		return true;
	if( IsAnimatedByName(uvGen, TSTR("V Tiling")) )
		return true;
	if( IsAnimatedByName(uvGen, TSTR("Angle")) )
		return true;

	if( IsAnimatedByName(uvGen, TSTR("U Angle")) )
		return true;
	if( IsAnimatedByName(uvGen, TSTR("V Angle")) )
		return true;
	if( IsAnimatedByName(uvGen, TSTR("W Angle")) )
		return true;

	return false;
#else
	CStr className;
	texmap->GetClassName(className);
	if( strcmp(className,"Bitmap") && strcmp(className,"Plasma Layer") && strcmp(className,"Plasma Layer Dbg."))
		return false;
	return (IHasAnimatedControllers(((BitmapTex *)texmap)->GetUVGen()));
#endif
	hsGuardEnd; 
}

hsBool hsMaterialConverter::IHasAnimatedControllers(Animatable* anim)
{
	hsGuardBegin("hsMaterialConverter::IHasAnimatedControllers");

	if( anim )
	{
		Control* ctl = GetControlInterface(anim);
		if (hsControlConverter::Instance().HasKeyTimes(ctl))
			return true;
		
		int nSub = anim->NumSubs();
		int i;
		for (i = 0; i < nSub; i++)
		{
			if (anim->SubAnim(i)==nil)
				continue;

			if( IHasAnimatedControllers(anim->SubAnim(i)) )
				return true;
		}
	}

	return false;
	hsGuardEnd; 
}


hsBool hsMaterialConverter::IIsAnimatedTexmap(Texmap* texmap)
{
	hsGuardBegin("hsMaterialConverter::IIsAnimatedTexmap");

	if (!texmap)
		return false;

	Control *ctl=nil;
	if (hsControlConverter::Instance().GetControllerByName(texmap, TSTR("Ambient"), ctl)) 
		return true;
	if (hsControlConverter::Instance().GetControllerByName(texmap, TSTR("Diffuse"), ctl)) 
		return true;
	if (hsControlConverter::Instance().GetControllerByName(texmap, TSTR("Color"), ctl)) 
		return true;
	if (hsControlConverter::Instance().GetControllerByName(texmap, TSTR("Opacity"), ctl)) 
		return true;

	if (HasAnimatedTextures(texmap) || IsAVILayer(texmap) || IsBinkLayer(texmap)|| IsQTLayer(texmap) || ITextureTransformIsAnimated(texmap))
		return true;

	return false;
	hsGuardEnd; 
}

//
// returns true if this material is animated
//
hsBool hsMaterialConverter::IsAnimatedMaterial(Mtl* mtl)
{
	hsGuardBegin("hsMaterialConverter::IsAnimatedMaterial");

	if (!mtl)
		return false;

	if (IsMultiMat(mtl))
	{
		int iMtl;
		for (iMtl = 0; iMtl < mtl->NumSubMtls(); iMtl++)
		{
			if (IsAnimatedMaterial(mtl->GetSubMtl(iMtl)))
				return true;
		}
		return false;
	}
	else
//	if (IsStdMat(mtl) || mtl->ClassID() == Class_ID(DMTL_CLASS_ID, 0)) 
	{ 
		// It is std material.	does not have any submaterials
		StdMat* std = (StdMat *)mtl;
		int i;
		for(i=0;i<std->NumSubTexmaps();i++)
		{
			if (IIsAnimatedTexmap(std->GetSubTexmap(i)))
				return true;
		}
		return false;
	}

	hsGuardEnd; 
}

void hsMaterialConverter::GetUsedMaterials(plMaxNode* node, hsBitVector& used)
{
	Mtl* mtl = GetBaseMtl(node);

	used.Clear();
	if( !IsMultiMat(mtl) )
	{
		used.SetBit(0);
		return;
	}

	hsBool deleteIt = false;
	TriObject* triObj = node->GetTriObject(deleteIt);
	if( triObj )
	{
		Mesh* mesh = &(triObj->mesh);
	
		int numFaces = mesh->getNumFaces();
		int i;
		for( i = 0; i < numFaces; i++ )
		{
			Face		*face = &mesh->faces[ i ];

			used.SetBit(face->getMatID());
		}
	}
	return;
}

//// HasMaterialDiffuseOrOpacityAnimation ////////////////////////////////////
//	Returns true if the material or any of its submaterials has a diffuse
//	animation controller. Handy for those times when you need to decide on
//	a lighting model...
//	UPDATE 8.26 mcn - Now checks for opacity controllers as well

hsBool	hsMaterialConverter::HasMaterialDiffuseOrOpacityAnimation(plMaxNode* node, Mtl* mtl)
{
	hsGuardBegin( "hsMaterialConverter::HasMaterialDiffuseOrOpacityAnimation" );

	if( !mtl )
		mtl = GetBaseMtl(node);

	if( !mtl )
		return false;

	const char	*dbgName = mtl->GetName();

	// mf
	// Inserting this test here. The glaring omission was that if you had several
	// passes, each using a different material opacity, then you can't bake the
	// alpha into the vertices, so you can't go to kLiteVtxNonPreshaded (which is
	// what this function really tests to see if you can do).
	// More specifically, we can't bake material opacity into the verts if:
	//		Two or more passes use alpha blending AND have different opacity values.
	if( IsMultipassMat(mtl) )
	{
		float baseOpac = -1.f;
		Mtl* subMtl = mtl->GetSubMtl(0);
		if( subMtl->ClassID() == PASS_MTL_CLASS_ID )
		{
			plPassMtlBase* passMtl = (plPassMtlBase*)subMtl;
			if( plPassMtlBase::kBlendAlpha == passMtl->GetOutputBlend() )
				baseOpac = passMtl->GetOpacity();
		}
		int iMtl;
		for( iMtl = 1; iMtl < mtl->NumSubMtls(); iMtl++ )
		{
			if( subMtl->ClassID() == PASS_MTL_CLASS_ID )
			{
				plPassMtlBase* passMtl = (plPassMtlBase*)subMtl;
				if( (plPassMtlBase::kBlendAlpha == passMtl->GetOutputBlend())
					&&(baseOpac != passMtl->GetOpacity()) )
				{
					if( baseOpac >= 0 )
						return true;
					baseOpac = passMtl->GetOpacity();
				}
			}
		}
	}

	if( IsMultiMat( mtl ) )
	{
		hsBitVector usedSubs;
		GetUsedMaterials(node, usedSubs);

		int iMtl;
		for( iMtl = 0; iMtl < mtl->NumSubMtls(); iMtl++ )
		{
			// We have to check for nil here, because HasMaterialDif... assumes that when you pass in nil,
			// it should just grab the material on the node. In this case that's the multimat we're looping
			// through. Hellooooooo infinite loop.
			if( mtl->GetSubMtl(iMtl) && usedSubs.IsBitSet(iMtl) && HasMaterialDiffuseOrOpacityAnimation( node, mtl->GetSubMtl(iMtl) ) ) 
				return true;
		}
		return false;
	}
	else if( IsMultipassMat( mtl ) || IsCompositeMat( mtl ) )
	{
		int iMtl;
		for( iMtl = 0; iMtl < mtl->NumSubMtls(); iMtl++ )
		{
			if( mtl->GetSubMtl(iMtl) && HasMaterialDiffuseOrOpacityAnimation( node, mtl->GetSubMtl(iMtl) ) ) 
				return true;
		}
		return false;
	}
	else if ( IsParticleMat( mtl ) )
	{
		plParticleMtl *partMtl = (plParticleMtl *)mtl;
		return partMtl->GetColorController() != nil;
	}
	else if( IsHsMaxMat(mtl) || IsDecalMat(mtl) )
	{ 
		// It is std material.	does not have any submaterials
		StdMat			*std = (StdMat *)mtl;
		plPassMtlBase	*passMtl = (plPassMtlBase *)mtl;
		Control			*ctl = nil;
		int				i;

		if( passMtl->GetPreshadeColorController() != nil )
			return true;
		if( passMtl->GetRuntimeColorController() != nil )
			return true;
		if( passMtl->GetOpacityController() != nil )
			return true;

		for( i = 0; i < std->NumSubTexmaps(); i++ )
		{
			if( hsControlConverter::Instance().GetControllerByName( std->GetSubTexmap( i ), TSTR( "Diffuse" ), ctl ) ) 
				return true;
			if( hsControlConverter::Instance().GetControllerByName( std->GetSubTexmap( i ), TSTR( "Color" ), ctl ) ) 
				return true;
			if( hsControlConverter::Instance().GetControllerByName( std->GetSubTexmap( i ), TSTR( "Opacity" ), ctl ) ) 
				return true;
		}
		return false;
	}

	return false;

	hsGuardEnd; 
}

//// HasEmissiveLayer ////////////////////////////////////////////////////////
//	Returns true if the any of the layers of any of the submaterials or
//	the main material are emissive.

hsBool	hsMaterialConverter::HasEmissiveLayer(plMaxNode* node, Mtl* mtl)
{
	hsGuardBegin( "hsMaterialConverter::HasEmissiveLayer" );

	if( !mtl )
		mtl = GetBaseMtl(node);

	if( !mtl )
		return false;

	const char	*dbgName = mtl->GetName();

	if( IsMultiMat( mtl ) )
	{
		hsBitVector usedSubs;
		GetUsedMaterials(node, usedSubs);

		int iMtl;
		for( iMtl = 0; iMtl < mtl->NumSubMtls(); iMtl++ )
		{
			if( usedSubs.IsBitSet(iMtl) && HasEmissiveLayer( node, mtl->GetSubMtl(iMtl) ) ) 
				return true;
		}
		return false;
	}
	else if( IsMultipassMat( mtl ) || IsCompositeMat( mtl ) )
	{
		int iMtl;
		for( iMtl = 0; iMtl < mtl->NumSubMtls(); iMtl++ )
		{
			if( HasEmissiveLayer( node, mtl->GetSubMtl(iMtl) ) ) 
				return true;
		}
		return false;
	}
	else if ( IsParticleMat( mtl ) )
	{
		plParticleMtl *partMtl = (plParticleMtl *)mtl;
		if( partMtl->GetParamBlockByID( plParticleMtl::kRefBasic )->GetInt( plParticleMtl::kNormal ) == plParticleMtl::kEmissive )
			return true;
	}
	else if( mtl->ClassID() == PASS_MTL_CLASS_ID )
	{ 
		// It is std material.	does not have any submaterials
		plPassMtlBase	*passMtl = (plPassMtlBase *)mtl;

		if( passMtl->GetEmissive() )
			return true;
	}

	return false;

	hsGuardEnd; 
}

//
// returns true if the material onthis node is animated
//
hsBool hsMaterialConverter::HasAnimatedMaterial(plMaxNode* node)
{
	hsGuardBegin("hsMaterialConverter::HasAnimatedMaterial");

	return (node ? IsAnimatedMaterial(node->GetMtl()) : false);
	hsGuardEnd; 
}

Mtl* hsMaterialConverter::FindSubMtlByName(TSTR& name, Animatable* anim)
{
	hsGuardBegin("hsMaterialConverter::FindSubMtlByName");

	if( !anim || !IsMtl(anim) )
		return nil;

	Mtl* mtl = (Mtl*)anim;

	if( mtl->GetName() == name )
		return mtl;

	if( IsMultiMat(mtl) )
	{
		int i;
		for( i = 0; i < mtl->NumSubs(); i++ )
		{
			Mtl* retVal;
			if( retVal = FindSubMtlByName(name, mtl->SubAnim(i)) )
				return retVal;
		}
	}

	return nil;
	hsGuardEnd; 
}

Mtl* hsMaterialConverter::FindSceneMtlByName(TSTR& name)
{
	hsGuardBegin("hsMaterialConverter::FindSceneMtlByName");

	ReferenceTarget *scene = fInterface->GetScenePointer();

	// First look through the editor slots
	ReferenceTarget* mtlEdit;
	mtlEdit = scene->GetReference(0);

	int i;
	for( i = 0; i < mtlEdit->NumSubs(); i++ )
	{
		Mtl* mtl = FindSubMtlByName(name, mtlEdit->SubAnim(i));
		if( mtl )
			return (mtl);
	}


	// Now look through the rest of the scene
	MtlBaseLib& mtlLib = *(MtlBaseLib*)scene->GetReference(1);
	for( i = 0; i < mtlLib.Count(); i++ )
	{
		Mtl* mtl = FindSubMtlByName(name, mtlLib[i]);
		if( mtl )
			return (mtl);
	}

	return nil;
	hsGuardEnd; 
}

int hsMaterialConverter::GetMaterialArray(Mtl *mtl, plMaxNode* node, hsTArray<hsGMaterial*>& out, UInt32 multiIndex /* = 0 */)
{
	hsTArray<plExportMaterialData>* arGh = CreateMaterialArray(mtl, node, multiIndex);
	int i;
	for( i = 0; i < arGh->GetCount(); i++ )
	{
		out.Append(arGh->Get(i).fMaterial);
	}

	delete arGh;

	return out.GetCount();
}

static void GetMtlNodes(Mtl *mtl, INodeTab& nodes)
{
	if (!mtl)
		return;

	RefList& refs = mtl->GetRefList();
	RefListItem *item = refs.FirstItem();
	while (item)
	{
		if (item->maker->SuperClassID() == BASENODE_CLASS_ID)
		{
			INode *node = (INode*)item->maker;
			if (node->GetMtl() == mtl)
				nodes.Append(1, &node);
		}

		item = item->next;
	}
}

int hsMaterialConverter::GetMaterialArray(Mtl *mtl, hsTArray<hsGMaterial*>& out, UInt32 multiIndex /* = 0 */)
{
	INodeTab nodes;
	GetMtlNodes(mtl, nodes);

	for (int i = 0; i < nodes.Count(); i++)
	{
		hsTArray<hsGMaterial*> tempOut;
		GetMaterialArray(mtl, (plMaxNode*)nodes[i], tempOut, multiIndex);

		for (int j = 0; j < tempOut.GetCount(); j++)
		{
			if (out.Find(tempOut[j]) == out.kMissingIndex)
				out.Append(tempOut[j]);
		}
	}

	return out.GetCount();
}

// Grab all the hsGMaterials that have been created as a result of converting mtl
void hsMaterialConverter::CollectConvertedMaterials(Mtl *mtl, hsTArray<hsGMaterial *>& out)
{
	int i;
	for (i = 0; i < fDoneMaterials.GetCount(); i++)
	{
		const DoneMaterialData &dmd = fDoneMaterials.Get(i);
		if (dmd.fMaxMaterial == mtl)
			out.Append(dmd.fHsMaterial);
	}
}

plClothingItem *hsMaterialConverter::GenerateClothingItem(plClothingMtl *mtl, const plLocation &loc)
{
	char clothKeyName[256];
	plClothingItem *cloth = TRACKED_NEW plClothingItem();
	cloth->SetName(mtl->GetName());
	cloth->fSortOrder = (mtl->GetDefault() ? 0 : 1);

	char *accName = mtl->GetForcedAccessoryName();
	if (accName && strcmp(accName, ""))
		cloth->fAccessoryName = hsStrcpy(accName);
	
	Color tint1 = mtl->GetDefaultTint1();
	Color tint2 = mtl->GetDefaultTint2();
	cloth->fDefaultTint1[0] = tint1.r * 255;
	cloth->fDefaultTint1[1] = tint1.g * 255;
	cloth->fDefaultTint1[2] = tint1.b * 255;
	cloth->fDefaultTint2[0] = tint2.r * 255;
	cloth->fDefaultTint2[1] = tint2.g * 255;
	cloth->fDefaultTint2[2] = tint2.b * 255;
	
	sprintf(clothKeyName, "CItm_%s", cloth->fName);
	hsgResMgr::ResMgr()->NewKey(clothKeyName, cloth, loc);
	
	plNodeRefMsg* nodeRefMsg = TRACKED_NEW plNodeRefMsg(plKeyFinder::Instance().FindSceneNodeKey(loc), 
												plNodeRefMsg::kOnRequest, -1, plNodeRefMsg::kGeneric);
	hsgResMgr::ResMgr()->AddViaNotify(cloth->GetKey(), nodeRefMsg, plRefFlags::kActiveRef);

	mtl->InitTilesets();
	cloth->fTileset = mtl->GetTilesetIndex();
	plClothingTileset *tileset = mtl->fTilesets.Get(cloth->fTileset); 
	int i, j;
	for (i = 0; i < tileset->fElements.GetCount(); i++)
	{
		for (j = 0; j < plClothingElement::kLayerMax; j++)
		{
			UInt32 clipLevels;
			UInt32 startWidth;
			char *elementName = tileset->fElements.Get(i)->fName;
			plPlasmaMAXLayer *layer = (plPlasmaMAXLayer *)mtl->GetTexmap(i, j);
			if (layer == nil || layer->GetPBBitmap() == nil)
				continue;

			const char *texName = layer->GetPBBitmap()->bi.Name();

			for (clipLevels = 0, startWidth = layer->GetPBBitmap()->bi.Width(); 
				 startWidth > tileset->fElements.Get(i)->fWidth;
				 clipLevels++, startWidth >>= 1);

			plMipmap *tex = plMipmap::ConvertNoRef(plLayerConverter::Instance().CreateSimpleTexture(texName, loc, clipLevels));
			if (tex == nil)
			{
				if (fErrorMsg->Set(!(fWarned & kWarnedMissingClothingTexture), mtl->GetName(), 
					"Unable to create texture %s. This clothing item won't look right.", 
					texName).CheckAskOrCancel() )
				{
					fWarned |= kWarnedMissingClothingTexture;
				}
				continue;
			}
			plElementRefMsg *eMsg = TRACKED_NEW plElementRefMsg(cloth->GetKey(), plRefMsg::kOnCreate, i, -1, elementName, j);
			hsgResMgr::ResMgr()->AddViaNotify(tex->GetKey(), eMsg, plRefFlags::kActiveRef);
		}
	}
	mtl->ReleaseTilesets();
	
	plPlasmaMAXLayer *layer = (plPlasmaMAXLayer *)mtl->GetThumbnail();
	plMipmap *thumbnail = nil;
	PBBitmap *pbbm = nil;
	if (layer != nil)
	{
		char texName[ 512 ];
		if( layer->GetBitmapFileName( texName, sizeof( texName ) ) )
			thumbnail = plMipmap::ConvertNoRef(plLayerConverter::Instance().CreateSimpleTexture(texName, loc, 0, plBitmap::kForceOneMipLevel));
	}
	if (thumbnail != nil)
	{
		plGenRefMsg *msg= TRACKED_NEW plGenRefMsg(cloth->GetKey(), plRefMsg::kOnCreate, -1, -1);
		hsgResMgr::ResMgr()->AddViaNotify(thumbnail->GetKey(), msg, plRefFlags::kActiveRef); 
	}
	cloth->fDescription = hsStrcpy(mtl->GetDescription());
	cloth->fCustomText = hsStrcpy(mtl->GetCustomText());

	return cloth;
}

static int ICompareBaseLayerTexture(const hsMaterialConverter::DoneMaterialData* one, const hsMaterialConverter::DoneMaterialData* two)
{
	const plLayerInterface* oneLay = one->fHsMaterial->GetLayer(0);
	const plLayerInterface* twoLay = two->fHsMaterial->GetLayer(0);
	const plBitmap* oneTex = oneLay->GetTexture();
	const plBitmap* twoTex = twoLay->GetTexture();

	if( !oneTex && !twoTex )
		return 0;
	if( oneTex && !twoTex )
		return 1;
	if( !oneTex && twoTex )
		return -1;

	return _stricmp(oneTex->GetKey()->GetName(), twoTex->GetKey()->GetName());
}

static int IIsAnimatedLayer(const plLayerInterface* lay)
{
	return nil == plLayer::ConvertNoRef(lay);
}

static int ICompareColors(const hsColorRGBA& one, const hsColorRGBA& two)
{
	int oneR = int(one.r * 256.f);
	int oneG = int(one.g * 256.f);
	int oneB = int(one.b * 256.f);
	int twoR = int(two.r * 256.f);
	int twoG = int(two.g * 256.f);
	int twoB = int(two.b * 256.f);

	int powerOne = oneR + oneG + oneB;
	int powerTwo = twoR + twoG + twoB;

	if( powerOne < powerTwo )
		return -1;
	if( powerOne > powerTwo )
		return 1;

	if( oneR < twoR )
		return -1;
	if( oneR > twoR )
		return 1;

	if( oneG < twoG )
		return -1;
	if( oneG > twoG )
		return 1;

	if( oneB < twoB )
		return -1;
	if( oneB > twoB )
		return 1;

	return 0;
}

static int ICompareDoneLayers(const plLayerInterface* one, const plLayerInterface* two)
{
	int retVal;

	if( one == two )
		return 0;

	if( one->GetTexture() && !two->GetTexture() )
		return 1;
	if( !one->GetTexture() && two->GetTexture() )
		return -1;

	if( one->GetTexture() && two->GetTexture() )
	{
		retVal = _stricmp(one->GetTexture()->GetKey()->GetName(), two->GetTexture()->GetKey()->GetName());
		if( retVal < 0 )
			return -1;
		else if( retVal > 0 )
			return 1;
	}

	retVal =  Int32(one->GetBlendFlags()) - Int32(two->GetBlendFlags());
	if( retVal < 0 )
		return -1;
	else if( retVal > 0 )
		return 1;

	retVal =  Int32(one->GetZFlags()) - Int32(two->GetZFlags());
	if( retVal < 0 )
		return -1;
	else if( retVal > 0 )
		return 1;

	retVal =  Int32(one->GetClampFlags()) - Int32(two->GetClampFlags());
	if( retVal < 0 )
		return -1;
	else if( retVal > 0 )
		return 1;

	retVal =  Int32(one->GetMiscFlags()) - Int32(two->GetMiscFlags());
	if( retVal < 0 )
		return -1;
	else if( retVal > 0 )
		return 1;

	retVal =  Int32(one->GetShadeFlags()) - Int32(two->GetShadeFlags());
	if( retVal < 0 )
		return -1;
	else if( retVal > 0 )
		return 1;

	retVal = ICompareColors(one->GetAmbientColor(), two->GetAmbientColor());
	if( retVal < 0 )
		return -1;
	else if( retVal > 0 )
		return 1;

	retVal = ICompareColors(one->GetPreshadeColor(), two->GetPreshadeColor());
	if( retVal < 0 )
		return -1;
	else if( retVal > 0 )
		return 1;

	retVal = ICompareColors(one->GetRuntimeColor(), two->GetRuntimeColor());
	if( retVal < 0 )
		return -1;
	else if( retVal > 0 )
		return 1;

	retVal = ICompareColors(one->GetSpecularColor(), two->GetSpecularColor());
	if( retVal < 0 )
		return -1;
	else if( retVal > 0 )
		return 1;

	if( one->GetSpecularPower() < two->GetSpecularPower() )
		return -1;
	if( one->GetSpecularPower() > two->GetSpecularPower() )
		return 1;

	if( one->GetLODBias() < two->GetLODBias() )
		return -1;
	if( one->GetLODBias() > two->GetLODBias() )
		return 1;

	if( one->GetOpacity() < two->GetOpacity() )
		return -1;
	if( one->GetOpacity() > two->GetOpacity() )
		return 1;

	if( one->GetUVWSrc() < two->GetUVWSrc() )
		return -1;
	if( one->GetUVWSrc() > two->GetUVWSrc() )
		return 1;

	if( one->GetTexture() && two->GetTexture() )
	{
		if( one->GetTransform() != two->GetTransform() )
		{
			// Okay, they're not equal. Greater/Lesser doesn't make much
			// sense in this context, so we just need some arbitrary but 
			// consistent comparison. So if the transforms aren't equal,
			// the one on the layer with a larger pointer value is greater.
			if( one > two )
				return 1;
			else
				return -1;
		}
	}

	if( IIsAnimatedLayer(one) || IIsAnimatedLayer(two) )
	{
		// Same deal as transform. If either is animated, then for
		// the purposes here (whether one can replace the other), they
		// aren't interchangeable. Even if they have the same animation,
		// they need to stay independently playable.
		if( one > two )
			return 1;
		else
			return -1;
	}

	return 0;
}

static int ICompareDoneMats(const void *arg1, const void *arg2)
{
	const hsMaterialConverter::DoneMaterialData* one = *(const hsMaterialConverter::DoneMaterialData**)arg1;
	const hsMaterialConverter::DoneMaterialData* two = *(const hsMaterialConverter::DoneMaterialData**)arg2;

	hsGMaterial* oneMat = one->fHsMaterial;
	hsGMaterial* twoMat = two->fHsMaterial;

	// compare the base layers
	// First compare the textures. If those are the same, these two materials are very much alike.
	// This will (quickly) weed out 99% of the non-equivalent materials before the more expensive checks.
	plLayerInterface* oneLay = oneMat->GetLayer(0);
	plLayerInterface* twoLay = twoMat->GetLayer(0);
	int retVal = ICompareBaseLayerTexture(one, two);
	if( retVal > 0 )
		return 1;
	if( retVal < 0 )
		return -1;

	// Check for lightmap compatible-ness.
	// The case we're looking for is if:
	//		two different nodes are using the same material,
	// &&	both are lightmapped
	// &&	either 
	//			they are different lightmap components
	//		||	the lightmap component doesn't want to share.
	// If true, we want to ensure these two materials don't get combined.
	// Since this function is used for material sorting, we'll (arbitrarily)
	// return the comparison of owner node pointers as our consistent result.
	plMaxNode* oneNode = one->fNode;
	plMaxNode* twoNode = two->fNode;
	if( oneNode && twoNode && (oneNode != twoNode) )
	{
		plLightMapComponent* oneLM = oneNode->GetLightMapComponent();
		plLightMapComponent* twoLM = twoNode->GetLightMapComponent();
		if( oneLM != twoLM )
		{
			return oneNode > twoNode ? 1 : -1;
		}
		if( oneLM )
		{
			if( !oneLM->GetShared() ) // and therefore twoLM, since they're equal
			{
				return oneNode > twoNode ? 1 : -1;
			}
		}
	}

	if( oneMat == twoMat )
		return 0;

	// Now compare everything else about the base layer.
	retVal = ICompareDoneLayers(oneMat->GetLayer(0), twoMat->GetLayer(0));
	if( retVal < 0 )
		return -1;
	else if( retVal > 0 )
		return 1;

	// base layers the same, go up a layer at a time. Non-existence of a layer is < any existent layer
	int i;
	for( i = 1; i < oneMat->GetNumLayers(); i++ )
	{
		if( twoMat->GetNumLayers() <= i )
			return 1;

		retVal = ICompareDoneLayers(oneMat->GetLayer(i), twoMat->GetLayer(i));
		if( retVal < 0 )
			return -1;
		else if( retVal > 0 )
			return 1;
	}
	if( oneMat->GetNumLayers() < twoMat->GetNumLayers() )
		return -1;

	return 0;
}

void hsMaterialConverter::IPrintDoneMat(hsStream* stream, const char* prefix, DoneMaterialData* doneMat)
{
	if( doneMat->fOwnedCopy )
		stream->WriteString("Unique ");
	stream->WriteString(prefix);

	char buff[512];
	sprintf(buff, "%s\n", doneMat->fMaxMaterial ? doneMat->fMaxMaterial->GetName() : "BLANK");
	stream->WriteString(buff);

	sprintf(buff, "\t\t%d Layers\n", doneMat->fHsMaterial->GetNumLayers());
	stream->WriteString(buff);

	int i;
	for( i = 0; i < doneMat->fHsMaterial->GetNumLayers(); i++ )
	{
		const plLayerInterface* layer = doneMat->fHsMaterial->GetLayer(i);

		const char* blendMode = "error";
		switch(layer->GetBlendFlags() & hsGMatState::kBlendMask)
		{
		case hsGMatState::kBlendAlpha:
			blendMode = "Alpha";
			break;
		case hsGMatState::kBlendMult:
			blendMode = "Mult";
			break;
		case hsGMatState::kBlendAdd:
			blendMode = "Add";
			break;
		case hsGMatState::kBlendAddColorTimesAlpha:
			blendMode = "AddColorTimesAlpha";
			break;
		case hsGMatState::kBlendDetail:
			blendMode = "Detail";
			break;
		case hsGMatState::kBlendMADD:
			blendMode = "MADD";
			break;
		case hsGMatState::kBlendDot3:
			blendMode = "Dot3";
			break;
		default:
			blendMode = "Opaque";
			break;
		}

		sprintf(buff, "\t\tLayer %d [%s]\n", i, IIsAnimatedLayer(layer) ? "Animated" : "Static");
		stream->WriteString(buff);

		sprintf(buff, "\t\t\t%s [B%#08x Z%#08x C%#08x M%#08x S%08x]\n", blendMode, 
			layer->GetBlendFlags(), 
			layer->GetZFlags(),
			layer->GetClampFlags(),
			layer->GetMiscFlags(),
			layer->GetShadeFlags());
		stream->WriteString(buff);

		sprintf(buff, "\t\t\tAmbient(%f,%f,%f) Preshade(%f,%f,%f)\n",
			layer->GetAmbientColor().r,
			layer->GetAmbientColor().g,
			layer->GetAmbientColor().b,
			layer->GetPreshadeColor().r,
			layer->GetPreshadeColor().g,
			layer->GetPreshadeColor().b);
		stream->WriteString(buff);

		sprintf(buff, "\t\t\tColor(%f,%f,%f) Opacity(%f) UVWSrc(%x)\n",
			layer->GetRuntimeColor().r,
			layer->GetRuntimeColor().g,
			layer->GetRuntimeColor().b,
			layer->GetOpacity(),
			layer->GetUVWSrc());
		stream->WriteString(buff);

		sprintf(buff, "\t\t\tSpec(%f,%f,%f) Power(%f) LODBias(%f)\n",
			layer->GetSpecularColor().r,
			layer->GetSpecularColor().g,
			layer->GetSpecularColor().b,
			layer->GetSpecularPower(),
			layer->GetLODBias());
		stream->WriteString(buff);

		sprintf(buff, "\t\t\tTexture %s\n", layer->GetTexture() && layer->GetTexture()->GetKey() && layer->GetTexture()->GetKey()->GetName()
			? layer->GetTexture()->GetKey()->GetName()
			: "None");
		stream->WriteString(buff);

		if( layer->GetTransform().fFlags & hsMatrix44::kIsIdent )
		{
			sprintf(buff, "\t\t\tXForm = None\n");
			stream->WriteString(buff);
		}
		else
		{
			sprintf(buff, "\t\t\tXForm = \t{ {%f,%f,%f,%f}, \n\t\t\t\t\t{%f,%f,%f,%f}, \n\t\t\t\t\t{%f,%f,%f,%f} }\n",
				layer->GetTransform().fMap[0][0],
				layer->GetTransform().fMap[0][1],
				layer->GetTransform().fMap[0][2],
				layer->GetTransform().fMap[0][3],

				layer->GetTransform().fMap[1][0],
				layer->GetTransform().fMap[1][1],
				layer->GetTransform().fMap[1][2],
				layer->GetTransform().fMap[1][3],

				layer->GetTransform().fMap[2][0],
				layer->GetTransform().fMap[2][1],
				layer->GetTransform().fMap[2][2],
				layer->GetTransform().fMap[2][3]);
			stream->WriteString(buff);
		}
	}
}

hsBool hsMaterialConverter::IEquivalent(DoneMaterialData* one, DoneMaterialData* two)
{
	if( one->fOwnedCopy || two->fOwnedCopy )
		return false;

	return ICompareDoneMats(&one, &two) == 0;
}

void hsMaterialConverter::ISortDoneMaterials(hsTArray<DoneMaterialData*>& doneMats)
{
	doneMats.SetCount(fDoneMaterials.GetCount());
	int i;
	for( i = 0; i < fDoneMaterials.GetCount(); i++ )
		doneMats[i] = &fDoneMaterials[i];


	void* arr = doneMats.AcquireArray();
	qsort((void*)arr, doneMats.GetCount(), sizeof(DoneMaterialData*), ICompareDoneMats);
}

void hsMaterialConverter::IGenMaterialReport(const char* path)
{
	hsTArray<DoneMaterialData*> doneMats;
	ISortDoneMaterials(doneMats);

	IPrintDoneMaterials(path, doneMats);
}

void hsMaterialConverter::IPrintDoneMaterials(const char* path, hsTArray<DoneMaterialData*>& doneMats)
{
    TSTR maxFileTstr = GetCOREInterface()->GetCurFileName();
	char maxFile[256];
	hsStrncpy(maxFile, maxFileTstr, 128);
	char *dot = strrchr(maxFile, '.');
	if (dot)
		*dot = 0;

	char fileName[512];
	if( path[hsStrlen(path)-1] == '\\' )
	{
		sprintf(fileName, "%slog\\mat_%s.log", path, maxFile);
	}
	else
	{
		sprintf(fileName, "%s\\log\\mat_%s.log", path, maxFile);
	}

	hsUNIXStream stream;
	if( !stream.Open(fileName, "wt") )
	{
		// We may not have a \log folder. If that failed, try
		// putting it in the \dat folder. If that doesn't work,
		// just quietly give up.
		if( path[hsStrlen(path)-1] == '\\' )
		{
			sprintf(fileName, "%sdat\\mat_%s.log", path, maxFile);
		}
		else
		{
			sprintf(fileName, "%s\\dat\\mat_%s.log", path, maxFile);
		}
		if( !stream.Open(fileName, "wt") )
			return;
	}

	stream.WriteString(maxFile);
	stream.WriteString("\n===============================================\n===============================================\n");

	if( !doneMats.GetCount() )
	{
		char buff[256];
		sprintf(buff, "");
		stream.WriteString("No Materials Generated\n");
		stream.Close();
		return;
	}

	char pref[32];
	sprintf(pref, "%d\t", 0);
	IPrintDoneMat(&stream, pref, doneMats[0]);

	bool lastWasDup = false;
	int dupSets = 0;
	int duplicates = 0;
	int uniques = 0;
	int i;
	for( i = 1; i < doneMats.GetCount(); i++ )
	{
		if( IEquivalent(doneMats[i], doneMats[i-1]) )
		{
			if( !lastWasDup )
			{
				dupSets++;
				lastWasDup = true;
			}
			duplicates++;
			sprintf(pref, "==%d\t", i);
		}
		else if( !ICompareBaseLayerTexture(doneMats[i], doneMats[i-1]) )
		{
			sprintf(pref, "~~%d\t", i);
			lastWasDup = false;
		}
		else
		{
			sprintf(pref, "%d\t", i);
			lastWasDup = false;
		}
		if( doneMats[i]->fOwnedCopy )
			uniques++;

		IPrintDoneMat(&stream, pref, doneMats[i]);

	}
	char buff[256];
	sprintf(buff, "\n===================================================================\n");
	stream.WriteString(buff);
	sprintf(buff, "%d sets of duplicates, %d total duplicate count\n", dupSets, duplicates);
	stream.WriteString(buff);

	sprintf(buff, "System generated duplicates:\n");
	stream.WriteString(buff);

	sprintf(buff, "Gameplay forced unique - %d\n", uniques);
	stream.WriteString(buff);

	sprintf(buff, "RT:%d, UV:%d, AL:%d, FD:%d\n", dupCuzRT, dupCuzNumUV, dupCuzAlphaLayer, dupCuzFade);
	stream.WriteString(buff);

	sprintf(buff, "\nThank you, and have a lovely day.\n");
	stream.WriteString(buff);

	stream.Close();
}

hsMaterialConverter::DoneMaterialData* hsMaterialConverter::IFindDoneMaterial(DoneMaterialData& done)
{
	int i;
	for( i = 0; i < fDoneMaterials.GetCount(); i++ )
	{
		if( IEquivalent(&fDoneMaterials[i], &done) )
		{
			return &fDoneMaterials[i];
		}
	}
	return nil;
}

plMipmap *hsMaterialConverter::GetStaticColorTexture(Color c, plLocation &loc)
{
	char texName[256];
	UInt32 colorHex = MakeUInt32Color(c.r, c.g, c.b, 1.f);
	sprintf(texName, "StaticColorTex_4x4_%X", colorHex);

	int w = 4;
	int h = 4;

	plMipmap *texture = plBitmapCreator::Instance().CreateBlankMipmap(w, h, plMipmap::kARGB32Config, 1, texName, loc );

	// set the color data
	UInt32* pix = (UInt32*)texture->GetImage();
	int x, y;

	for (y = 0; y < h; y++)
	{
		for (x = 0; x < w; x++)
		{
			*pix++ = colorHex;
		}
	}

	return texture;
}
