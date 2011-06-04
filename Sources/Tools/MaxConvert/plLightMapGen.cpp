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

//#define MF_NEW_RGC

#include "hsTypes.h"
#include "Max.h"
#include "dummy.h"
#include "notify.h"

#include "plLightMapGen.h"
#include "../plGImage/plMipmap.h"
#include "../MaxMain/plMaxNode.h"
#include "../MaxExport/plErrorMsg.h"
#include "plRenderGlobalContext.h"
#include "plMaxLightContext.h"
#include "../plSurface/plLayer.h"
#include "../plSurface/hsGMaterial.h"
#include "../MaxMain/plPluginResManager.h"
#include "../plDrawable/plGeometrySpan.h"
#include "hsFastMath.h"
#include "hsControlConverter.h"
#include "plBitmapCreator.h"
#include "../pnKeyedObject/plKey.h"
#include "../plResMgr/plKeyFinder.h"
#include "../plResMgr/plPageInfo.h"

#include "../plMessage/plLayRefMsg.h"
#include "../plMessage/plMatRefMsg.h"

#include "../MaxComponent/plLightMapComponent.h"

#include "../plGImage/hsCodecManager.h"
#include "../plAgeDescription/plAgeDescription.h"


static plLightMapGen theLMG;

static const float kBlurMapRange = 20.f;

#ifdef MF_NEW_RGC
void getRGC(void* param, NotifyInfo* info)
{
	if( info->intcode == NOTIFY_PRE_RENDERFRAME )
	{
		plLightMapGen* lmg = (plLightMapGen*)param;
		RenderGlobalContext* rgc = (RenderGlobalContext*)info->callParam;
		lmg->SetRGC(rgc);
	}
}
#endif // MF_NEW_RGC

#ifndef MF_NEW_RGC
#define MF_NO_RAY_SHADOW
#endif // MF_NEW_RGC

#define MF_NO_SHADOW_BLUR
#if defined(MF_NEW_RGC) && !defined(MF_NO_SHADOW_BLUR)
#define MF_NO_SHADOW_BLUR
#endif // MF_NEW_RGC

class LMGScanPoint
{
public:
	hsScalar			fU;
	hsPoint3			fBary;
};

class LMGScanlineData
{
public:
	LMGScanlineData() : fEmpty(true) {}

	hsBool				fEmpty;
	LMGScanPoint		fNear;
	LMGScanPoint		fFar;
};

static int kDefaultSize = 64;

static UInt32 MakeUInt32Color(float r, float g, float b, float a)
{
	return (UInt32(a * 255.9f) << 24)
			|(UInt32(r * 255.9f) << 16)
			|(UInt32(g * 255.9f) << 8)
			|(UInt32(b * 255.9f) << 0);
}

plLightMapGen& plLightMapGen::Instance()
{
	return theLMG;
}

#ifdef MF_NEW_RGC
// Don't call this ever ever ever. I mean really. Never.
void plLightMapGen::SetRGC(RenderGlobalContext* rgc) 
{ 
	fRGC = rgc; 
} 
#endif // MF_NEW_RGC

plLightMapGen::plLightMapGen()
:	fWidth(64),
	fHeight(64),
	fScale(1.f),
	fUVWSrc(-1),
	fMapRange(-1.f),
	fInterface(nil),
	fRenderer(nil),
	fRecalcLightMaps(true),
	fRGC(nil),
	fRP(nil)
{
	fWidth = kDefaultSize;
	fHeight = kDefaultSize;
}

plLightMapGen::~plLightMapGen()
{
	Close();
}

// Set up the structures we'll need to compute the lighting.
// You could turn off shadows by commenting out the call to
// MakeRenderInstances, since those are the guys that will
// cast shadows. Modify which lights contribute by changing
// the criteria in IFindLightsRecur.
hsBool plLightMapGen::Open(Interface* ip, TimeValue t, bool forceRegen)
{
	if( !fInterface && ip )
	{
		fInterface = ip;
		fTime = t;

		fRP = TRACKED_NEW RendParams;
		fRP->SetRenderElementMgr(fInterface->GetRenderElementMgr(RS_Production));

#ifdef MF_NEW_RGC
		RegisterNotification(
			getRGC, 
			this, 
			NOTIFY_PRE_RENDERFRAME
			);

		fRenderer = (Renderer*)CreateInstance(RENDERER_CLASS_ID, Class_ID(SREND_CLASS_ID,0));

		ViewParams vp;
		vp.prevAffineTM = Matrix3(true);
		vp.affineTM = Matrix3(true);
		vp.projType = PROJ_PERSPECTIVE;
		vp.hither = 1.f;
		vp.yon = 30.f;
		vp.distance = 1.f;
		vp.zoom = 1.f;
		vp.fov = hsScalarPI / 4.f;
		vp.nearRange = 1.f;
		vp.farRange = 30.f;

		fRenderer->Open(fInterface->GetRootNode(), 
			nil,
			&vp,
			*fRP, 
			fInterface->GetMAXHWnd());

		FrameRendParams frp;
		frp.ambient.Black();
		frp.background.Black();
		frp.globalLightLevel.Black();
		frp.frameDuration = 1.f;
		frp.relSubFrameDuration = 1.f;
		frp.regxmin = 0;
		frp.regxmax = 1;
		frp.regymin = 0;
		frp.regymax = 1;
		frp.blowupCenter = Point2(0.5f,0.5f);
		frp.blowupFactor = Point2(1.f, 1.f);

		BitmapInfo bminfo;
		bminfo.SetType(BMM_TRUE_32);
		bminfo.SetWidth(2);
		bminfo.SetHeight(2);
		bminfo.SetCustWidth(1);
		bminfo.SetCustHeight(1);
		if( !bminfo.Validate() )
		{
			// oops!
			return false;
		}

		Bitmap* tobm = TheManager->Create(&bminfo);


		fRenderer->Render(fTime, 
			tobm, 
			frp, 
			fInterface->GetMAXHWnd()
			);

		tobm->DeleteThis();

#else MF_NEW_RGC

		fRGC = TRACKED_NEW plRenderGlobalContext(fInterface, fTime);
		fRGC->MakeRenderInstances((plMaxNode*)fInterface->GetRootNode(), fTime);

#endif // MF_NEW_RGC

		fPreppedMipmaps.SetCount(0);
		fCreatedLayers.SetCount(0);
		fNewMaps.SetCount(0);

		fAllLights.SetCount(0);
		fActiveLights.SetCount(0);
		IFindLightsRecur((plMaxNode*)fInterface->GetRootNode());
	}
	fRecalcLightMaps = forceRegen;

	return fAllLights.GetCount() > 0;
}

hsBool plLightMapGen::Close()
{
	// HACK to get rid of keys held by the lightmap components, because
	// we can't delete the bitmaps in ICompressLightMaps unless these
	// refs are gone
	for (int i = 0; i < fSharedComponents.size(); i++)
	{
		if (fSharedComponents[i]->GetLightMapKey()) // if it has a key
			fSharedComponents[i]->SetLightMapKey(nil); // nil it out
	}
	fSharedComponents.clear();

	ICompressLightMaps();

#ifndef MF_NEW_RGC
	delete fRGC;
#else // MF_NEW_RGC
	if( fRenderer )
		fRenderer->Close(fInterface->GetMAXHWnd());
	fRenderer = nil;
#endif // MF_NEW_RGC
	fRGC = nil;
	delete fRP;
	fRP = nil;

	fPreppedMipmaps.SetCount(0);
	fCreatedLayers.SetCount(0);
	fNewMaps.SetCount(0);

	IReleaseActiveLights();
	IReleaseAllLights();

	fInterface = nil;

	return true;
}

//#define MIPMAP_LOG

#ifdef MIPMAP_LOG
void DumpMipmap(plMipmap* mipmap, const char* prefix)
{
	hsUNIXStream dump;
	char buf[256];
	sprintf(buf, "log\\%s.txt", prefix);
	dump.Open(buf, "wt");

	for (int i = 0; i < mipmap->GetNumLevels(); i++)
	{
		mipmap->SetCurrLevel(i);

		UInt32 width = mipmap->GetCurrWidth();
		UInt32 height = mipmap->GetCurrHeight();

		sprintf(buf, "----- Level %d (%dx%d) -----\n", i, width, height);
		dump.WriteString(buf);

		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				UInt32 color = *(mipmap->GetAddr32(x, y));
				UInt8 r = ((UInt8)((color)>>16));
				UInt8 g = ((UInt8)((color)>>8));
				UInt8 b = ((UInt8)((color)>>0));
				UInt8 a = ((UInt8)((color)>>24));
				sprintf(buf, "[%3d,%3d,%3d,%3d]", r, g, b, a);
				dump.WriteString(buf);
			}

			dump.WriteString("\n");
		}
	}

	dump.Close();
}
#endif // MIPMAP_LOG

hsBool plLightMapGen::ICompressLightMaps()
{
	int i;
	for( i = 0; i < fPreppedMipmaps.GetCount(); i++ )
	{
		plMipmap* orig = fPreppedMipmaps[i];

		if( orig )
		{
			const hsScalar kFilterSigma = 1.0f;

			if( IsFresh(orig) )
			{
				hsAssert(!orig->IsCompressed(), "How did we just generate a compressed texture?");
				orig->Filter(kFilterSigma);
			}

			if( !orig->IsCompressed() && !(orig->GetFlags() & plMipmap::kForceNonCompressed) )
			{
#ifdef MIPMAP_LOG
				DumpMipmap(orig, orig->GetKeyName());
#endif // MIPMAP_LOG

				plMipmap *compressed = 
					hsCodecManager::Instance().CreateCompressedMipmap(plMipmap::kDirectXCompression, orig);

				if( compressed )
				{
					const plLocation &textureLoc = plPluginResManager::ResMgr()->GetCommonPage(orig->GetKey()->GetUoid().GetLocation(),
																					plAgeDescription::kTextures );
					char name[512];
					sprintf(name, "%s_DX", orig->GetKey()->GetName());

					plKey compKey = hsgResMgr::ResMgr()->FindKey(plUoid(textureLoc, plMipmap::Index(), name));
					if( compKey )
						plBitmapCreator::Instance().DeleteExportedBitmap(compKey);

					hsgResMgr::ResMgr()->NewKey( name, compressed, textureLoc );

					int j;
					for( j = 0; j < fCreatedLayers.GetCount(); j++ )
					{
						if( orig == fCreatedLayers[j]->GetTexture() )
						{
							fCreatedLayers[j]->GetKey()->Release(orig->GetKey());
							hsgResMgr::ResMgr()->AddViaNotify(compressed->GetKey(), TRACKED_NEW plLayRefMsg(fCreatedLayers[j]->GetKey(), plRefMsg::kOnReplace, 0, plLayRefMsg::kTexture), plRefFlags::kActiveRef);
						}
					}

					plBitmapCreator::Instance().DeleteExportedBitmap(orig->GetKey());
				}
			}
		}
	}
	return true;
}

hsBool plLightMapGen::MakeMaps(plMaxNode* node, const hsMatrix44& l2w, const hsMatrix44& w2l, hsTArray<plGeometrySpan *> &spans, plErrorMsg *pErrMsg, plConvertSettings *settings)
{
	const char* dbgNodeName = node->GetName();

	plLightMapComponent* lmapComp = node->GetLightMapComponent();
	if( !lmapComp )
		return false;

	SetUVWSrc(lmapComp->GetUVWSrc());
	SetScale(lmapComp->GetScale());

	// If we don't want maps here, don't bother.
	if( !IWantsMaps(node) )
	{
		pErrMsg->Set(true, node->GetName(), "Lightmap generation requested on bogus object").CheckAndAsk();
		return false;
	}

	if( !IValidateUVWSrc(spans) )
	{
		pErrMsg->Set(true, node->GetName(), "Lightmap generation requested but UVW src bogus. Check mapping.").CheckAndAsk();
		return false;
	}

	// If there aren't any lights, don't bother
	if( !InitNode(node, false) )
	{
		pErrMsg->Set(true, node->GetName(), "Lightmap generation requested but no lights on object. Kind of wasteful.").CheckAndAsk();
		return true;
	}

	// If we have trouble getting a bitmap size, there's probably something wrong with the geometry
	if( !ISelectBitmapDimension(node, l2w, w2l, spans) )
	{
		pErrMsg->Set(true, node->GetName(), "Lightmap generation failure determining bitmap size, probably geometry problem.").CheckAndAsk();
		return false;
	}

	// Okay, we're going to do it. The lights are
	// set up for this guy so we just need some geometry.
	// Find the drawable and which spans correspond 
	// to this node, and feed them through. 
	//
	// IShadeGeometrySpans() and lower return whether any light was actually found.
	if( !IShadeGeometrySpans(node, l2w, w2l, spans) )
	{
		pErrMsg->Set(true, node->GetName(), "Lightmap generation requested but no light found on object. Kind of wasteful.").CheckAndAsk();
	}

	DeInitNode();

	return true;
}

// The next couple of functions don't do anything interesting except
// get us down to the face level where we can work.
hsBool plLightMapGen::IShadeGeometrySpans(plMaxNode* node, const hsMatrix44& l2w, const hsMatrix44& w2l, hsTArray<plGeometrySpan *> &spans)
{
	hsBool retVal = false;
	int i;
	for( i = 0; i < spans.GetCount(); i++ )
	{
		retVal |= IShadeSpan(node, l2w, w2l, *spans[i]);
	}
	return retVal;
}

hsBool plLightMapGen::IsFresh(plBitmap* map) const
{
	return fRecalcLightMaps || fNewMaps.Find(map) != fNewMaps.kMissingIndex;
}

hsBool plLightMapGen::IShadeSpan(plMaxNode* node, const hsMatrix44& l2w, const hsMatrix44& w2l, plGeometrySpan& span)
{
	// This will look for a suitable lightmap layer and return that. If there
	// isn't one already, it will set one up for us.
	plLayerInterface* lay = IGetLightMapLayer(node, span);
	// This next check should never happen, since we've created the layer ourselves.
	if( !lay || !lay->GetTexture() )//|| !lay->GetTexture()->GetBitmap() )
		return false;

	int i;

	if( !(span.fProps & plGeometrySpan::kDiffuseFoldedIn) )
	{
		hsBool foldin = 0 != (span.fProps & plGeometrySpan::kLiteVtxNonPreshaded);
		hsScalar opacity = 1.f;
		hsColorRGBA dif = hsColorRGBA().Set(1.f, 1.f, 1.f, 1.f);
		if( foldin )
		{
			// Find the opacity to fold in.
			// There should be one (or less) layers in this material using layer opacity,
			// or else we should have put this span as kLiteMaterial instead of kLiteVtxNonPreshaded,
			// so we're safe just getting the first opacity on a blended layer.
			// Likewise we're safe getting the first diffuse color that's not on an
			// emissive layer (since emissive layers ignore lightmapping).
			// If we are using kLiteMaterial, we still need to copy from InitColor to Stuff,
			// just don't do the modulate.
			for( i = 0; i < span.fMaterial->GetNumLayers(); i++ )
			{
				if( span.fMaterial->GetLayer(i)->GetBlendFlags() & hsGMatState::kBlendAlpha )
				{
					opacity = span.fMaterial->GetLayer(i)->GetOpacity();
					break;
				}
			}
			for( i = 0; i < span.fMaterial->GetNumLayers(); i++ )
			{
				if( !(span.fMaterial->GetLayer(i)->GetShadeFlags() & hsGMatState::kShadeEmissive) )
				{
					dif = span.fMaterial->GetLayer(i)->GetRuntimeColor();
					break;
				}
			}
		}
		for( i = 0; i < span.fNumVerts; i++ )
		{
			hsColorRGBA multColor, addColor;
			span.ExtractInitColor( i, &multColor, &addColor);
			if( foldin )
			{
				multColor *= dif;	// We like to use kVertexNonPreshaded for lightmapped objects, which needs the runtime diffuse folded in
				multColor.a *= opacity;
			}
			addColor.Set(0,0,0,0);
			span.StuffVertex(i, &multColor, &addColor);
		}
		if( span.fInstanceRefs )
		{
			int j;
			for( j = 0; j < span.fInstanceRefs->GetCount(); j++ )
			{
				plGeometrySpan* inst = (*span.fInstanceRefs)[j];
				inst->fProps |= plGeometrySpan::kDiffuseFoldedIn;
			}
		}
	}
	else
	{
		for( i = 0; i < span.fNumVerts; i++ )
		{
			hsColorRGBA multColor, addColor;
			span.ExtractInitColor( i, &multColor, &addColor);
			addColor.Set(0,0,0,0);
			span.StuffVertex(i, &multColor, &addColor);
		}
		return true;
	}

	// If we aren't recalculating all our lightmaps, then we only want to compute lightmaps
	// which have a creation time of now.
	if( !IsFresh(lay->GetTexture()) )
		return true;

	plMipmap* accum = IMakeAccumBitmap(lay);

	hsBool retVal = false;
	int nFaces = span.fNumIndices / 3;
	for( i = 0; i < nFaces; i++ )
	{
		retVal |= IShadeFace(node, l2w, w2l, span, i, accum);
	}

	IAddToLightMap(lay, accum);

	return retVal;
}

plMipmap* plLightMapGen::IMakeAccumBitmap(plLayerInterface* lay) const
{
	plMipmap* dst = plMipmap::ConvertNoRef( lay->GetTexture() );//->GetBitmap();
	hsAssert( dst != nil, "nil mipmap in IMakeAccumBitmap()" );

	int width = dst->GetWidth();
	int height = dst->GetHeight();

	// Temporary mipmap here, so we don't have to worry about using plBitmapCreator
	plMipmap* bitmap = TRACKED_NEW plMipmap( width, height, plMipmap::kRGB32Config, 1 );
	HSMemory::Clear(bitmap->GetImage(), bitmap->GetHeight() * bitmap->GetRowBytes() );

	return bitmap;
}

hsBool plLightMapGen::IAddToLightMap(plLayerInterface* lay, plMipmap* src) const
{
	plMipmap* dst = plMipmap::ConvertNoRef( lay->GetTexture() );//->GetBitmap();
	hsAssert( dst != nil, "nil mipmap in IAddToLightMap()" );

	src->SetCurrLevel( 0 );
	dst->SetCurrLevel( 0 );

	// BLURLATER
//	static hsScalar kFilterSigma = 0.5f;
//	src->Filter(kFilterSigma);

	// What we really want to do here is antialias our rasterization, so we can
	// just sum in contributions of lighting at the boarder between spans sharing
	// a light map. A quick hackaround is to use the max between existing color
	// (in dst) and current spans illumination contribution (in src).
	int i, j;
	for( j = 0; j < dst->GetHeight(); j++ )
	{
		for( i = 0; i < dst->GetWidth(); i++ )
		{
			UInt32 srcRed = (*src->GetAddr32(i, j) >> 16) & 0xff;
			UInt32 dstRed = (*dst->GetAddr32(i, j) >> 16) & 0xff;
//			dstRed += srcRed;
			if( dstRed < srcRed )
				dstRed = srcRed;
			if( dstRed > 0xff )
				dstRed = 0xff;

			UInt32 srcGreen = (*src->GetAddr32(i, j) >> 8) & 0xff;
			UInt32 dstGreen = (*dst->GetAddr32(i, j) >> 8) & 0xff;
//			dstGreen += srcGreen;
			if( dstGreen < srcGreen )
				dstGreen = srcGreen;
			if( dstGreen > 0xff )
				dstGreen = 0xff;

			UInt32 srcBlue = (*src->GetAddr32(i, j) >> 0) & 0xff;
			UInt32 dstBlue = (*dst->GetAddr32(i, j) >> 0) & 0xff;
//			dstBlue += srcBlue;
			if( dstBlue < srcBlue )
				dstBlue = srcBlue;
			if( dstBlue > 0xff )
				dstBlue = 0xff;

			*dst->GetAddr32(i, j) = 0xff000000
				| (dstRed << 16)
				| (dstGreen << 8)
				| (dstBlue << 0);
		}
	}
	dst->MakeDirty();

	delete src;

	return true;
}

hsBool plLightMapGen::IShadeFace(plMaxNode* node, const hsMatrix44& l2w, const hsMatrix44& w2l, plGeometrySpan& span, int iFace, plMipmap* bitmap)
{
	// Okay, here's where the metal hits the road, whatever that means.
	// We're going to get our bitmap, and step along the face texel by texel,
	// summing up the light at each texel and stuffing it in the bitmap.

	// Set up a light context for the shading below.
	Box3 bbox;
	node->EvalWorldState(fTime).obj->GetDeformBBox(fTime, bbox, &node->GetObjectTM(fTime));
	plMaxLightContext ctx(bbox, fTime);

	// First, get the face info we'll be using.

	// This will look for a suitable lightmap layer and return that. 
	// There should be one there already, because we called this
	// in IShadeSpan
	plLayerInterface* lay = IGetLightMapLayer(node, span);
	int iOurUv = IGetUVWSrc();
	// A little late to be checking this, but whatever...
	if( iOurUv < 0 )
		return false;

	int width = bitmap->GetWidth();
	int height = bitmap->GetHeight();

	hsMatrix44 norml2w;
	hsMatrix44 temp;

	l2w.GetInverse( &temp);
	temp.GetTranspose( &norml2w );

	hsPoint3 pt[3];
	hsVector3 norm[3];
	hsPoint3 uv[3];

	int i;
	for( i = 0; i < 3; i++ )
	{
		hsColorRGBA trash;

		span.ExtractVertex(span.fIndexData[iFace*3 + i], &pt[i], &norm[i], &trash);
		span.ExtractUv(span.fIndexData[iFace*3 + i], iOurUv, &uv[i]);

		pt[i] = l2w * pt[i];
	
		norm[i] = norml2w * norm[i];

		uv[i] = lay->GetTransform() * uv[i];

		uv[i].fX *= width-1;
		uv[i].fX += 0.5f;
		uv[i].fY *= height-1;
		uv[i].fY += 0.5f;

	}

	Color amb(0,0,0);

	return IShadeVerts(ctx, amb, pt, norm, uv, bitmap);
}

hsBool plLightMapGen::IShadeVerts(plMaxLightContext& ctx, const Color& amb, const hsPoint3 pt[3], const hsVector3 norm[3], const hsPoint3 uv[3], plMipmap* bitmap)
{
	int width = bitmap->GetWidth();
	int height = bitmap->GetHeight();
	bitmap->SetCurrLevel( 0 );

	hsTArray<LMGScanlineData> scanline;
	scanline.SetCount(height);

	int lowestV = height;
	int highestV = 0;
	int i0, i1, i2;
	for( i0 = 0; i0 < 3; i0++ )
	{
		i1 = i0 == 2 ? 0 : i0+1;
		i2 = i1 == 2 ? 0 : i1+1;

		hsScalar v0 = uv[i0].fY;
		hsScalar v1 = uv[i1].fY;

		int vStart = int(v0);
		int vEnd = int(v1);
		if( vStart == vEnd )
			continue;

		int vStep = vStart < vEnd ? 1 : -1;
		int vMid;
		for( vMid = vStart; vMid != vEnd + vStep; vMid += vStep )
		{
			// This shouldn't really happen, but might with some slop.
			if( (vMid < 0) || (vMid >= height) )
				continue;

			hsPoint3 bary;
			bary[i0] = (v1 - float(vMid)) / (v1 - v0);
			bary[i1] = 1.f - bary[i0];
			bary[i2] = 0;
			hsScalar u = uv[i0].fX * bary[i0]
						+ uv[i1].fX * bary[i1];
			if( scanline[vMid].fEmpty )
			{
				scanline[vMid].fNear.fU = u;
				scanline[vMid].fNear.fBary = bary;
				scanline[vMid].fFar = scanline[vMid].fNear;
				
				scanline[vMid].fEmpty = false;
				if( vMid < lowestV )
					lowestV = vMid;
				if( vMid > highestV )
					highestV = vMid;
			}
			else
			{
				if( u < scanline[vMid].fNear.fU )
				{
					scanline[vMid].fNear.fU = u;
					scanline[vMid].fNear.fBary = bary;
				}
				else if( u > scanline[vMid].fFar.fU )
				{
					scanline[vMid].fFar.fU = u;
					scanline[vMid].fFar.fBary = bary;
				}
			}
		}
	}
	int i;
	for( i = lowestV; i <= highestV; i++ )
	{
		if( !scanline[i].fEmpty )
		{
			int uStart = int(scanline[i].fNear.fU);
			if( uStart < 0 )
				uStart = 0;
			int uEnd = int(scanline[i].fFar.fU);
			if( uEnd >= width )
				uEnd = width - 1;
			if( uStart == uEnd )
				continue;
			int uMid;
			for( uMid = uStart; uMid <= uEnd; uMid++ )
			{
				hsScalar t = (scanline[i].fFar.fU - float(uMid)) / (scanline[i].fFar.fU - scanline[i].fNear.fU);
				hsPoint3 bary = scanline[i].fNear.fBary * t;
				bary += scanline[i].fFar.fBary * (1.f - t);

				hsPoint3 p = pt[0] * bary[0] + pt[1] * bary[1] + pt[2] * bary[2];
				hsVector3 n = norm[0] * bary[0] + norm[1] * bary[1] + norm[2] * bary[2];

				hsFastMath::NormalizeAppr(n);

				UInt32 color = IShadePoint(ctx, amb, p, n);
				*bitmap->GetAddr32(uMid, i) = color;

			}
		}
	}

	return true;
}

hsBool plLightMapGen::IGetLight(INode* node)
{
	if( node->UserPropExists("RunTimeLight") )
		return false;

	Object *obj = node->EvalWorldState(fTime).obj;

	if (obj && (obj->SuperClassID() == SClass_ID(LIGHT_CLASS_ID))) 
	{
		plLightMapInfo* liInfo = fAllLights.Push();

		LightObject* liObj = (LightObject*)obj;

		liInfo->fResetShadowType = 0;
		liInfo->fResetMapRange = -1.f;
		liInfo->fMapRange = -1.f;

		liInfo->fLiNode = node;
		liInfo->fObjLiDesc = nil;
		liInfo->fNewRender = true;

		return true;
	}

	return false;
}

hsBool plLightMapGen::Update(TimeValue t)
{
	fTime = t;

#ifndef MF_NEW_RGC
	if( fRGC )
		fRGC->Update(t);
#endif // MF_NEW_RGC

	return fAllLights.GetCount() != 0;
}

hsBool plLightMapGen::IFindLightsRecur(INode* node)
{
	IGetLight(node);

	int i;
	for( i = 0; i < node->NumberOfChildren(); i++ )
		IFindLightsRecur(node->GetChildNode(i));

	return fAllLights.GetCount() > 0;
}

hsBool plLightMapGen::InitNode(INode* node, hsBool softShadow)
{
	fActiveLights.SetCount(0);

	plMaxNode* maxNode = (plMaxNode*)node;
	if( !maxNode->CanConvert() )
		return false;

	if( maxNode->GetNoPreShade() )
		return false;

#ifndef MF_NO_SHADOW_BLUR
	fMapRange = softShadow ? kBlurMapRange : -1.f;
#endif // MF_NO_SHADOW_BLUR

	IFindActiveLights((plMaxNode*)node);
	
	return fActiveLights.GetCount() > 0;
}

hsBool plLightMapGen::DeInitNode()
{
	IReleaseActiveLights();

	return true;
}

hsBounds3Ext plLightMapGen::IGetBoundsLightSpace(INode* node, INode* liNode)
{
	TimeValue currTime(0);

	hsBounds3Ext bnd;
	bnd.MakeEmpty();
	Object *obj = node->EvalWorldState(currTime).obj;
	if( !obj )
		return bnd;

	Box3 box;

	if( obj->ClassID() == Class_ID(DUMMY_CLASS_ID,0) )
	{
		DummyObject* dummy = (DummyObject*)obj;
		box = dummy->GetBox();
	}
	else
	if( obj->CanConvertToType(triObjectClassID) )
	{
		TriObject	*meshObj = (TriObject *)obj->ConvertToType(currTime, triObjectClassID);
		if( !meshObj )
			return bnd;

		Mesh& mesh = meshObj->mesh;
		box = mesh.getBoundingBox();
		
		if( meshObj != obj )
			meshObj->DeleteThis();
	}

	bnd.Union(&hsPoint3(box.pmin.x, box.pmin.y, box.pmin.z));
	bnd.Union(&hsPoint3(box.pmax.x, box.pmax.y, box.pmax.z));

	Matrix3 maxL2W = node->GetObjectTM(currTime);
	Matrix3 maxW2Light = Inverse(liNode->GetObjectTM(currTime));
	Matrix3 maxL2Light = maxL2W * maxW2Light;
	hsMatrix44 l2l;
	hsControlConverter::Instance().Matrix3ToHsMatrix44(&maxL2Light, &l2l);


	bnd.Transform(&l2l);

	return bnd;
}

hsBool plLightMapGen::IDirAffectsNode(plLightMapInfo* liInfo, LightObject* liObj, INode* node)
{
	hsBounds3Ext bnd = IGetBoundsLightSpace(node, liInfo->fLiNode);

	if( bnd.GetType() != kBoundsNormal )
		return false;

	if( bnd.GetMins().fZ > 0 )
		return false;

	LightState ls;
	liObj->EvalLightState(TimeValue(0), FOREVER, &ls);

	hsScalar radX = ls.fallsize;
	hsScalar radY = radX;
	if( ls.shape == RECT_LIGHT )
		radY /= ls.aspect;

	if( bnd.GetMins().fX > radX )
		return false;
	if( bnd.GetMaxs().fX < -radX )
		return false;

	if( bnd.GetMins().fY > radY )
		return false;
	if( bnd.GetMaxs().fY < -radY )
		return false;

	if( !ls.useAtten )
		return true;

	if( bnd.GetMaxs().fZ < -ls.attenEnd )
		return false;

	return true;
}

hsBool plLightMapGen::ISpotAffectsNode(plLightMapInfo* liInfo, LightObject* liObj, INode* node)
{
	hsBounds3Ext bnd = IGetBoundsLightSpace(node, liInfo->fLiNode);

	if( bnd.GetType() != kBoundsNormal )
		return false;

	if( bnd.GetMins().fZ > 0 )
		return false;

	LightState ls;
	liObj->EvalLightState(TimeValue(0), FOREVER, &ls);

	hsScalar coneRad[2];
	coneRad[0] = ls.fallsize * hsScalarPI / 180.f;
	coneRad[1] = coneRad[0];
	if( ls.shape == RECT_LIGHT )
		coneRad[1] /= ls.aspect;

	hsPoint3 corners[8];
	bnd.GetCorners(corners);

	int numPos[4] = { 0, 0, 0, 0 };
	int j;
	for( j = 0; j < 8; j++ )
	{
		hsScalar rad;
		rad = hsScalar(atan2(corners[j].fX, -corners[j].fZ));
		if( rad > coneRad[0] )
			numPos[0]++;
		if( rad < -coneRad[0] )
			numPos[2]++;
		rad = hsScalar(atan2(corners[j].fY, -corners[j].fZ));
		if( rad > coneRad[1] )
			numPos[1]++;
		if( rad < -coneRad[1] )
			numPos[3]++;
	}
	for( j = 0; j < 4; j++ )
	{
		if( numPos[j] >= 8 )
			return false;
	}

	if( ls.useAtten )
	{
		if( bnd.GetMaxs().fZ < -ls.attenEnd )
			return false;
	}

	return true;
}

hsBool plLightMapGen::IOmniAffectsNode(plLightMapInfo* liInfo, LightObject* liObj, INode* node)
{
	LightState ls;
	liObj->EvalLightState(TimeValue(0), FOREVER, &ls);

	if( !ls.useAtten )
		return true;

	hsBounds3Ext bnd = IGetBoundsLightSpace(node, liInfo->fLiNode);

	if( bnd.GetType() != kBoundsNormal )
		return false;

	hsScalar radius = ls.attenEnd;

	int i;
	for( i = 0; i < 3; i++ )
	{
		if( bnd.GetMins()[i] > radius )
			return false;
		if( bnd.GetMaxs()[i] < -radius )
			return false;
	}

	return true;
}

hsBool plLightMapGen::ILightAffectsNode(plLightMapInfo* liInfo, LightObject* liObj, INode* node)
{
	const char* liName = liInfo->fLiNode->GetName();
	const char* nodeName = node->GetName();

	LightState ls;
	liObj->EvalLightState(TimeValue(0), FOREVER, &ls);

	hsBool excluded = false;
	if( !liObj->GetUseLight() )
	{
		excluded = true;
	}
	if( !excluded && liObj->GetExclList() && liObj->GetExclList()->TestFlag(NT_AFFECT_ILLUM) )
	{
		hsBool inExc = -1 != liObj->GetExclList()->FindNode(node);
		if( (!inExc) ^ (!liObj->GetExclList()->TestFlag(NT_INCLUDE)) )
			excluded = true;
	}
	if( excluded )
		return false;

	switch( ls.type )
	{
	case OMNI_LGT:
		return IOmniAffectsNode(liInfo, liObj, node);
	case SPOT_LGT:
		return ISpotAffectsNode(liInfo, liObj, node);
	case DIRECT_LGT:
		return IDirAffectsNode(liInfo, liObj, node);
	default:
	case AMBIENT_LGT:
		return true;
	}
	return false;
}

hsBool plLightMapGen::IPrepLight(plLightMapInfo* liInfo, INode* node)
{
	const char* liName = liInfo->fLiNode->GetName();
	const char* nodeName = node->GetName();

	INode* liNode = liInfo->fLiNode;
	LightObject* liObj = (LightObject*)liNode->EvalWorldState(fTime).obj;

	// redundant check, if it doesn't have a light object it shouldn't be in the list
	if( liObj )
	{
		hsBool affectsNode = ILightAffectsNode(liInfo, liObj, node);
		if( affectsNode )
		{

#ifdef MF_NO_RAY_SHADOW
			// for reasons known only to god and someone deep in the bowels of kinetix,
			// the lighting is (sometimes) barfing if the shadow type is ray-traced.
			// until i can track that down, i'll force shadow mapped shadows.
			liInfo->fResetShadowType = liObj->GetShadowType();
			if( liInfo->fResetShadowType > 0 )
			{
				liObj->SetShadowType(0);
				liInfo->fNewRender = true;
			}
#endif MF_NO_RAY_SHADOW
			if( fMapRange > 0 )
			{
				if( liInfo->fMapRange != fMapRange )
				{
					liInfo->fResetMapRange = liObj->GetMapRange(fTime);
					liObj->SetMapRange(fTime, fMapRange);
					liInfo->fMapRange = fMapRange;
					liInfo->fNewRender = true;
				}
			}
			else if( liInfo->fResetMapRange > 0 )
			{
				if( liInfo->fMapRange != liInfo->fResetMapRange )
				{
					liObj->SetMapRange(fTime, liInfo->fResetMapRange);
					liInfo->fMapRange = liInfo->fResetMapRange;
					liInfo->fNewRender = true;
				}
			}
			
			ObjLightDesc* objLiDesc = liInfo->fObjLiDesc;
			if( !objLiDesc )
				objLiDesc = liObj->CreateLightDesc(liNode);
			
			plMaxRendContext rc;
			objLiDesc->Update(fTime, rc, fRGC, node->RcvShadows(), liInfo->fNewRender);
			objLiDesc->UpdateViewDepParams(Matrix3(true));
			
			liInfo->fNewRender = false;

			liInfo->fObjLiDesc = objLiDesc;
			
			fActiveLights.Append(liInfo);
		}
	}

	return true;
}

hsBool plLightMapGen::IFindActiveLights(plMaxNode* node)
{
	fActiveLights.SetCount(0);
	int i;
	for( i = 0; i < fAllLights.GetCount(); i++ )
	{
		IPrepLight(&fAllLights[i], node);
	}

	return fActiveLights.GetCount() > 0;
}

hsBool plLightMapGen::IReleaseAllLights()
{
	int i;
	for( i = 0; i < fAllLights.GetCount(); i++ )
	{
		if( fAllLights[i].fResetMapRange > 0 )
		{
			LightObject* liObj = (LightObject*)fAllLights[i].fLiNode->EvalWorldState(fTime).obj;
			liObj->SetMapRange(fTime, fAllLights[i].fResetMapRange);

		}
#ifdef MF_NO_RAY_SHADOW
		// Fix the shadow method back.
		if( fAllLights[i].fResetShadowType > 0 )
		{
			LightObject* liObj = (LightObject*)fAllLights[i].fLiNode->EvalWorldState(fTime).obj;
			liObj->SetShadowType(fAllLights[i].fResetShadowType);
		}
#endif // MF_NO_RAY_SHADOW

		if( fAllLights[i].fObjLiDesc )
			fAllLights[i].fObjLiDesc->DeleteThis();

		fAllLights[i].fObjLiDesc = nil;
	}
	fAllLights.SetCount(0);
	
	return true;
}

hsBool plLightMapGen::IReleaseActiveLights()
{
	fActiveLights.SetCount(0);

	return true;
}

hsBool plLightMapGen::IWantsMaps(plMaxNode* node)
{
	if( !(node->CanConvert() && node->GetDrawable()) )
		return false;

	return nil != node->GetLightMapComponent();
}

hsBool plLightMapGen::IValidateUVWSrc(hsTArray<plGeometrySpan *>& spans) const
{
	int i;
	for( i = 0; i < spans.GetCount(); i++ )
	{
		int numUVWs = spans[i]->GetNumUVs();
		if( IGetUVWSrc() >= numUVWs )
			return false;
	}
	return true;
}

void plLightMapGen::IInitBitmapColor(plMipmap* bitmap, const hsColorRGBA& col) const
{
	UInt32 initColor = MakeUInt32Color(col.r, col.g, col.b, col.a);
	UInt32* pix = (UInt32*)bitmap->GetImage();
	UInt32* pixEnd = ((UInt32*)bitmap->GetImage()) + bitmap->GetWidth() * bitmap->GetHeight();
	while( pix < pixEnd )
		*pix++ = initColor;
}

plLayerInterface* plLightMapGen::IGetLightMapLayer(plMaxNode* node, plGeometrySpan& span)
{
	plLayerInterface* lay = IMakeLightMapLayer(node, span);

	plMipmap* mip = plMipmap::ConvertNoRef(lay->GetTexture());
	hsAssert(mip, "This should have been a mipmap we created ourselves.");
	if( !mip )
		return nil;
	if( fPreppedMipmaps.Find(mip) == fPreppedMipmaps.kMissingIndex )
	{
		if( IsFresh(mip) )
		{
			hsColorRGBA initColor = node->GetLightMapComponent()->GetInitColor();
			// Get this off the node, where the lightmap component has stashed it.
			IInitBitmapColor(mip, initColor);
		}

		fPreppedMipmaps.Append(mip);
	}
	if( fCreatedLayers.Find(lay) == fCreatedLayers.kMissingIndex )
	{
		fCreatedLayers.Append(lay);
	}
	return lay;
}

plLayerInterface* plLightMapGen::IMakeLightMapLayer(plMaxNode* node, plGeometrySpan& span)
{
	hsGMaterial* mat = span.fMaterial;

	int i;
	for( i = 0; i < mat->GetNumPiggyBacks(); i++ )
	{
		if( mat->GetPiggyBack(i)->GetMiscFlags() & hsGMatState::kMiscLightMap )
			return mat->GetPiggyBack(i);
	}

	char newMatName[256];
	sprintf(newMatName, "%s_%s_LIGHTMAPGEN", mat->GetKey()->GetName(), node->GetName());
	plLocation nodeLoc = node->GetLocation();

	plKey matKey = hsgResMgr::ResMgr()->FindKey(plUoid(nodeLoc, hsGMaterial::Index(), newMatName));
	if( matKey )
	{
		mat = hsGMaterial::ConvertNoRef(matKey->ObjectIsLoaded());
		for( i = 0; i < mat->GetNumPiggyBacks(); i++ )
		{
			if( mat->GetPiggyBack(i)->GetMiscFlags() & hsGMatState::kMiscLightMap )
			{
				span.fMaterial = mat;
				return mat->GetPiggyBack(i);
			}
		}
		hsAssert(false, "Something not a light map material registered with our name?");
	}
	hsGMaterial* objMat = nil;
	
	hsBool sharemaps = node->GetLightMapComponent()->GetShared();
	if( sharemaps )
	{
		objMat = mat;
	}
	else
	{
		objMat = TRACKED_NEW hsGMaterial;
		hsgResMgr::ResMgr()->NewKey(newMatName, objMat, nodeLoc);

		for( i = 0; i < mat->GetNumLayers(); i++ )
			hsgResMgr::ResMgr()->AddViaNotify(mat->GetLayer(i)->GetKey(), TRACKED_NEW plMatRefMsg(objMat->GetKey(), plRefMsg::kOnCreate, -1, plMatRefMsg::kLayer), plRefFlags::kActiveRef);
	}

	objMat->SetCompositeFlags(objMat->GetCompositeFlags() | hsGMaterial::kCompIsLightMapped);

	// Make sure layer (and mip) name are unique across pages by putting the page name in
	const plPageInfo* pageInfo = plKeyFinder::Instance().GetLocationInfo(node->GetLocation());

	char layName[256];
	sprintf(layName, "%s_%s_LIGHTMAPGEN", pageInfo->GetPage(), node->GetName());
	
	plKey layKey = node->FindPageKey(plLayer::Index(), layName);


	if( !layKey )
	{
		int w = fWidth;
		int h = fHeight;

		plKey mipKey;
		if( node->GetLightMapComponent()->GetLightMapKey() )
		{
			mipKey = node->GetLightMapComponent()->GetLightMapKey();
		}
		else
		{
			char mipmapName[ 256 ];
			sprintf( mipmapName, "%s_mip", layName );

			// Deleted the NOTE here because it was incorrect in every meaningful sense of the word. - mf

			const plLocation &textureLoc = plPluginResManager::ResMgr()->GetCommonPage( nodeLoc, plAgeDescription::kTextures );

			mipKey = hsgResMgr::ResMgr()->FindKey(plUoid(textureLoc, plMipmap::Index(), mipmapName));

			if( !mipKey && !fRecalcLightMaps )
			{
				char compressedName[512];
				sprintf(compressedName, "%s_DX", mipmapName);

				plKey compKey = hsgResMgr::ResMgr()->FindKey(plUoid(textureLoc, plMipmap::Index(), compressedName));

				if( compKey )
					mipKey = compKey;
			}

			if( mipKey )
			{
				plBitmap* bitmap = plBitmap::ConvertNoRef(mipKey->ObjectIsLoaded());
				if( bitmap )
				{
					if( node->GetLightMapComponent()->GetCompress() != bitmap->IsCompressed() )
					{
						// make sure the lightmap component isn't holding a key,
						// it will get assigned one a few lines later anyway
						if (node->GetLightMapComponent()->GetLightMapKey())
							node->GetLightMapComponent()->SetLightMapKey(nil);

						plBitmapCreator::Instance().DeleteExportedBitmap(mipKey);
					}
				}
			}

			if( !mipKey )
			{
				plMipmap* bitmap = plBitmapCreator::Instance().CreateBlankMipmap(w, h, plMipmap::kRGB32Config, 1, mipmapName, nodeLoc);
				mipKey = bitmap->GetKey();
				fNewMaps.Append(bitmap);

				if( !node->GetLightMapComponent()->GetCompress() )
					bitmap->SetFlags(bitmap->GetFlags() | plMipmap::kForceNonCompressed);
			}
			if( node->GetLightMapComponent()->GetShared() )
			{
				// HACK since we are setting the key, save the pointer to the light map
				// component so we can get rid of the key it holds later
				fSharedComponents.push_back(node->GetLightMapComponent());

				node->GetLightMapComponent()->SetLightMapKey(mipKey);
			}
		}

		plLayer* layer = TRACKED_NEW plLayer;
		layer->InitToDefault();
		layKey = hsgResMgr::ResMgr()->NewKey(layName, layer, nodeLoc);
		hsgResMgr::ResMgr()->AddViaNotify(mipKey, TRACKED_NEW plLayRefMsg(layer->GetKey(), plRefMsg::kOnCreate, 0, plLayRefMsg::kTexture), plRefFlags::kActiveRef);
		layer->SetAmbientColor(hsColorRGBA().Set(1.f, 1.f, 1.f, 1.f));
		layer->SetZFlags(hsGMatState::kZNoZWrite);
		layer->SetBlendFlags(hsGMatState::kBlendMult);
		layer->SetClampFlags(hsGMatState::kClampTexture);
		layer->SetUVWSrc(IGetUVWSrc());
		layer->SetMiscFlags(hsGMatState::kMiscLightMap);
	}

	hsgResMgr::ResMgr()->AddViaNotify(layKey, TRACKED_NEW plMatRefMsg(objMat->GetKey(), plRefMsg::kOnCreate, -1, plMatRefMsg::kPiggyBack), plRefFlags::kActiveRef);

	span.fMaterial = objMat;


	return plLayerInterface::ConvertNoRef(layKey->GetObjectPtr());

}

// Like ShadePoint, but only computes the amount of light striking the surface,
// so ignoring the N dot L term.
Color plLightMapGen::ShadowPoint(plMaxLightContext& ctx)
{
	ctx.globContext = fRGC;

	Color accum;
	accum.Black();
	int i;
	for( i = 0; i < fActiveLights.GetCount(); i++ )
	{
		const char* dbgLiName = fActiveLights[i]->fLiNode->GetName();

		Color color;
		Point3 liDir;
		float dot_nl, diffuseCoef;
		BOOL hit = fActiveLights[i]->fObjLiDesc->Illuminate(ctx, ctx.Normal(), color, liDir, dot_nl, diffuseCoef);
		if( hit )
		{
			accum += color;
		}
	}

	return accum;
}

Color plLightMapGen::ShadePoint(plMaxLightContext& ctx)
{
	ctx.globContext = fRGC;

	Color accum;
	accum.Black();
	int i;
	for( i = 0; i < fActiveLights.GetCount(); i++ )
	{
		Color color;
		Point3 liDir;
		float dot_nl, diffuseCoef;
		BOOL hit = fActiveLights[i]->fObjLiDesc->Illuminate(ctx, ctx.Normal(), color, liDir, dot_nl, diffuseCoef);
		if( hit )
		{
			accum += color * diffuseCoef;
		}
	}

	return accum;
}

Color plLightMapGen::ShadePoint(plMaxLightContext& ctx, const Point3& p, const Point3& n)
{
	ctx.SetPoint(p, n);

	return ShadePoint(ctx);

}

Color plLightMapGen::ShadePoint(plMaxLightContext& ctx, const hsPoint3& p, const hsVector3& n)
{
	ctx.SetPoint(p, n);

	return ShadePoint(ctx);

}

UInt32 plLightMapGen::IShadePoint(plMaxLightContext& ctx, const Color& amb, const hsPoint3& p, const hsVector3& n)
{
	ctx.globContext = fRGC;
	ctx.SetPoint(p, n);

	Color accum = ShadePoint(ctx);
	accum += amb;
	accum.ClampMinMax();

	UInt32 retVal;

	retVal = MakeUInt32Color(accum.r, accum.g, accum.b, 1.f);

	return retVal;
}

hsBool plLightMapGen::ISelectBitmapDimension(plMaxNode* node, const hsMatrix44& l2w, const hsMatrix44& w2l, hsTArray<plGeometrySpan *>& spans)
{
	float duDr = 0;
	float dvDr = 0;

	float totFaces = 0;

	int i;
	for( i = 0; i < spans.GetCount(); i++ )
	{
		plGeometrySpan *span = spans[i];

		int nFaces = span->fNumIndices / 3;
		int j;
		for( j = 0; j < nFaces; j++ )
		{
			hsPoint3 pt[3];
			hsPoint3 uv[3];

			int k;
			for( k = 0; k < 3; k++ )
			{
				hsVector3 vTrash;
				hsColorRGBA cTrash;

				span->ExtractVertex(span->fIndexData[j*3 + k], &pt[k], &vTrash, &cTrash);

				pt[k] = l2w * pt[k];

				span->ExtractUv(span->fIndexData[j*3 + k], IGetUVWSrc(), &uv[k]);
			}

			if( (uv[0].fX >= 1.f)
				&&(uv[1].fX >= 1.f)
				&&(uv[2].fX >= 1.f) )
				continue;

			if( (uv[0].fY >= 1.f)
				&&(uv[1].fY >= 1.f)
				&&(uv[2].fY >= 1.f) )
				continue;

			if( (uv[0].fX <= 0)
				&&(uv[1].fX <= 0)
				&&(uv[2].fX <= 0) )
				continue;

			if( (uv[0].fY <= 0)
				&&(uv[1].fY <= 0)
				&&(uv[2].fY <= 0) )
				continue;

			float magDU[2];
			magDU[0] = fabsf(uv[1].fX - uv[0].fX);
			magDU[1] = fabsf(uv[2].fX - uv[0].fX);
			if( magDU[0] > magDU[1] )
			{
				float dist = hsVector3(pt+1, pt+0).Magnitude();

				if( dist > 1.e-3f )
					duDr += magDU[0] / dist;
			}
			else
			{
				float dist = hsVector3(pt+2, pt+0).Magnitude();

				if( dist > 1.e-3f )
					duDr += magDU[1] / dist;
			}

			float magDV[2];
			magDV[0] = fabsf(uv[1].fY - uv[0].fY);
			magDV[1] = fabsf(uv[2].fY - uv[0].fY);
			if( magDV[0] > magDV[1] )
			{
				float dist = hsVector3(pt+1, pt+0).Magnitude();

				if( dist > 1.e-3f )
					dvDr += magDV[0] / dist;
			}
			else
			{
				float dist = hsVector3(pt+2, pt+0).Magnitude();

				if( dist > 1.e-3f )
					dvDr += magDV[1] / dist;
			}

			totFaces++;
		}
	}

	if( totFaces < 1.f )
		return false;

	duDr /= totFaces;
	dvDr /= totFaces;

	const int kMaxSize = 256;
	const int kMinSize = 32;
	const int kMaxAspect = 8;

	const float kTexPerFoot = 1.f;

	if( duDr > 0 )
	{
		fWidth = kTexPerFoot / duDr;

		if( fWidth > kMaxSize )
			fWidth = kMaxSize;
		if( fWidth < kMinSize )
			fWidth = kMinSize;
	}
	else
	{
		fWidth = kMinSize;
	}
	fWidth *= fScale;
	fWidth = IPowerOfTwo(fWidth);
	
	if( dvDr > 0 )
	{
		fHeight = kTexPerFoot / duDr;

		if( fHeight > kMaxSize )
			fHeight = kMaxSize;
		if( fHeight < kMinSize )
			fHeight = kMinSize;
	}
	else
	{
		fHeight = kMinSize;
	}
	fHeight *= fScale;
	fHeight = IPowerOfTwo(fHeight);

	if( fHeight / fWidth > kMaxAspect )
		fWidth = fHeight / kMaxAspect;
	if( fWidth / fHeight > kMaxAspect )
		fHeight = fWidth / kMaxAspect;

	if( fWidth > 512 )
		fWidth = 512;
	if( fHeight > 512 )
		fHeight = 512;

	return true;
}

int plLightMapGen::IPowerOfTwo(int sz) const
{
	int i = 0;
	while( (1 << i) < sz )
		i++;

	int p2sz = 1 << i;

	if( p2sz - sz > sz - (p2sz >> 1) )
		p2sz >>= 1;

	return p2sz;
}