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
///////////////////////////////////////////////////////////////////////////////
//																			 //
//	plLayerConverter - Utility class that converts plPlasmaMAXLayers into	 //
//					   other stuff.											 //
//	Cyan, Inc.																 //
//																			 //
//// Version History //////////////////////////////////////////////////////////
//																			 //
//	1.13.2002 mcn - Created.												 //
//																			 //
//// Notes ////////////////////////////////////////////////////////////////////
//																			 //
//	If you add a new PlasmaMAXLayer type, you need to add the appropriate	 //
//	test in ConvertTexmap() as well as your own function to do the			 //
//	conversion. Messy, but thanks to the dependencies of the convert process,//
//	we can't do it the nice, pretty, OOP way.								 //
//																			 //
///////////////////////////////////////////////////////////////////////////////

#include "hsWindows.h"
#include "Max.h"
#include "stdmat.h"
#include "istdplug.h"
#include "iparamb2.h"
#include "iparamm2.h"

#include "hsUtils.h"
#include "hsExceptionStack.h"

#include "plLayerConverter.h"

#include "../MaxConvert/hsMaxLayerBase.h"
#include "../MaxConvert/hsConverterUtils.h"
#include "../MaxConvert/hsControlConverter.h"
#include "../MaxConvert/hsMaterialConverter.h"
#include "../MaxConvert/plBitmapCreator.h"
#include "hsResMgr.h"
#include "../MaxExport/plErrorMsg.h"
#include "../MaxMain/plMaxNode.h"

#include "../pnKeyedObject/plUoid.h"
#include "../pnKeyedObject/plKey.h"
#include "../pnSceneObject/plSceneObject.h"
#include "../plSurface/plLayerInterface.h"
#include "../plSurface/plLayer.h"
#include "../pnMessage/plRefMsg.h"
#include "../pnMessage/plObjRefMsg.h"
#include "../plMessage/plLayRefMsg.h"
#include "../plDrawable/plGeometrySpan.h"

#include "../plGImage/plMipmap.h"
#include "../plGImage/plDynamicTextMap.h"
#include "../plGImage/plCubicEnvironmap.h"
#include "../plPipeline/plCubicRenderTarget.h"
#include "../plPipeline/plCubicRenderTargetModifier.h"
#include "../plPipeline/plDynamicEnvMap.h"

#include "../pfSurface/plLayerAVI.h"
#include "../pfSurface/plLayerBink.h"

#include "../MaxPlasmaMtls/Layers/plPlasmaMAXLayer.h"
#include "../MaxPlasmaMtls/Layers/plLayerTex.h"
#include "../MaxPlasmaMtls/Layers/plLayerTexBasicPB.h"
#include "../MaxPlasmaMtls/Layers/plLayerTexBitmapPB.h"
#include "../MaxPlasmaMtls/Layers/plStaticEnvLayer.h"
#include "../MaxPlasmaMtls/Layers/plDynamicEnvLayer.h"
#include "../MaxPlasmaMtls/Layers/plDynamicTextLayer.h"
#include "../MaxPlasmaMtls/Layers/plAngleAttenLayer.h"
#include "../MaxPlasmaMtls/Layers/plMAXCameraLayer.h"

#include "../MaxComponent/plComponent.h"
#include "../MaxComponent/plWaterComponent.h"
#include "../pfCamera/plCameraModifier.h"

//// Local Static Stuff ///////////////////////////////////////////////////////

namespace
{
	enum {
		kWarnedTooManyUVs			= 0x01,
		kWarnedNoBaseTexture		= 0x02,
		kWarnedUpperTextureMissing	= 0x04,
		kWarnedNoUpperTexture		= 0x08,
	};

	const char	sWarnBaseTextureMissing[] = "The object \"%s\"'s material has a base layer that is assigned texture \"%s\", but the texture file is missing. "
										"This can cause unwanted effects during runtime."; 
	const char	sWarnUpperTextureMissing[] = "The object \"%s\"'s material has an upper layer that is assigned texture \"%s\", but the texture file is missing. "
										"This is not supported in the engine, so the upper layer will be ignored."; 
	const char	sWarnNoUpperTexture[] = "The object \"%s\"'s material has an uppper layer that is not assigned a texture. "
										"This is not supported in the engine, so the upper layer will be disabled."; 
}


//// Constructor/Destructor ///////////////////////////////////////////////////

plLayerConverter::plLayerConverter() :
	fInterface( nil ),
	fConverterUtils( hsConverterUtils::Instance() )
{
	fErrorMsg = nil;
	fWarned = 0;
	fSaving = false;
}

plLayerConverter::~plLayerConverter()
{
}

plLayerConverter	&plLayerConverter::Instance( void )
{
	hsGuardBegin( "plLayerConverter::Instance" );

	static plLayerConverter		instance;

	return instance;

	hsGuardEnd;
}

void	plLayerConverter::Init( hsBool save, plErrorMsg *msg )
{
	fSaving = save;
	fErrorMsg = msg;
	fWarned = 0;
	fInterface = GetCOREInterface();
}

void	plLayerConverter::DeInit( void )
{
	int i;
	for( i = 0; i < fConvertedLayers.GetCount(); i++ )
	{
		if( fConvertedLayers[ i ] != nil )
			fConvertedLayers[ i ]->IClearConversionTargets();
	}
	fConvertedLayers.Reset();
}

//// Mute/Unmute Warnings /////////////////////////////////////////////////////

void	plLayerConverter::MuteWarnings( void )
{
	fSavedWarned = fWarned; 
	fWarned |= kWarnedNoBaseTexture | kWarnedUpperTextureMissing; 
}

void	plLayerConverter::UnmuteWarnings( void )
{
	fWarned = fSavedWarned; 
}

///////////////////////////////////////////////////////////////////////////////
//// Main Switcheroo //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//// ConvertTexmap ////////////////////////////////////////////////////////////
		
plLayerInterface	*plLayerConverter::ConvertTexmap( Texmap *texmap,
														plMaxNode *maxNode,
														UInt32 blendFlags, hsBool preserveUVOffset,
														hsBool upperLayer )
{
	hsGuardBegin( "plLayerConverter::ConvertTexmap" );
	
	fDbgNodeName = maxNode->GetName();

	// We only convert plPlasmaMAXLayers
	plPlasmaMAXLayer	*layer = plPlasmaMAXLayer::GetPlasmaMAXLayer( texmap );
	if( layer == nil )
	{
		fErrorMsg->Set( true, "Plasma Layer Error", "Cannot convert layer '%s'--unrecognized MAX layer type", texmap->GetName() );
		fErrorMsg->Show();
		fErrorMsg->Set();
		return nil;
	}

	// KLUDGE - Some things don't set the name for their layers (ie projected
	// runtime lights).  So that we don't end up with an empty keyname, set the
	// name to the nodes name if there isn't one. -Colin
	const char* layerName = layer->GetName();
	if (!layerName || layerName[0] == '\0')
		layer->SetName(maxNode->GetName());

	// Switch on the class ID
	plLayerInterface	*plasmaLayer = nil;

	if( layer->ClassID() == LAYER_TEX_CLASS_ID )
		plasmaLayer = IConvertLayerTex( layer, maxNode, blendFlags, preserveUVOffset, upperLayer );

	else if( layer->ClassID() == STATIC_ENV_LAYER_CLASS_ID )
		plasmaLayer = IConvertStaticEnvLayer( layer, maxNode, blendFlags, preserveUVOffset, upperLayer );

	else if( layer->ClassID() == DYNAMIC_ENV_LAYER_CLASS_ID )
		plasmaLayer = IConvertDynamicEnvLayer( layer, maxNode, blendFlags, preserveUVOffset, upperLayer );

	else if( layer->ClassID() == DYN_TEXT_LAYER_CLASS_ID )
		plasmaLayer = IConvertDynamicTextLayer( layer, maxNode, blendFlags, preserveUVOffset, upperLayer );

	else if( layer->ClassID() == ANGLE_ATTEN_LAYER_CLASS_ID )
		plasmaLayer = IConvertAngleAttenLayer( layer, maxNode, blendFlags, preserveUVOffset, upperLayer );

	else if( layer->ClassID() == MAX_CAMERA_LAYER_CLASS_ID )
		plasmaLayer = IConvertCameraLayer( layer, maxNode, blendFlags, preserveUVOffset, upperLayer );


	IRegisterConversion( layer, plasmaLayer );

	return plasmaLayer;

	hsGuardEnd;
}

//// IRegisterConversion //////////////////////////////////////////////////////

void	plLayerConverter::IRegisterConversion( plPlasmaMAXLayer *origLayer, plLayerInterface *convertedLayer )
{
	if( convertedLayer == nil )
		return;

	// Add this to our list of converted layers (so we can clean them up later)
	if( fConvertedLayers.Find( origLayer ) == fConvertedLayers.kMissingIndex )
		fConvertedLayers.Append( origLayer );

	// Now add the converted layer to that layer's list of conversion targets.
	// (easier than us keeping a huge lookup table, since this is *acting* 
	//  as that lookup table more or less)
	origLayer->IAddConversionTarget( convertedLayer );
}

///////////////////////////////////////////////////////////////////////////////
//// Main Processing Functions ////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//// IConvertLayerTex /////////////////////////////////////////////////////////
							
plLayerInterface	*plLayerConverter::IConvertLayerTex( plPlasmaMAXLayer *layer, 
																plMaxNode *maxNode, UInt32 blendFlags, 
																hsBool preserveUVOffset, hsBool upperLayer )
{
	hsGuardBegin( "plLayerConverter::IConvertLayerTex" );

	IParamBlock2		*bitmapPB;
	plLocation			loc;


	loc = maxNode->GetLocation();
	bitmapPB = layer->GetParamBlockByID( plLayerTex::kBlkBitmap );
	
	if( !bitmapPB )
	{
		fErrorMsg->Set( !bitmapPB, "Plasma Layer Error", "Bitmap paramblock for Plasma Layer not found" ).Show();
		fErrorMsg->Set();
		return nil;
	}

	// Get a new layer to play with
	plLayer *plasmaLayer = ICreateLayer( layer->GetName(), upperLayer, loc );

	// We're using a texture, try and get its info
	PBBitmap	*pbbm = nil;
	BitmapInfo	*bi = nil;

	if( bitmapPB->GetInt( kBmpUseBitmap ) )
	{
		if( bitmapPB )
			pbbm = bitmapPB->GetBitmap( kBmpBitmap );
		if( pbbm )
			bi = &pbbm->bi;
	}

	// If the texture had bad info, assert and return the empty layer
	if( !bi || !bi->Name() || !strcmp(bi->Name(), "") )
	{
		if( upperLayer )
		{
			if( fErrorMsg->Set( !( fWarned & kWarnedNoUpperTexture ), "Plasma Export Error", sWarnNoUpperTexture, maxNode->GetName() ).CheckAskOrCancel() )
				fWarned |= kWarnedNoUpperTexture; 
			fErrorMsg->Set( false );

			delete plasmaLayer;
			return nil;
		}
		else
		{
			return (plLayerInterface *)plasmaLayer;
		}
	}

	// Setup the texture creation parameters
	plBitmapData bd;
	bd.fileName = bi->Name();

	// Create texture and add it to list if unique
	Int32 texFlags = 0;//hsGTexture::kMipMap;

	// Texture Alpha/Color
	if( bitmapPB->GetInt( kBmpInvertColor ) )
		plasmaLayer->SetBlendFlags( plasmaLayer->GetBlendFlags() | hsGMatState::kBlendInvertColor );
	if( bitmapPB->GetInt( kBmpDiscardColor ) )
		plasmaLayer->SetBlendFlags( plasmaLayer->GetBlendFlags() | hsGMatState::kBlendNoTexColor );
	if( bitmapPB->GetInt( kBmpDiscardAlpha ) )
		plasmaLayer->SetBlendFlags( plasmaLayer->GetBlendFlags() | hsGMatState::kBlendNoTexAlpha );
	if( bitmapPB->GetInt( kBmpInvertAlpha ) )
		bd.invertAlpha = true;

	// Texture quality
	if( bitmapPB->GetInt( kBmpNonCompressed ) )
		texFlags |= plBitmap::kForceNonCompressed;

	if( bitmapPB->GetInt( kBmpNoDiscard ) )
		texFlags |= plBitmap::kDontThrowAwayImage;

	switch( bitmapPB->GetInt( kBmpScaling ) )
	{
		case kScalingHalf: texFlags |= plBitmap::kHalfSize;  break;
		case kScalingNone: texFlags |= plBitmap::kNoMaxSize; break;
	}

	// Mip map filtering.
	if( bitmapPB->GetInt( kBmpNoFilter ) )
		texFlags |= plBitmap::kForceOneMipLevel;
	if( bitmapPB->GetInt( kBmpMipBias ) )
	{
		plasmaLayer->SetZFlags( plasmaLayer->GetZFlags() | hsGMatState::kZLODBias );
		plasmaLayer->SetLODBias( bitmapPB->GetFloat( kBmpMipBiasAmt, fConverterUtils.GetTime( fInterface ) ) );
	}
	float sig = bitmapPB->GetFloat( kBmpMipBlur );

	bd.texFlags = texFlags;
	bd.sig = sig;

	// Get detail parameters
	if( bitmapPB->GetInt( kBmpUseDetail ) )
	{											// TODO: be smarter
		if( blendFlags & hsGMatState::kBlendAdd )
			bd.createFlags |= plMipmap::kCreateDetailAdd;
		else if( blendFlags & hsGMatState::kBlendMult )
			bd.createFlags |= plMipmap::kCreateDetailMult;
		else
			bd.createFlags |= plMipmap::kCreateDetailAlpha;
		bd.detailDropoffStart = float(bitmapPB->GetInt(kBmpDetailStartSize)) / 100.f;
		bd.detailDropoffStop = float(bitmapPB->GetInt(kBmpDetailStopSize)) / 100.f;
		bd.detailMax = float(bitmapPB->GetInt(kBmpDetailStartOpac)) / 100.f;
		bd.detailMin = float(bitmapPB->GetInt(kBmpDetailStopOpac)) / 100.f;
	}

	// Get max export dimension (since the function we eventually call
	// expects the max of the two dimensions, we figure that out here and
	// pass it on)
	bd.maxDimension = bitmapPB->GetInt( kBmpExportWidth );
	int expHt = bitmapPB->GetInt( kBmpExportHeight );
	if( bd.maxDimension < expHt )
		bd.maxDimension = expHt;
	int clipID = 0, w;
	for( clipID = 0, w = bi->Width(); w > bd.maxDimension; w >>= 1, clipID++ );

	// Do the UV gen (before we do texture, since it could modify the bitmapData struct)
	IProcessUVGen( layer, plasmaLayer, &bd, preserveUVOffset );

	// Create the texture.  If it works, assign it to the layer
	if( ( plasmaLayer = IAssignTexture( &bd, maxNode, plasmaLayer, upperLayer, clipID ) ) == nil )
		return nil;

	// All done!
	return (plLayerInterface *)plasmaLayer;

	hsGuardEnd;
}

//// IConvertStaticEnvLayer ///////////////////////////////////////////////////
							
plLayerInterface	*plLayerConverter::IConvertStaticEnvLayer( plPlasmaMAXLayer *layer, 
																plMaxNode *maxNode, UInt32 blendFlags, 
																hsBool preserveUVOffset, hsBool upperLayer )
{
	hsGuardBegin( "plLayerConverter::IConvertStaticEnvLayer" );

	IParamBlock2		*bitmapPB;
	plLocation			loc;

	loc = maxNode->GetLocation();
	bitmapPB = layer->GetParamBlockByID( plStaticEnvLayer::kBlkBitmap );
	
	if( !bitmapPB )
	{
		fErrorMsg->Set( !bitmapPB, "Plasma Layer Error", "Bitmap paramblock for Plasma Layer not found" ).Show();
		fErrorMsg->Set();
		return nil;
	}

	// Get a new layer to play with
	plLayer *plasmaLayer = ICreateLayer( layer->GetName(), upperLayer, loc );

	// Get the texture info
	PBBitmap *pbbm = bitmapPB->GetBitmap( plStaticEnvLayer::kBmpFrontBitmap + 0 );
	BitmapInfo *bi = nil;
	if( pbbm )
		bi = &pbbm->bi;

	// If the texture had bad info, assert and return the empty layer
	if (!bi || !bi->Name() || !strcmp(bi->Name(), ""))
	{
		// Or don't assert since it can get annoying when you are using someone
		// elses file and don't have all the textures.
		return (plLayerInterface *)plasmaLayer;
	}

	// Setup the texture creation parameters
	plBitmapData bd;
	bd.fileName = bi->Name();

	// Create texture and add it to list if unique
	Int32 texFlags = 0;

	// Texture Alpha/Color
	if( bitmapPB->GetInt( plStaticEnvLayer::kBmpInvertColor ) )
		plasmaLayer->SetBlendFlags( plasmaLayer->GetBlendFlags() | hsGMatState::kBlendInvertColor );
	if( bitmapPB->GetInt( plStaticEnvLayer::kBmpDiscardColor ) )
		plasmaLayer->SetBlendFlags( plasmaLayer->GetBlendFlags() | hsGMatState::kBlendNoTexColor );
	if( bitmapPB->GetInt( kBmpDiscardAlpha ) )
		plasmaLayer->SetBlendFlags( plasmaLayer->GetBlendFlags() | hsGMatState::kBlendNoTexAlpha );
	if( bitmapPB->GetInt( plStaticEnvLayer::kBmpInvertAlpha ) )
		bd.invertAlpha = true;

	// Texture quality
	if( bitmapPB->GetInt( plStaticEnvLayer::kBmpNonCompressed ) )
		texFlags |= plBitmap::kForceNonCompressed;

	switch( bitmapPB->GetInt( plStaticEnvLayer::kBmpScaling ) )
	{
		case plStaticEnvLayer::kScalingHalf: texFlags |= plBitmap::kHalfSize;  break;
		case plStaticEnvLayer::kScalingNone: texFlags |= plBitmap::kNoMaxSize; break;
	}

	bd.texFlags = texFlags;
	bd.isStaticCubicEnvMap = true;
	for( int i = 0; i < 6; i++ )
	{
		PBBitmap *face = bitmapPB->GetBitmap( plStaticEnvLayer::kBmpFrontBitmap + i );
		if( !face )
			return (plLayerInterface *)plasmaLayer;
		bd.faceNames[ i ] = face->bi.Name();
	}

	// Get detail parameters
	if( bitmapPB->GetInt( plStaticEnvLayer::kBmpUseDetail ) )
	{											// TODO: be smarter
		if( blendFlags & hsGMatState::kBlendAdd )
			bd.createFlags = plMipmap::kCreateDetailAdd;
		else if( blendFlags & hsGMatState::kBlendMult )
			bd.createFlags = plMipmap::kCreateDetailMult;
		else
			bd.createFlags = plMipmap::kCreateDetailAlpha;

		bd.detailDropoffStart = float( bitmapPB->GetInt( plStaticEnvLayer::kBmpDetailStartSize ) ) / 100.f;
		bd.detailDropoffStop = float( bitmapPB->GetInt( plStaticEnvLayer::kBmpDetailStopSize ) ) / 100.f;
		bd.detailMax = float( bitmapPB->GetInt( plStaticEnvLayer::kBmpDetailStartOpac ) ) / 100.f;
		bd.detailMin = float( bitmapPB->GetInt( plStaticEnvLayer::kBmpDetailStopOpac ) ) / 100.f;
	}

	/// Since we're a cubic environMap, we don't care about the UV transform nor the uvwSrc
	plasmaLayer->SetUVWSrc( 0 );
	plasmaLayer->SetUVWSrc( plasmaLayer->GetUVWSrc() | plLayerInterface::kUVWReflect );

	// Create the texture.  If it works, assign it to the layer
	if( ( plasmaLayer = IAssignTexture( &bd, maxNode, plasmaLayer, upperLayer ) ) == nil )
		return nil;

	// Tag this layer as reflective cubic environmentmapping
	if( bitmapPB->GetInt(plStaticEnvLayer::kBmpRefract) )
		plasmaLayer->SetMiscFlags( plasmaLayer->GetMiscFlags() | hsGMatState::kMiscUseRefractionXform );
	else
		plasmaLayer->SetMiscFlags( plasmaLayer->GetMiscFlags() | hsGMatState::kMiscUseReflectionXform );

	return (plLayerInterface *)plasmaLayer;

	hsGuardEnd;
}

//// IConvertDynamicEnvLayer //////////////////////////////////////////////////
							
plLayerInterface	*plLayerConverter::IConvertDynamicEnvLayer( plPlasmaMAXLayer *layer, 
																plMaxNode *maxNode, UInt32 blendFlags, 
																hsBool preserveUVOffset, hsBool upperLayer )
{
	hsGuardBegin( "plLayerConverter::IConvertDynamicEnvLayer" );

	IParamBlock2		*bitmapPB;
	plLocation			loc;

	loc = maxNode->GetLocation();
	bitmapPB = layer->GetParamBlockByID( plDynamicEnvLayer::kBlkBitmap );
	
	if( !bitmapPB )
	{
		fErrorMsg->Set( !bitmapPB, "Plasma Layer Error", "Bitmap paramblock for Plasma Layer not found" ).Show();
		fErrorMsg->Set();
		return nil;
	}

	// Get a new layer to play with
	plLayer *plasmaLayer = ICreateLayer( layer->GetName(), upperLayer, loc );

	// Get the anchor node
	plMaxNode	*anchor = (plMaxNode *)bitmapPB->GetINode( plDynamicEnvLayer::kBmpAnchorNode );
	if( anchor == nil )
		// Default to self as the anchor--just make sure we make unique versions of this material!
		anchor = maxNode;
	
	if( !anchor->CanConvert() || !( anchor->GetForceLocal() || anchor->GetDrawable() ) )
	{
		fErrorMsg->Set( true, "Plasma Layer Error", "The dynamic envMap material %s has an invalid anchor specified. Please specify a valid Plasma scene object as an anchor.", plasmaLayer->GetKeyName() ).Show();
		fErrorMsg->Set();
		return (plLayerInterface *)plasmaLayer;
	}

	// Create texture and add it to list if unique
	Int32 texFlags = 0;

	/// Since we're a cubic environMap, we don't care about the UV transform nor the uvwSrc
	plasmaLayer->SetUVWSrc( 0 );
	plasmaLayer->SetUVWSrc( plasmaLayer->GetUVWSrc() | plLayerInterface::kUVWReflect );

	// Create the texture.  If it works, assign it to the layer
	char	texName[ 256 ];
	if( anchor == maxNode )
	{
		// Self-anchoring material, make sure the name is unique via the nodeName
		sprintf( texName, "%s_cubicRT@%s", plasmaLayer->GetKeyName(), maxNode->GetName() );
	}
	else
		sprintf( texName, "%s_cubicRT", plasmaLayer->GetKeyName() );

	plBitmap *texture = (plBitmap *)IMakeCubicRenderTarget( texName, maxNode, anchor );
	if( texture )
		hsgResMgr::ResMgr()->AddViaNotify( texture->GetKey(), TRACKED_NEW plLayRefMsg( plasmaLayer->GetKey(), plRefMsg::kOnCreate, 0, plLayRefMsg::kTexture ), plRefFlags::kActiveRef );

	// Tag this layer as reflective cubic environmentmapping
	if( bitmapPB->GetInt(plDynamicEnvLayer::kBmpRefract) )
		plasmaLayer->SetMiscFlags( plasmaLayer->GetMiscFlags() | hsGMatState::kMiscUseRefractionXform );
	else
		plasmaLayer->SetMiscFlags( plasmaLayer->GetMiscFlags() | hsGMatState::kMiscUseReflectionXform );

	return (plLayerInterface *)plasmaLayer;

	hsGuardEnd;
}

plLayerInterface	*plLayerConverter::IConvertCameraLayer(plPlasmaMAXLayer *layer, 
														   plMaxNode *maxNode, UInt32 blendFlags, 
														   hsBool preserveUVOffset, hsBool upperLayer)
{
	hsGuardBegin( "plLayerConverter::IConvertCameraLayer" );

	IParamBlock2		*pb;
	plLocation			loc;

	loc = maxNode->GetLocation();
	pb = layer->GetParamBlockByID(plMAXCameraLayer::kBlkMain);

	if (!pb)
	{
		fErrorMsg->Set(!pb, "Plasma Layer Error", "Paramblock for Plasma Camera Layer not found" ).Show();
		fErrorMsg->Set();
		return nil;
	}

	plLayer *plasmaLayer = ICreateLayer (layer->GetName(), upperLayer, loc);

	plMaxNode *rootNode = (plMaxNode*)pb->GetINode(ParamID(plMAXCameraLayer::kRootNode));
	plDynamicCamMap *map = plEnvMapComponent::GetCamMap(rootNode ? rootNode : maxNode);
	if (map)
	{
		Int32 texFlags = 0;
		if (!pb->GetInt(ParamID(plMAXCameraLayer::kExplicitCam)))
		{
			plasmaLayer->SetUVWSrc(plLayerInterface::kUVWPosition);
			plasmaLayer->SetMiscFlags(hsGMatState::kMiscCam2Screen | hsGMatState::kMiscPerspProjection);
			hsgResMgr::ResMgr()->AddViaNotify(rootNode->GetSceneObject()->GetKey(), TRACKED_NEW plGenRefMsg(map->GetKey(), plRefMsg::kOnCreate, -1, plDynamicCamMap::kRefRootNode), plRefFlags::kActiveRef);
			hsgResMgr::ResMgr()->AddViaNotify(plasmaLayer->GetKey(), TRACKED_NEW plGenRefMsg(map->GetKey(), plRefMsg::kOnCreate, -1, plDynamicCamMap::kRefMatLayer), plRefFlags::kActiveRef);
			if (!pb->GetInt(ParamID(plMAXCameraLayer::kForce)))
			{
				plBitmap *disableTexture = hsMaterialConverter::Instance().GetStaticColorTexture(pb->GetColor(ParamID(plMAXCameraLayer::kDisableColor)), loc);
				hsgResMgr::ResMgr()->AddViaNotify(disableTexture->GetKey(), TRACKED_NEW plGenRefMsg(map->GetKey(), plRefMsg::kOnCreate, -1, plDynamicCamMap::kRefDisableTexture), plRefFlags::kActiveRef);
			}
		}
		else
		{
			plMaxNode *camNode = (plMaxNode*)pb->GetINode(ParamID(plMAXCameraLayer::kCamera));
			if (camNode)
			{
				const plCameraModifier1 *mod = plCameraModifier1::ConvertNoRef(camNode->GetSceneObject()->GetModifierByType(plCameraModifier1::Index()));
				if (mod)
					hsgResMgr::ResMgr()->AddViaNotify(mod->GetKey(), TRACKED_NEW plGenRefMsg(map->GetKey(), plRefMsg::kOnCreate, -1, plDynamicCamMap::kRefCamera), plRefFlags::kActiveRef);
			}

			plasmaLayer->SetUVWSrc(pb->GetInt(ParamID(plMAXCameraLayer::kUVSource)));
		}

		hsTArray<plMaxNode*> nodeList;
		hsMaterialConverter::GetNodesByMaterial(maxNode->GetMtl(), nodeList);
		int i;
		for (i = 0; i < nodeList.GetCount(); i++)
		{
			hsgResMgr::ResMgr()->AddViaNotify(nodeList[i]->GetSceneObject()->GetKey(), TRACKED_NEW plGenRefMsg(map->GetKey(), plRefMsg::kOnCreate, -1, plDynamicCamMap::kRefTargetNode), plRefFlags::kActiveRef);
		}
		hsgResMgr::ResMgr()->AddViaNotify(map->GetKey(), TRACKED_NEW plLayRefMsg(plasmaLayer->GetKey(), plRefMsg::kOnCreate, -1, plLayRefMsg::kTexture), plRefFlags::kActiveRef);

	}

	return plasmaLayer;

	hsGuardEnd;
}

//// IConvertDynamicTextLayer /////////////////////////////////////////////////
							
plLayerInterface	*plLayerConverter::IConvertDynamicTextLayer( plPlasmaMAXLayer *layer, 
																plMaxNode *maxNode, UInt32 blendFlags, 
																hsBool preserveUVOffset, hsBool upperLayer )
{
	hsGuardBegin( "plLayerConverter::IConvertDynamicTextLayer" );

	plDynamicTextLayer	*maxLayer;
	IParamBlock2		*bitmapPB;
	plLocation			loc;

	maxLayer = (plDynamicTextLayer *)layer;
	loc = maxNode->GetLocation();
	bitmapPB = maxLayer->GetParamBlockByID( plDynamicTextLayer::kBlkBitmap );
	
	if( !bitmapPB )
	{
		fErrorMsg->Set( !bitmapPB, "Plasma Layer Error", "Bitmap paramblock for Plasma Layer not found" ).Show();
		fErrorMsg->Set();
		return nil;
	}

	// Get a new layer to play with
	plLayer *plasmaLayer = ICreateLayer( maxLayer->GetName(), upperLayer, loc );


	/// UV Gen
	IProcessUVGen( maxLayer, plasmaLayer, nil, preserveUVOffset );

	// Create the "texture"
	plDynamicTextMap *texture = ICreateDynTextMap( plasmaLayer->GetKeyName(), 
													bitmapPB->GetInt( plDynamicTextLayer::kBmpExportWidth ), 
													bitmapPB->GetInt( plDynamicTextLayer::kBmpExportHeight ),
													bitmapPB->GetInt( plDynamicTextLayer::kBmpIncludeAlphaChannel ),
													maxNode );

	// Set the initial bitmap if necessary
	UInt32 *initBuffer = IGetInitBitmapBuffer( maxLayer );
	if( initBuffer != nil )
	{
		texture->SetInitBuffer( initBuffer );
		delete [] initBuffer;
	}

	// Add the texture in
	hsgResMgr::ResMgr()->AddViaNotify( texture->GetKey(), TRACKED_NEW plLayRefMsg( plasmaLayer->GetKey(), plRefMsg::kOnCreate, 0, plLayRefMsg::kTexture ), plRefFlags::kActiveRef );

	// All done!
	return (plLayerInterface *)plasmaLayer;

	hsGuardEnd;
}

//// IGetInitBitmapBuffer /////////////////////////////////////////////////////
//	Called to get a 32-bit uncompressed ARGB version of the init bitmap of
//	a dynamic text layer

UInt32	*plLayerConverter::IGetInitBitmapBuffer( plDynamicTextLayer *layer ) const
{
	UInt32			*buffer;
	hsRGBAColor32	*buffPtr;
	UInt16			width, height;


	IParamBlock2 *bitmapPB = layer->GetParamBlockByID( plDynamicTextLayer::kBlkBitmap );
	Bitmap		*initBitmap = layer->GetBitmap( TimeValue( 0 ) );

	if( bitmapPB->GetInt( (ParamID)plDynamicTextLayer::kBmpUseInitImage ) == 0 || initBitmap == nil )
		return nil;

	width = bitmapPB->GetInt( (ParamID)plDynamicTextLayer::kBmpExportWidth );
	height = bitmapPB->GetInt( (ParamID)plDynamicTextLayer::kBmpExportHeight );

	buffer = TRACKED_NEW UInt32[ width * height ];
	if( buffer == nil )
		return nil;

	// Fill buffer from the MAX bitmap
	PixelBuf		l64( width );
	BMM_Color_64	*p64 = l64.Ptr();

	buffPtr = (hsRGBAColor32 *)buffer;
	for( int y = 0; y < height; y++ )
	{
		hsRGBAColor32	color;

		if( !initBitmap->GetLinearPixels( 0, y, width, p64 ) )
		{
			delete [] buffer;
			return nil;
		}

		for( int x = 0; x < width; x++ )
		{
			color.SetARGB(  p64[ x ].a * 255.f / 65535.f,
							p64[ x ].r * 255.f / 65535.f,
							p64[ x ].g * 255.f / 65535.f,
							p64[ x ].b * 255.f / 65535.f );
			buffPtr[ x ] = color;
		}

		buffPtr += width;
	}

	return buffer;
}

static UInt32 MakeUInt32Color(float r, float g, float b, float a)
{
	return (UInt32(a * 255.9f) << 24)
			|(UInt32(r * 255.9f) << 16)
			|(UInt32(g * 255.9f) << 8)
			|(UInt32(b * 255.9f) << 0);
}

plBitmap* plLayerConverter::IGetAttenRamp(plMaxNode *node, BOOL isAdd, int loClamp, int hiClamp)
{
	char funkName[512];
	sprintf(funkName, "%s_%d_%d", 
		isAdd ? "AttenRampAdd" : "AttenRampMult",
		loClamp,
		hiClamp);

	float range = float(hiClamp - loClamp) * 1.e-2f;
	float lowest = float(loClamp) * 1.e-2f;
	const int kLUTWidth = 16;
	const int kLUTHeight = 16;

	// NOTE: CreateBlankMipmap might return an old mipmap if it was already created, so in those
	// cases we wouldn't really need to re-write the texture. However, there's no harm in doing so,
	// and since we're close to Alpha, I don't want to shake up the code any more than absolutely 
	// necessary. -mcn
	plMipmap *texture = plBitmapCreator::Instance().CreateBlankMipmap( kLUTWidth, kLUTHeight, plMipmap::kARGB32Config, 1, funkName, node->GetLocation() );

	UInt32* pix = (UInt32*)texture->GetImage();

	if( isAdd )
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
				x *= range;
				x += lowest;
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
				float val = x * y;
				val *= range;
				val += lowest;
				*pix++ = MakeUInt32Color(1.f, 1.f, 1.f, val);
			}
		}
	}

	return texture;
}


plLayer* plLayerConverter::ICreateAttenuationLayer(const char* name, plMaxNode *node, int uvwSrc,
															float tr0, float op0, float tr1, float op1,
															int loClamp, int hiClamp)
{
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

	BOOL chanAdd = false;
	if( op0 < tr0 )
	{
		if( (op0 < op1) && (tr1 < op1) )
			chanAdd = true;
	}
	else if( op0 > tr0 )
	{
		if( (op0 > op1) && (tr1 > op1) )
			chanAdd = true;
	}
	plBitmap* funkRamp = IGetAttenRamp(node, chanAdd, loClamp, hiClamp);

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

	layer->SetUVWSrc(uvwSrc);

	return layer;
}

plLayerInterface* plLayerConverter::IConvertAngleAttenLayer(plPlasmaMAXLayer *layer, 
																plMaxNode *maxNode, UInt32 blendFlags, 
																hsBool preserveUVOffset, hsBool upperLayer)
{
	hsGuardBegin( "plPlasmaMAXLayer::IConvertAngleAttenLayer" );
	if( !upperLayer )
	{
		fErrorMsg->Set(true, maxNode->GetName(), "Angle Attenuation layers can only be used as a top layer").Show();
		fErrorMsg->Set();
		return nil;
	}
	plAngleAttenLayer* aaLay = (plAngleAttenLayer*)layer;
	Box3 fade = aaLay->GetFade();
	float tr0 = cosf(DegToRad(180.f - fade.Min().x));
	float op0 = cosf(DegToRad(180.f - fade.Min().y));
	float tr1 = cosf(DegToRad(180.f - fade.Max().x));
	float op1 = cosf(DegToRad(180.f - fade.Max().y));

	int loClamp = aaLay->GetLoClamp();
	int hiClamp = aaLay->GetHiClamp();

	int uvwSrc = aaLay->Reflect() ? plLayerInterface::kUVWReflect : plLayerInterface::kUVWNormal;

	plLayer* lut = ICreateAttenuationLayer(layer->GetName(), maxNode, uvwSrc, tr0, op0, tr1, op1, loClamp, hiClamp);

	return lut;

	hsGuardEnd;
}
//// ICreateLayer /////////////////////////////////////////////////////////////

plLayer		*plLayerConverter::ICreateLayer( const char *name, hsBool upperLayer, plLocation &loc )
{
	hsGuardBegin( "plPlasmaMAXLayer::ICreateLayer" );

	plLayer	*layer = TRACKED_NEW plLayer;
	layer->InitToDefault();

	hsgResMgr::ResMgr()->NewKey( name, layer, loc );

	return layer;

	hsGuardEnd;
}

//// IProcessUVGen ////////////////////////////////////////////////////////////

void	plLayerConverter::IProcessUVGen( plPlasmaMAXLayer *srcLayer, plLayer *destLayer, 
										plBitmapData *bitmapData, hsBool preserveUVOffset )
{
	hsGuardBegin( "plPlasmaMAXLayer::IProcessUVGen" );

	StdUVGen *uvGen = (StdUVGen *)srcLayer->GetTheUVGen();

	int tiling = uvGen->GetTextureTiling();

	// If set this indicates the texture map is tiled in U
	if (!(tiling & U_WRAP))
	{
		destLayer->SetClampFlags( destLayer->GetClampFlags() | hsGMatState::kClampTextureU );
		if( bitmapData != nil )
			bitmapData->clampFlags |= plBitmapData::kClampU;
	}

	// If set this indicates the texture map is tiled in V
	if (!(tiling & V_WRAP))
	{
		destLayer->SetClampFlags( destLayer->GetClampFlags() | hsGMatState::kClampTextureV );
		if( bitmapData != nil )
			bitmapData->clampFlags |= plBitmapData::kClampV;
	}

	// UVW Src
	Int32 uvwSrc = srcLayer->GetMapChannel() - 1;

	if( fErrorMsg->Set( !( fWarned & kWarnedTooManyUVs ) && 
						( ( uvwSrc < 0 ) || ( uvwSrc >= plGeometrySpan::kMaxNumUVChannels ) ), 
						destLayer->GetKeyName(), "Only %d UVW channels (1-%d) currently supported", 
		plGeometrySpan::kMaxNumUVChannels, plGeometrySpan::kMaxNumUVChannels).CheckAskOrCancel() )
		fWarned |= kWarnedTooManyUVs; 
	fErrorMsg->Set( false );

	destLayer->SetUVWSrc( uvwSrc );

	// Get the actual texture transform
	hsMatrix44 hsTopX;
	if (uvGen && (hsControlConverter::Instance().StdUVGenToHsMatrix44(&hsTopX, uvGen, preserveUVOffset == true)))
		destLayer->SetTransform( hsTopX );

	// All done!
	hsGuardEnd;
}

//// ICreateDynTextMap ////////////////////////////////////////////////////////

plDynamicTextMap	*plLayerConverter::ICreateDynTextMap( const char *layerName, UInt32 width, UInt32 height,
														hsBool includeAlphaChannel, plMaxNode *node )
{
	hsGuardBegin( "plPlasmaMAXLayer::ICreateDynTextMap" );

	char				texName[ 256 ];
	plKey				key;
	plDynamicTextMap	*map = nil;

	
	// Need a unique key name for every layer that uses one. We could also key
	// off of width and height, but layerName should be more than plenty
	sprintf( texName, "%s_dynText", layerName );

	// Does it already exist?
	key = node->FindPageKey( plDynamicTextMap::Index(), texName );
	if( key != nil )
	{
		map = plDynamicTextMap::ConvertNoRef( key->GetObjectPtr() );
		if( map != nil )
			return map;
	}

	// Create
	map = TRACKED_NEW plDynamicTextMap();
	map->SetNoCreate( width, height, includeAlphaChannel );

	/// Add a key for it
	key = hsgResMgr::ResMgr()->NewKey( texName, map, node->GetLocation() );

	// All done!
	return map;

	hsGuardEnd;
}

///////////////////////////////////////////////////////////////////////////////
//// Texture/Bitmap Management ////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

plBitmap *plLayerConverter::CreateSimpleTexture(const char *fileName, const plLocation &loc, 
												UInt32 clipID /* = 0 */, UInt32 texFlags /* = 0 */, bool useJPEG /* = false */)
{
	plBitmapData bd;
	bd.fileName = fileName;
	bd.texFlags = texFlags;
	bd.createFlags = 0;
	bd.detailDropoffStart = 0;
	bd.detailDropoffStop = 0;
	bd.detailMax = 0;
	bd.detailMin = 0;
	bd.sig = 0;
	bd.isStaticCubicEnvMap = false;
	bd.invertAlpha = false;
	bd.clampFlags = 0;
	bd.useJPEG = useJPEG;

	return plBitmapCreator::Instance().CreateTexture(&bd, loc, clipID);
}

//// IAssignTexture ///////////////////////////////////////////////////////////
//	Create a texture and assign it to the layer given. Returns the layer again,
//	or nil if there was an error and it got deleted.

plLayer *plLayerConverter::IAssignTexture( plBitmapData *bd, plMaxNode *maxNode, plLayer *destLayer, hsBool upperLayer, int clipID )
{
	plBitmap *texture = plBitmapCreator::Instance().CreateTexture( bd, maxNode->GetLocation(), clipID );
	if( texture == nil )
	{
		if( upperLayer )
		{
			if( fErrorMsg->Set( !( fWarned & kWarnedUpperTextureMissing ), "Plasma Export Error", sWarnUpperTextureMissing, maxNode->GetName(), bd->fileName ).CheckAskOrCancel() )
				fWarned |= kWarnedUpperTextureMissing; 
			fErrorMsg->Set( false );

			delete destLayer;
			return nil;
		}
		else
		{
			if( fErrorMsg->Set( !( fWarned & kWarnedNoBaseTexture ), "Plasma Export Error", sWarnBaseTextureMissing, maxNode->GetName(), bd->fileName ).CheckAskOrCancel() )
				fWarned |= kWarnedNoBaseTexture; 
			fErrorMsg->Set( false );

			return destLayer;
		}
	}
	else
		hsgResMgr::ResMgr()->AddViaNotify( texture->GetKey(), TRACKED_NEW plLayRefMsg( destLayer->GetKey(), plRefMsg::kOnCreate, 0, plLayRefMsg::kTexture ), plRefFlags::kActiveRef );

	return destLayer;
}

//// IMakeCubicRenderTarget ///////////////////////////////////////////////////
//	Makes a plCubicRenderTarget as a texture. Also constructs the associated
//	modifier and attaches it to the necessary object (hacked for now)

plCubicRenderTarget	*plLayerConverter::IMakeCubicRenderTarget( const char *name, plMaxNode *node, plMaxNode *anchor )
{
	plDynamicEnvMap* env = plEnvMapComponent::GetEnvMap(anchor);
	if( env )
		return env;

	char				modName[ 256 ];
	plCubicRenderTarget	*cubic = nil;


	plKey	key;

	key = node->FindPageKey( plCubicRenderTarget::Index(), name );
	if( key != nil )
	{
		plCubicRenderTarget *cubic = plCubicRenderTarget::ConvertNoRef( key->GetObjectPtr() );
		if( cubic != nil )
			return cubic;
	}

	/// Get the key from the anchor
	if( anchor == nil || anchor->GetSceneObject() == nil )
		return nil;

	plKey	sObjKey = anchor->GetSceneObject()->GetKey();
	if( sObjKey == nil )
		return nil;

	/// Create
	cubic = TRACKED_NEW plCubicRenderTarget( plRenderTarget::kIsTexture, 256, 256, 32 );
	hsAssert( cubic != nil, "Cannot create cubic render target!" );

	/// Add a key
	key = hsgResMgr::ResMgr()->NewKey( name, cubic, node->GetLocation() );

	/// Now make a modifier
	plCubicRenderTargetModifier	*mod = TRACKED_NEW plCubicRenderTargetModifier();
	sprintf( modName, "%s_mod", name );

	hsgResMgr::ResMgr()->NewKey( modName, mod, node->GetLocation() );
	hsgResMgr::ResMgr()->AddViaNotify( cubic->GetKey(), TRACKED_NEW plGenRefMsg( mod->GetKey(), plRefMsg::kOnCreate, 0, 0 ), plRefFlags::kPassiveRef );
	hsgResMgr::ResMgr()->AddViaNotify( mod->GetKey(), TRACKED_NEW plObjRefMsg( sObjKey, plRefMsg::kOnCreate, 0, plObjRefMsg::kModifier ), plRefFlags::kActiveRef );

	return cubic;
}
