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
#include "plComponentProcBase.h"

#include "resource.h"
#include "plComponent.h"
#include "plComponentReg.h"
#include <map>
#include "plAudioComponents.h"
#include "plMiscComponents.h"
#include "plAnimComponent.h"
#include "../plInterp/plAnimEaseTypes.h"
#include "../plAvatar/plAGAnim.h"

#include "../pnSceneObject/plSceneObject.h"
#include "../pnSceneObject/plCoordinateInterface.h"

#include "../MaxConvert/plConvert.h"
#include "../MaxMain/plPluginResManager.h"
#include "../MaxMain/plPlasmaRefMsgs.h"


#include "plgDispatch.h"
#include "../pnMessage/plObjRefMsg.h"
#include "../pnMessage/plIntRefMsg.h"
#include "../pnMessage/plNodeRefMsg.h"


#include "../plScene/plSceneNode.h"
#include "../MaxConvert/hsConverterUtils.h"
#include "../MaxConvert/hsControlConverter.h"
#include "../plInterp/plController.h"
#include "../MaxMain/plMaxNode.h"
#include "../pnKeyedObject/plKey.h"

//Physics Related
//#include "../plHavok1/plHKPhysical.h"			//Physics Comp
#include "../pnSceneObject/plSimulationInterface.h"
#include "../MaxMain/plPhysicalProps.h"
#include "../plPhysX/plPXPhysical.h"

// Sound Related
#include "../plPhysical/plEnvEffectDetector.h"
#include "../pnMessage/plEnvEffectMsg.h"
#include "../PubUtilLib/plAudible/plWinAudible.h"
#include "../pnSceneObject/plAudioInterface.h"

// Anim Related
#include "plMaxAnimUtils.h"
#include "plMaxWaveUtils.h"
#include "../pfAudio/plRandomSoundMod.h"
#include "../plAudio/plWin32StaticSound.h"
#include "../plAudio/plWin32StreamingSound.h"
#include "../plAudio/plWin32GroupedSound.h"
#include "../plAudioCore/plSoundBuffer.h"
#include "../plFile/plFileUtils.h"

// Valdez Asset Manager Related
#include "../../AssetMan/PublicInterface/MaxAssInterface.h"
#include <shlwapi.h>

// Fun soft volume stuff
#include "plSoftVolumeComponent.h"
#include "../plIntersect/plSoftVolume.h"

// Misc
#include "../MaxMain/plMaxCFGFile.h"
#include "plPickNode.h"

// EAX stuff
#include "../plAudio/plEAXListenerMod.h"
#include <eax-util.h>
#include <eaxlegacy.h>

#include "../plResMgr/plLocalization.h"
#include "../plPhysical/plPhysicalSndGroup.h"

// EAX3 values which eax4 no longer defines, but we still need.
// Single window material preset
#define EAX_MATERIAL_SINGLEWINDOW          (-2800)
#define EAX_MATERIAL_SINGLEWINDOWLF        0.71f
#define EAX_MATERIAL_SINGLEWINDOWROOMRATIO 0.43f

// Double window material preset
#define EAX_MATERIAL_DOUBLEWINDOW          (-5000)
#define EAX_MATERIAL_DOUBLEWINDOWLF        0.40f
#define EAX_MATERIAL_DOUBLEWINDOWROOMRATIO 0.24f

// Thin door material preset
#define EAX_MATERIAL_THINDOOR              (-1800)
#define EAX_MATERIAL_THINDOORLF            0.66f
#define EAX_MATERIAL_THINDOORROOMRATIO     0.66f

// Thick door material preset
#define EAX_MATERIAL_THICKDOOR             (-4400)
#define EAX_MATERIAL_THICKDOORLF           0.64f
#define EAX_MATERIAL_THICKDOORROOMRATIO	   0.27f

// Wood wall material preset
#define EAX_MATERIAL_WOODWALL              (-4000)
#define EAX_MATERIAL_WOODWALLLF            0.50f
#define EAX_MATERIAL_WOODWALLROOMRATIO     0.30f

// Brick wall material preset
#define EAX_MATERIAL_BRICKWALL             (-5000)
#define EAX_MATERIAL_BRICKWALLLF           0.60f
#define EAX_MATERIAL_BRICKWALLROOMRATIO    0.24f

// Stone wall material preset
#define EAX_MATERIAL_STONEWALL             (-6000)
#define EAX_MATERIAL_STONEWALLLF           0.68f
#define EAX_MATERIAL_STONEWALLROOMRATIO    0.20f

// Curtain material preset
#define EAX_MATERIAL_CURTAIN               (-1200)
#define EAX_MATERIAL_CURTAINLF             0.15f
#define EAX_MATERIAL_CURTAINROOMRATIO      1.00f

void DummyCodeIncludeFuncAudio() {}


/////////////////////////////////////////////////////////////////////////////////////////////////
/// Base Sound Emitter Component ////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

enum
{
	kSoundFileName, 
	kLoopingChekBx_DEAD,			//Removed in v3
	kLoopBegin_DEAD,				//Removed in v2
	kLoopEnd_DEAD,					//Removed in v2
	kMinFallOffRad_DEAD,			// removed in v6
	kMaxFallOffRad_DEAD,			// removed in v6
	kSoundAutoStartCkBx, 
	kSoundLoopCkBx,
	kSoundLoopSegCkBx_DEAD,			//Removed in v3
	kSoundLoopSegBeg_DEAD,
	kSoundLoopSegEnd_DEAD,
	kSoundLoopSegBegDDList_DEAD,	//Inserted in v3
	kSoundLoopSegEndDDList_DEAD,	//Inserted in v3
	kSoundLoopSegBeg2_DEAD,			//Inserted in v3
	kSoundLoopSegEnd2_DEAD,			//Inserted in v3
	kSFileNameTextField,
	kOldSoundVolumeSlider,			//Inserted in v4 OBLITERATE
	kSoundIConeAngle,				//Inserted in v5
	kSoundOConeAngle,				//Inserted in v5
	kSoundOConeVolumeSlider,		//Inserted in v5
	kMinFallOffRad,
	kMaxFallOffRad,
	kSoundLoopName,
	kSoundConeBool,					//Inserted in v6,
	kNotSoOldSoundVolumeSlider,
	kSndFadeInEnable,
	kSndFadeInType,
	kSndFadeInLength,
	kSndFadeOutEnable,
	kSndFadeOutType,
	kSndFadeOutLength,
	kSndFadedVolume,			// Currently unsupported
	kSndSoftRegion,
	kSndSoftRegionEnable,
	kSndVersionCount,			// So we can do version upgrading (DAMN YOU MAX!!!)
	kSoundVolumeSlider,
	kSndDisableLOD,
	kSndChannelSelect,
	kSndAllowChannelSelect,
	kSndIsWMAFile_DEAD,
	kSndWMAStartClip_DEAD,
	kSndWMAEndClip_DEAD,
	kSndEnableCrossfadeCover_DEAD,
	kSndCrossfadeCoverFilename_DEAD,
	kSndCoverIsWMAFile_DEAD,
	kSndCoverWMAStartClip_DEAD,
	kSndCoverWMAEndClip_DEAD,
	kSndIsLocalOnly,
	kSndCategory,
	kSndPriority,
	kSndIncidental,
	kSndStreamCompressed,
};

enum
{
	kSndFadeTypeLinear,
	kSndFadeTypeLogarithmic,
	kSndFadeTypeExponential
};

UInt32	plBaseSoundEmitterComponent::fWarningFlags = 0;
//bool	plBaseSoundEmitterComponent::fAllowUnhide = false;

void	plBaseSoundEmitterComponent::IShowError( UInt32 type, const char *errMsg, const char *nodeName, plErrorMsg *pErrMsg )
{
	if( !( fWarningFlags & (1 << type) ) )
	{
		if( pErrMsg->Set( true, "Sound Component Error", errMsg, nodeName ).CheckAskOrCancel() )
			fWarningFlags |= (1 << type);
		pErrMsg->Set( false );
	}
}

plBaseSoundEmitterComponent::plBaseSoundEmitterComponent()
{
	fAllowUnhide = false;
	fAssetsUpdated = false;
	fCreateGrouped = false;
	fIndices.clear();
	fValidNodes.clear();
}

plBaseSoundEmitterComponent::~plBaseSoundEmitterComponent()
{
}


RefTargetHandle	plBaseSoundEmitterComponent::Clone( RemapDir &remap )
{
	// Do the base clone
	plBaseSoundEmitterComponent *obj = (plBaseSoundEmitterComponent *)plComponentBase::Clone( remap );

	obj->fSoundAssetId = fSoundAssetId;
	obj->fCoverSoundAssetID = fCoverSoundAssetID;

	return obj;
}

void	plBaseSoundEmitterComponent::IConvertOldVolume( void )
{
	int oldVol = fCompPB->GetInt( (ParamID)kOldSoundVolumeSlider, 0 );
	if( oldVol != 4999 )
	{
		float v = (float)( oldVol - 5000 ) / 5000.f;
		fCompPB->SetValue( (ParamID)kNotSoOldSoundVolumeSlider, 0, v );
		fCompPB->SetValue( (ParamID)kOldSoundVolumeSlider, 0, 4999 );
	}

	// Shut up.
	float notSoOldV = fCompPB->GetFloat( (ParamID)kNotSoOldSoundVolumeSlider, 0 );
	if( notSoOldV != -1.f )
	{
		float d3dValueReally = -5000.f + ( 5000.f * notSoOldV );

		float ourNewValue = (float)d3dValueReally / 100.f;
		
		fCompPB->SetValue( (ParamID)kSoundVolumeSlider, 0, ourNewValue );
		fCompPB->SetValue( (ParamID)kNotSoOldSoundVolumeSlider, 0, -1.f );
	}
}

float	plBaseSoundEmitterComponent::IGetDigitalVolume( void ) const
{
	return (float)pow( 10.f, fCompPB->GetFloat( (ParamID)kSoundVolumeSlider, 0 ) / 20.f );
}

#define OLD_MAX_ASS_CHUNK 0x5500
#define MAX_ASS_CHUNK 0x5501

IOResult plBaseSoundEmitterComponent::Save(ISave *isave) 
{
	IOResult res = plComponentBase::Save(isave);
	if (res != IO_OK)
		return res;

	isave->BeginChunk(MAX_ASS_CHUNK);
	ULONG nwrite;

	UInt64 id = fSoundAssetId;
	res = isave->Write(&id, sizeof(id), &nwrite);
	if (res != IO_OK)
		return res;

	id = fCoverSoundAssetID;
	res = isave->Write(&id, sizeof(id), &nwrite);
	if (res != IO_OK)
		return res;

	isave->EndChunk();

	return IO_OK;
}	

IOResult plBaseSoundEmitterComponent::Load(ILoad *iload) 
{
	IOResult res = plComponentBase::Load(iload);
	if (res != IO_OK)
		return res;

	while (IO_OK == (res = iload->OpenChunk()))
	{
		if (iload->CurChunkID() == OLD_MAX_ASS_CHUNK)
		{
			VARIANT tempVar;
			ULONG nread;
			res = iload->Read(&tempVar, sizeof(VARIANT), &nread);
			fSoundAssetId = tempVar.decVal.Lo64;
		}
		// Secret AssMan value used for no good....
		else if (iload->CurChunkID() == MAX_ASS_CHUNK)
		{
			ULONG nread;
			UInt64 id;
			res = iload->Read(&id, sizeof(id), &nread);
			fSoundAssetId = id;
			res = iload->Read(&id, sizeof(id), &nread);
			fCoverSoundAssetID = id;
		}
		iload->CloseChunk();
		if (res != IO_OK) 
			return res;
	}

	return IO_OK;
}

void plBaseSoundEmitterComponent::SetSoundAssetId( plBaseSoundEmitterComponent::WhichSound which, jvUniqueId assetId, const TCHAR *fileName )
{
	if( which == kBaseSound )
	{
		fSoundAssetId = assetId;

		fCompPB->SetValue( (ParamID)kSoundFileName, 0, (TCHAR *)fileName );
		if( fCompPB->GetMap() )
			fCompPB->GetMap()->Invalidate( (ParamID)kSoundFileName );
	}
	else
	{
		hsAssert( false, "Setting a sound that isn't supported on this component" );
	}
}

jvUniqueId plBaseSoundEmitterComponent::GetSoundAssetID( plBaseSoundEmitterComponent::WhichSound which )
{
	if( which == kCoverSound )
		return fCoverSoundAssetID;
	else if( which == kBaseSound )
		return fSoundAssetId;

	hsAssert( false, "Getting a sound that isn't supported on this component" );
	return fSoundAssetId;
}

void	plBaseSoundEmitterComponent::IUpdateAssets( void )
{
	if( fAssetsUpdated )
		return;

	if( !fSoundAssetId.IsEmpty() || !fCoverSoundAssetID.IsEmpty() )
	{
		MaxAssInterface *maxAssInterface = GetMaxAssInterface();
		if( !maxAssInterface ) 
			return;
		
		// Download the latest version and retrieve the filename
		char newfilename[ MAX_PATH ];
		if(maxAssInterface->GetLatestVersionFile( fSoundAssetId, newfilename, MAX_PATH ) )
		{
			// AssetID overrides filename
			fCompPB->SetValue( (ParamID)kSoundFileName, 0, newfilename );
		}

		fAssetsUpdated = true;
	}
	else
		fAssetsUpdated = true;
}

TCHAR	*plBaseSoundEmitterComponent::GetSoundFileName( plBaseSoundEmitterComponent::WhichSound which )
{
	IUpdateAssets();

	if( which == kBaseSound )
		return fCompPB->GetStr( (ParamID)kSoundFileName );

	hsAssert( false, "Getting a sound that isn't supported on this component" );
	return nil;
}

hsBool plBaseSoundEmitterComponent::DeInit( plMaxNode *node, plErrorMsg *pErrMsg )
{
	fCreateGrouped = false;
	fIndices.clear();
	fValidNodes.clear();
	return true;
}

// Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
hsBool plBaseSoundEmitterComponent::SetupProperties( plMaxNode *pNode, plErrorMsg *pErrMsg )
{
	IConvertOldVolume();
/*
	for (int i = 0; i < fIndices.Count(); i++)
		delete(fIndices[i]);
	fIndices.SetCountAndZero(0);
*/

	return true;
}

void	plBaseSoundEmitterComponent::SetCreateGrouped( plMaxNode *baseNode, int commonSoundIdx )
{
	fIndices[ baseNode ] = commonSoundIdx;
	fCreateGrouped = true;
}

bool plBaseSoundEmitterComponent::IValidate(plMaxNode *node, plErrorMsg *pErrMsg)
{
	if( GetSoundFileName( kBaseSound ) == nil )
	{
		pErrMsg->Set(true, "Sound 3D FileName Error", "The Sound 3D component %s is missing a filename.", node->GetName()).Show();
		pErrMsg->Set(false);
		return false;
	}

	return true;
}

hsBool plBaseSoundEmitterComponent::PreConvert( plMaxNode *node, plErrorMsg *pErrMsg, Class_ID classToConvert )
{
	const char* dbgNodeName = node->GetName();
	fValidNodes[node] = IValidate(node, pErrMsg);
	if (!fValidNodes[node])
		return false;

	node->SetForceLocal(true);

	const plAudioInterface *ai = node->GetSceneObject()->GetAudioInterface();
	if (!ai)
	{
		ai = TRACKED_NEW plAudioInterface;
		plKey pAiKey = hsgResMgr::ResMgr()->NewKey(IGetUniqueName(node), (hsKeyedObject*)ai,node->GetKey()->GetUoid().GetLocation(), node->GetLoadMask());
		hsgResMgr::ResMgr()->AddViaNotify(pAiKey, TRACKED_NEW plObjRefMsg(node->GetKey(), plRefMsg::kOnCreate, 0, plObjRefMsg::kInterface), plRefFlags::kActiveRef);
	}
	if (!ai->GetAudible())
	{
		plAudible *pAudible = TRACKED_NEW plWinAudible;
		// Add a key for it
		plKey key = hsgResMgr::ResMgr()->NewKey(IGetUniqueName(node), pAudible, node->GetKey()->GetUoid().GetLocation(), node->GetLoadMask());
		
		plIntRefMsg* pMsg = TRACKED_NEW plIntRefMsg(node->GetKey(), plRefMsg::kOnCreate, 0, plIntRefMsg::kAudible);
		hsgResMgr::ResMgr()->AddViaNotify(pAudible->GetKey(), pMsg, plRefFlags::kActiveRef );

		pAudible->SetSceneNode(node->GetRoomKey());
	}

	if( !fCreateGrouped )
		fIndices[ node ] = node->GetNextSoundIdx();

	return true;
}

void	plBaseSoundEmitterComponent::IGrabFadeValues( plSound *sound )
{
	if( fCompPB->GetInt( (ParamID)kSndFadeInEnable, 0 ) != 0 )
	{
		// Fade in is enabled; set the params
		plSound::plFadeParams::Type		type;

		hsScalar len = (hsScalar)fCompPB->GetFloat( (ParamID)kSndFadeInLength, 0 );

		switch( fCompPB->GetInt( (ParamID)kSndFadeInType, 0 ) )
		{
			case kSndFadeTypeLinear:		type = plSound::plFadeParams::kLinear; break;
			case kSndFadeTypeLogarithmic:	type = plSound::plFadeParams::kLogarithmic; break;
			case kSndFadeTypeExponential:	type = plSound::plFadeParams::kExponential; break;
		}

		sound->SetFadeInEffect( type, len );
	}

	if( fCompPB->GetInt( (ParamID)kSndFadeOutEnable, 0 ) != 0 )
	{
		// Fade out is enabled; set the params
		plSound::plFadeParams::Type		type;

		hsScalar len = (hsScalar)fCompPB->GetFloat( (ParamID)kSndFadeOutLength, 0 );

		switch( fCompPB->GetInt( (ParamID)kSndFadeOutType, 0 ) )
		{
			case kSndFadeTypeLinear:		type = plSound::plFadeParams::kLinear; break;
			case kSndFadeTypeLogarithmic:	type = plSound::plFadeParams::kLogarithmic; break;
			case kSndFadeTypeExponential:	type = plSound::plFadeParams::kExponential; break;
		}

		sound->SetFadeOutEffect( type, len );
	}

//	sound->SetFadedVolume( (hsScalar)fCompPB->GetFloat( kSndFadedVolume, 0 ) );
}

void	plBaseSoundEmitterComponent::IGrabSoftRegion( plSound *sound, plErrorMsg *pErrMsg )
{
	// Do the soft volume, if there is one
	if( fCompPB->GetInt( (ParamID)kSndSoftRegionEnable, 0 ) != 0 )
	{
		plSoftVolBaseComponent* softComp = plSoftVolBaseComponent::GetSoftComponent( fCompPB->GetINode( (ParamID)kSndSoftRegion ) );
		if( softComp != nil )
		{
			plKey softKey = softComp->GetSoftVolume();
			if( softKey != nil )
			{
				// Make sure we set checkListener on the sucker
				plSoftVolume *vol = plSoftVolume::ConvertNoRef( softKey->GetObjectPtr() );
				if( vol != nil )
				{
					vol->SetCheckListener();
					hsgResMgr::ResMgr()->AddViaNotify( softKey, TRACKED_NEW plGenRefMsg( sound->GetKey(), plRefMsg::kOnCreate, 0, plSound::kSoftRegion ), plRefFlags::kActiveRef );
				}
			}
		}
		else
		{
			pErrMsg->Set(true, "Sound Emitter Error", "The Sound emitter component %s is checked to use a soft region, but no soft region is specified. Ignoring setting.", GetINode()->GetName() ).Show();
			pErrMsg->Set(false);
		}
	}
}

UInt32	plBaseSoundEmitterComponent::ICalcSourceBufferFlags( void ) const
{
	UInt32 bufferFlags = 0;

	if( IHasWaveformProps() )
	{
		if( fCompPB->GetInt( (ParamID)kSndAllowChannelSelect ) )
		{
			if( fCompPB->GetInt( (ParamID)kSndChannelSelect ) )
				bufferFlags = plSoundBuffer::kOnlyRightChannel;
			else
				bufferFlags = plSoundBuffer::kOnlyLeftChannel;
		}
	}

	return bufferFlags;
}

plSoundBuffer	*plBaseSoundEmitterComponent::GetSourceBuffer( const char *fileName, plMaxNode *srcNode, UInt32 srcBufferFlags )
{
	plSoundBuffer* sb = IGetSourceBuffer(fileName, srcNode, srcBufferFlags);

	const char* plasmaDir = plMaxConfig::GetClientPath();
	if (plasmaDir)
	{
		char sfxPath[MAX_PATH];
		sprintf(sfxPath, "%ssfx\\%s", plasmaDir, plFileUtils::GetFileName(fileName));
	
		// Export any localized versions as well
		for (int i = 0; i < plLocalization::GetNumLocales(); i++)
		{
			char localName[MAX_PATH];
			if (plLocalization::ExportGetLocalized(sfxPath, i, localName))
			{
				IGetSourceBuffer(localName, srcNode, srcBufferFlags);
			}
		}
	}

	return sb;
}

plSoundBuffer	*plBaseSoundEmitterComponent::IGetSourceBuffer( const char *fileName, plMaxNode *srcNode, UInt32 srcBufferFlags )
{
	plKey		key;
	char		keyName[ MAX_PATH ];
	char		fullPath[ MAX_PATH ];


	strcpy( keyName, fileName );
	::PathStripPath( keyName );
	
	// TEMP HACK until we get packed sounds: 
	// Given the source filename, we check to see if it's in our plasma game directory. If not, or if
	// it's out of date, we copy it over. We'll truncate the filename inside plSoundBuffer when we're ready.

	const char *plasmaDir = plMaxConfig::GetClientPath();
	if( plasmaDir != nil )
	{
		strcpy( fullPath, plasmaDir );
		strcat( fullPath, "sfx\\" );

		// Before we finish our path, make sure that directory EXISTS
		plFileUtils::CreateDir( fullPath );

		// Now finish the path...
		strcat( fullPath, keyName );

		// Check filestamp
		WIN32_FILE_ATTRIBUTE_DATA	oldFileAttrib, newFileAttrib;
		BOOL						oldOK, newOK;

		oldOK = GetFileAttributesEx( fileName, GetFileExInfoStandard, &oldFileAttrib );
		newOK = GetFileAttributesEx( fullPath, GetFileExInfoStandard, &newFileAttrib );

		if( oldOK && newOK )
		{
			// Only copy if the file is newer
			if( ::CompareFileTime( &oldFileAttrib.ftLastWriteTime, &newFileAttrib.ftLastWriteTime ) > 0 )
			{
	            ::CopyFile( fileName, fullPath, FALSE );
			}
		}
		else
		{
			// Can't compare, so either there was an error or the target file doesn't exist. Copy no matter what.
            ::CopyFile( fileName, fullPath, FALSE );
		}

		// Point to our new sound file
		fileName = fullPath;
	}

	// Additional info for the keyName--need some flag mangling, specifically for the left/right channel mangling
	if( srcBufferFlags & plSoundBuffer::kOnlyLeftChannel )
		strcat( keyName, ":L" );
	else if( srcBufferFlags & plSoundBuffer::kOnlyRightChannel )
		strcat( keyName, ":R" );

	key = srcNode->FindPageKey( plSoundBuffer::Index(), keyName );	
	if( key != nil )
		return plSoundBuffer::ConvertNoRef( key->GetObjectPtr() );

	// Not yet created, so make a new one
	plSoundBuffer	*buffer = TRACKED_NEW plSoundBuffer( fileName, srcBufferFlags );
	if( !buffer->IsValid() )
	{
		// Invalid, so delete and return nil
		delete buffer;
		return nil;
	}

	// We'll put it in a location parallel to the age, say, (age,district,"sounds")
	plLocation	loc = srcNode->GetLocation();
//	plKey roomKey = hsgResMgr::ResMgr()->NameToLoc( loc.GetAge(), loc.GetChapter(), "sounds" );
	// TEMP HACK FOR NOW, until we actually finish implementing this--just hide them in the same file
//	plKey roomKey = hsgResMgr::ResMgr()->FindLocation( loc.GetAge(), loc.GetChapter(), loc.GetPage() );

	// The buffer may be shared across multiple sources. We could or together the LoadMasks of all
	// the nodes that use it, or we can just go with the default loadmask of Always load, and
	// count on it never getting dragged into memory if nothing that references it does.
	hsgResMgr::ResMgr()->NewKey( keyName, buffer, srcNode->GetLocation());

	return buffer;
}

//// LookupLatestAsset ///////////////////////////////////////////////////////
//	Does a find in AssetMan for the given sound file and makes sure it's
//	copied over to the AssetMan directory, returning the path to said asset.
//	Returns true if found, false if not.

hsBool	plBaseSoundEmitterComponent::LookupLatestAsset( const char *waveName, char *retPath, plErrorMsg *errMsg )
{
	MaxAssInterface* assetMan = GetMaxAssInterface();
	if( assetMan == nil )
		return false;		// No AssetMan available

	// Try to find it in assetMan
	jvUniqueId assetId;
	if (assetMan->FindAssetByFilename(waveName, assetId))
	{
		// Get the latest version
		char assetPath[MAX_PATH];
		if (!assetMan->GetLatestVersionFile(assetId, assetPath, sizeof(assetPath)))
		{
			errMsg->Set( true, "SoundEmitter Convert Error",
						"Unable to update wave file '%s' because AssetMan was unable to get the latest version. Using local copy instead.", waveName ).Show();
			errMsg->Set( false );
			return false;
		}

		// Copy the string over and go
		hsStrcpy( retPath, assetPath );
		return true;
	}

	return false;
}

plSoundBuffer	*plBaseSoundEmitterComponent::IProcessSourceBuffer( plMaxNode *maxNode, plErrorMsg *errMsg )
{
	char	*fileName = GetSoundFileName( kBaseSound );
	if( fileName == nil )
		return nil;

	plSoundBuffer *srcBuffer = GetSourceBuffer( fileName, maxNode, ICalcSourceBufferFlags() );
	if( srcBuffer == nil )
	{
		IShowError( kSrcBufferInvalid, "The file specified for the sound 3D component %s is invalid. "
										"This emitter will not be exported.", GetINode()->GetName(), errMsg );
		return nil;
	}

	return srcBuffer;
}

void	plBaseSoundEmitterComponent::UpdateSoundFileSelection( void )
{
	plSoundBuffer	*baseBuffer = nil;


	// Attempt to load the sound in
	if( GetSoundFileName( kBaseSound ) == nil )
	{
		// Disable this feature by default
		fCompPB->SetValue( (ParamID)kSndAllowChannelSelect, 0, 0 );
	}
	else
	{
		if( IAllowStereoFiles() )
		{
			// We allow stereo files, so we don't want to allow stereo->mono select
			fCompPB->SetValue( (ParamID)kSndAllowChannelSelect, 0, 0 );
		}
		else
		{
			baseBuffer = TRACKED_NEW plSoundBuffer( GetSoundFileName( kBaseSound ) );
			if( baseBuffer != nil && baseBuffer->IsValid() )
			{
				// Update our stereo channel selection if necessary
				if( baseBuffer->GetHeader().fNumChannels == 1 )
				{
					fCompPB->SetValue( (ParamID)kSndAllowChannelSelect, 0, 0 );
				}
				else
				{
					fCompPB->SetValue( (ParamID)kSndAllowChannelSelect, 0, 1 );
				}
			}
			else
				// Disable this feature by default
				fCompPB->SetValue( (ParamID)kSndAllowChannelSelect, 0, 0 );
		}
	}

	if( baseBuffer != nil )
		delete baseBuffer;
}

hsScalar	plBaseSoundEmitterComponent::GetSoundVolume( void ) const
{
	return IGetDigitalVolume();
}

//// UpdateCategories ///////////////////////////////////////////////////////////////////////////
// Loads the given combo box with category selections and sets the ParamID for the category parameter.
// Returns false if there are no categories to choose for this component

hsBool	plBaseSoundEmitterComponent::UpdateCategories( HWND dialogBox, int &categoryID, ParamID &paramID )
{
	HWND	comboBox = GetDlgItem( dialogBox, IDC_SND_CATEGORY );
	char	**cats;
	int		*catEnums;
	int		i, currCat, idx, currIdx;


	// Get our list of cats
	if( !IGetCategoryList( cats, catEnums ) )
		return false;

	// We get two categories for this one: Background Music (default) and Ambience
	ComboBox_ResetContent( comboBox );

	currCat = fCompPB->GetInt( (ParamID)kSndCategory );
	currIdx = -1;

	for( i = 0; cats[ i ][ 0 ] != 0; i++ )
	{
		idx = ComboBox_AddString( comboBox, cats[ i ] );
		ComboBox_SetItemData( comboBox, idx, catEnums[ i ] );
		if( catEnums[ i ] == currCat )
			currIdx = idx;
	}

	if( currIdx != -1 )
		ComboBox_SetCurSel( comboBox, currIdx );
	else
	{
		// Option not found in our list, reset to a valid option
		ComboBox_SetCurSel( comboBox, 0 );
		fCompPB->SetValue( (ParamID)kSndCategory, 0, catEnums[ 0 ] );
	}

	// Return info
	paramID = (ParamID)kSndCategory;
	categoryID = IDC_SND_CATEGORY;

	return true;
}

SegmentMap *GetCompWaveSegmentMap(const char *file)
{
	if( file == nil )
		return nil;

	return GetWaveSegmentMap( file, nil );

/*
	const char *path = plMaxConfig::GetClientPath();
	if (file && path)
	{
		char fullpath[MAX_PATH];
		sprintf(fullpath, "%sSfx\\%s", path, file);
		return GetWaveSegmentMap(fullpath, nil);
	}

	return nil;
*/
}

//// ISetBaseParameters /////////////////////////////////////////////////////////////////////////
//	Sets up parameters on the given sound based on the common paramblock values

void	plBaseSoundEmitterComponent::ISetBaseParameters( plSound *destSound, plErrorMsg *pErrMsg )
{
	// Make sure our category is valid before we set it
	int		i, cat = fCompPB->GetInt( (ParamID)kSndCategory );
	char	**cats;
	int		*catEnums;

	if( IGetCategoryList( cats, catEnums ) )
	{
		for( i = 0; cats[ i ][ 0 ] != 0; i++ )
		{
			if( catEnums[ i ] == cat )
				break;
		}
		if( cats[ i ][ 0 ] == 0 )
			cat = catEnums[ 0 ];
	}
	destSound->SetType( cat );

	destSound->SetVolume( IGetDigitalVolume() );
	destSound->SetProperty( plSound::kPropAutoStart, fCompPB->GetInt( (ParamID)kSoundAutoStartCkBx ) );
	IGrabFadeValues( destSound );
	if( fCompPB->GetInt( (ParamID)kSoundLoopCkBx ) )
	{
		destSound->SetProperty( plSound::kPropLooping, true );

		const char *loop = fCompPB->GetStr((ParamID)kSoundLoopName);
		if (loop && loop[0] != '\0')
		{
			SegmentMap *segMap = GetCompWaveSegmentMap( GetSoundFileName( kBaseSound ) );
			if (segMap && segMap->find(loop) != segMap->end())
			{
				SegmentSpec *spec = (*segMap)[loop];
//					sound->SetLoopPoints(spec->fStart, spec->fEnd);
			}
		}
	}
	else
		destSound->SetProperty( plSound::kPropLooping, false );
}

//// AddToAnim //////////////////////////////////////////////////////////////////////////////////
//	Support for animated volumes

hsBool	plBaseSoundEmitterComponent::AddToAnim( plAGAnim *anim, plMaxNode *node )
{
	hsBool result = false;
	plController *ctl;
	hsControlConverter& cc = hsControlConverter::Instance();

	hsScalar start, end;
	if (!strcmp(anim->GetName(), ENTIRE_ANIMATION_NAME))
	{
		start = end = -1;
	}
	else
	{
		start = anim->GetStart();
		end = anim->GetEnd();
	}

	ctl = cc.MakeScalarController( fCompPB->GetController( (ParamID)kSoundVolumeSlider ), node, start, end );
	if( ctl != nil )
	{
		// Better only do this when the sound component is applied to only one object...
		if( fIndices.size() != 1 )
		{
			delete ctl;
			return false;
		}

		std::map<plMaxNode*, int>::iterator i = fIndices.begin();
		plSoundVolumeApplicator *app = TRACKED_NEW plSoundVolumeApplicator( (*i).second );
		app->SetChannelName(node->GetName());
		plAnimComponentBase::SetupCtl( anim, ctl, app, node );
		result = true;		
	}

	return result;
}

//	Class that accesses the paramblock

struct indexinfo
{
	indexinfo::indexinfo() { pNode = nil; fIndex = -1; }
	plMaxNode*	pNode;
	int			fIndex;	
};

int GetSoundNameAndIdx(plComponentBase *comp, plMaxNodeBase *node, const char*& name)
{
	int idx = -1;
	if( ( comp->ClassID() == SOUND_3D_COMPONENT_ID ||
		comp->ClassID() == BGND_MUSIC_COMPONENT_ID ||
		comp->ClassID() == GUI_SOUND_COMPONENT_ID ) && node->CanConvert())
	{
		idx = ((plBaseSoundEmitterComponent *)comp)->GetSoundIdx((plMaxNode*)node);
	}
	if(node->CanConvert())
		name = idx < 0 ? nil : comp->GetINode()->GetName();
	else
		name = nil;
	return idx;
}

int plAudioComp::GetSoundModIdx(plComponentBase *comp, plMaxNode *node)
{
	if( comp->ClassID() == SOUND_3D_COMPONENT_ID ||
		comp->ClassID() == BGND_MUSIC_COMPONENT_ID ||
		comp->ClassID() == GUI_SOUND_COMPONENT_ID )
		return ((plBaseSoundEmitterComponent *)comp)->GetSoundIdx(node);
	return -1;
}

bool	plAudioComp::IsLocalOnly( plComponentBase *comp )
{
	if( comp->ClassID() == SOUND_3D_COMPONENT_ID ||
		comp->ClassID() == BGND_MUSIC_COMPONENT_ID ||
		comp->ClassID() == GUI_SOUND_COMPONENT_ID )
		return ((plBaseSoundEmitterComponent *)comp)->IsLocalOnly() ? true : false;
	return false;
}

bool plAudioComp::IsSoundComponent(plComponentBase *comp)
{
	Class_ID id = comp->ClassID();

	if( id == SOUND_3D_COMPONENT_ID ||
		id == BGND_MUSIC_COMPONENT_ID ||
		id == GUI_SOUND_COMPONENT_ID )
		return true;

	return false;
}

class plAudioBaseComponentProc : public plLCBoxComponentProc
{

protected:

	void	IConvertOldVolume( IParamBlock2 *pb )
	{
		int oldVol = pb->GetInt( (ParamID)kOldSoundVolumeSlider, 0 );
		if( oldVol != 4999 )
		{
			float v = (float)( oldVol - 5000 ) / 5000.f;
			pb->SetValue( (ParamID)kNotSoOldSoundVolumeSlider, 0, v );
			pb->SetValue( (ParamID)kOldSoundVolumeSlider, 0, 4999 );
		}

		// Shut up.
		float notSoOldV = pb->GetFloat( (ParamID)kNotSoOldSoundVolumeSlider, 0 );
		if( notSoOldV != -1.f )
		{
			float d3dValueReally = -5000.f + ( 5000.f * notSoOldV );

			float ourNewValue = (float)d3dValueReally / 100.f;

			pb->SetValue( (ParamID)kSoundVolumeSlider, 0, ourNewValue );
			pb->SetValue( (ParamID)kNotSoOldSoundVolumeSlider, 0, -1.f );
		}
	}

	void	IGetNewLocalFileName( plBaseSoundEmitterComponent *soundComponent, plBaseSoundEmitterComponent::WhichSound which )
	{
		TCHAR	fileName[ MAX_PATH ], dirName[ MAX_PATH ], *name;

	
		name = soundComponent->GetSoundFileName( which );
		if( name != nil )
			strcpy( fileName, name );
		else
			strcpy( fileName, _T( "" ) );

		strcpy( dirName, fileName );
		::PathRemoveFileSpec( dirName );

		OPENFILENAME ofn = {0};
		ofn.lStructSize = sizeof( OPENFILENAME );
		ofn.hwndOwner = GetCOREInterface()->GetMAXHWnd();
		ofn.lpstrFilter = "WAV Files (*.wav)\0*.wav\0Windows Media Audio Files (*.wma)\0*.wma\0OGG Vorbis Files (*.ogg)\0*.ogg\0";
		ofn.lpstrFile = fileName;
		ofn.nMaxFile = sizeof( fileName );
		ofn.lpstrInitialDir = dirName;
		ofn.lpstrTitle = "Choose a sound file";
		ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;
		ofn.lpstrDefExt = "wav";

		if( GetOpenFileName( &ofn ) )
		{
			jvUniqueId emptyId;
			soundComponent->SetSoundAssetId( which, emptyId, fileName );
		}
	}

	void	IUpdateSoundButton( plBaseSoundEmitterComponent *soundComponent, HWND hDlg, int dlgBtnItemToSet, plBaseSoundEmitterComponent::WhichSound which )
	{
		ICustButton			*custButton;
		TCHAR				fileName[ MAX_PATH ];


		custButton = GetICustButton( GetDlgItem( hDlg, dlgBtnItemToSet ) );
		if( custButton != nil )
		{
			TCHAR *origName = soundComponent->GetSoundFileName( which );

			if( origName != nil && strlen( origName ) > 0 )
			{
				strcpy( fileName, origName );
				::PathStripPath( fileName );
				custButton->SetText( fileName );
			}
			else
				custButton->SetText( _T( "<None>" ) );

			ReleaseICustButton( custButton );
		}

		soundComponent->UpdateSoundFileSelection();
	}

	void	ISelectSoundFile( plBaseSoundEmitterComponent *soundComponent, HWND hDlg, int dlgBtnItemToSet, plBaseSoundEmitterComponent::WhichSound which )
	{
		MaxAssInterface* maxAssInterface = GetMaxAssInterface();

		// if we have the assetman plug-in, then try to use it, unless shift is held down
		if( maxAssInterface && !( GetKeyState( VK_SHIFT ) & 0x8000 ) )
		{
			jvUniqueId assetId = soundComponent->GetSoundAssetID(which);

			char fileName[MAX_PATH];
			if (maxAssInterface->OpenSoundDlg(assetId, fileName, MAX_PATH))
			{
				// Set asset ID and filename
				soundComponent->SetSoundAssetId(which, assetId, fileName);
			}
		}
		else
		{
			IGetNewLocalFileName( soundComponent, which );
		}

		// Update the button now
		if( hDlg != nil )
			IUpdateSoundButton( soundComponent, hDlg, dlgBtnItemToSet, which );
	}

	BOOL DlgProc( TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
	{
		plBaseSoundEmitterComponent *soundComp = (plBaseSoundEmitterComponent *)map->GetParamBlock()->GetOwner();


		switch( msg )
		{
			case WM_INITDIALOG:
				CheckDlgButton(hWnd, IDC_SND_TRACKVIEW, soundComp->fAllowUnhide ? BST_CHECKED : BST_UNCHECKED );
				return true;
			
			case WM_COMMAND:
				if( LOWORD( wParam ) == IDC_SND_TRACKVIEW )
				{
					soundComp->fAllowUnhide = ( IsDlgButtonChecked( hWnd, IDC_SND_TRACKVIEW ) == BST_CHECKED );
					plComponentShow::Update();
					return true;
				}
				break;
		}
		
		return false;
	}
};


//// Helper accessors and dialog procs ////

static plSingleCompSelProc gSoundSoftVolumeSelProc( kSndSoftRegion, IDC_COMP_SOUNDREGION_CHOOSE_VOLUME, "Select a soft region to use for the sound" );

// When one of our parameters that is a ref changes, send out the component ref
// changed message.  Normally, messages from component refs are ignored since
// they pass along all the messages of the ref, which generates a lot of false
// converts.
class plSoundSoftVolAccessor : public PBAccessor
{
public:
	void Set( PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t )
	{
		if( id == kSndSoftRegion )
		{
			plBaseSoundEmitterComponent *comp = (plBaseSoundEmitterComponent*)owner;
			comp->NotifyDependents( FOREVER, PART_ALL, REFMSG_USER_COMP_REF_CHANGED );
		}
	}
};

static plSoundSoftVolAccessor gSoundSoftVolAccessor;


enum
{
	kSndSharedParams,
	kS3DBaseParams,
	kS3DSoftVolumeParams,
	kSoundFadeParams,
	kSndWaveformParams,
	kSndEAXParams
};

//// Shared ParamBlock for All Sounds ///////////////////////////////////////////////////////////

#define sSoundSharedPBHeader(s)		kSndSharedParams,	IDD_COMP_SOUNDBASE,			s##,			0, 0, &gSoundCompProc,	\
									kSoundFadeParams,	IDD_COMP_SOUND_FADEPARAMS,	IDS_COMP_SOUNDFADEPARAMS,	0, 0, &gSoundFadeParamsProc

static ParamBlockDesc2 sSoundSharedPB
(
	plComponent::kBlkComp + 2, _T("Sound Shared Params"), 0, nil, P_AUTO_CONSTRUCT + P_AUTO_UI + P_MULTIMAP, plComponent::kRefComp,

	2,	// Number of rollouts
	kSndSharedParams,			IDD_COMP_SOUNDBASE,			IDS_COMP_SOUNDBASE,			0, 0, nil,
	kSoundFadeParams,				IDD_COMP_SOUND_FADEPARAMS,	IDS_COMP_SOUNDFADEPARAMS,	0, 0, nil,

	// params

	/// Version # (currently 0, won't use until we know everyone has paramblocks with this in it)
	kSndVersionCount, _T(""), TYPE_INT, 0, 0,
		p_range, 0, 10000,
		p_default, 0,
		end,
	
	kSoundFileName,		_T("fileName"),		TYPE_STRING, 		0, 0,
		end,
	
	kSoundAutoStartCkBx, _T("autoStart"),		TYPE_BOOL, 		0, 0,
		p_default,	FALSE,
		p_ui,	kSndSharedParams, TYPE_SINGLECHEKBOX, IDC_COMP_SOUND3D_AUTOSTART_CKBX,
		end,
	
	kSoundLoopCkBx, _T("loop"),		TYPE_BOOL, 		0, 0,
		p_default,	FALSE,
		p_ui,	kSndSharedParams, TYPE_SINGLECHEKBOX, IDC_COMP_SOUND3D_LOOPCHKBOX,
		end,
	kSoundLoopName, _T("loopName"),	TYPE_STRING,	0, 0,
		end,

	kSndDisableLOD, _T("disableLOD"),		TYPE_BOOL, 		0, 0,
		p_default,	FALSE,
		p_ui,	kSndSharedParams, TYPE_SINGLECHEKBOX, IDC_COMP_SOUND_DISABLELOD,
		end,

	kSoundVolumeSlider, _T("volume"),	TYPE_FLOAT,	 P_ANIMATABLE,	IDS_SND_VOLUME,
		p_ui,	kSndSharedParams, TYPE_SLIDER,	EDITTYPE_FLOAT, IDC_COMP_SOUND3D_SLIDERVIEWER, IDC_COMP_SOUND3D_VOLSLIDER, 4,
		p_range, -48.0f, 0.f,
		p_default, 0.f,
		end,

	kNotSoOldSoundVolumeSlider, _T(""),	TYPE_FLOAT,		0,	0,
		end,

	kOldSoundVolumeSlider, _T(""),	TYPE_INT,		0,	0,
		end,

	kSndCategory, _T("category"),	TYPE_INT,	0, 0,	
		p_range, plSound::kStartType, plSound::kNumTypes - 1,
		p_default, plSound::kSoundFX,
		end,

	/// Fade Parameters rollout
	kSndFadeInEnable, _T("fadeInEnable"),		TYPE_BOOL, 		0, 0,
		p_default,	FALSE,
		p_ui,	kSoundFadeParams, TYPE_SINGLECHEKBOX, IDC_SOUND3D_INENABLE,
		end,

	kSndFadeInType, _T("fadeInType"),		TYPE_INT, 		0, 0,
		p_default,	0,
		end,

	kSndFadeInLength, _T("fadeInLength"),	TYPE_FLOAT,	0,0,
		p_ui,	kSoundFadeParams, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_SOUND3D_INLENGTH, IDC_SOUND3D_INLENGTHSPIN, 1.0f,
		p_default,	1.f,
		end,

	kSndFadeOutEnable, _T("fadeOutEnable"),		TYPE_BOOL, 		0, 0,
		p_default,	FALSE,
		p_ui,	kSoundFadeParams, TYPE_SINGLECHEKBOX, IDC_SOUND3D_OUTENABLE,
		end,

	kSndFadeOutType, _T("fadeOutType"),		TYPE_INT, 		0, 0,
		p_default,	0,
		end,

	kSndFadeOutLength, _T("fadeOutLength"),	TYPE_FLOAT,	0,0,
		p_ui,	kSoundFadeParams, TYPE_SPINNER, EDITTYPE_FLOAT, IDC_SOUND3D_OUTLENGTH, IDC_SOUND3D_OUTLENGTHSPIN, 1.0f,
		p_default,	1.f,
		end,

	end
);

//// ParamBlock Macros for Waveform Properties Rollout ///////////////////////////////////////////

#define sSndWaveformPropsHeader		kSndWaveformParams, IDD_COMP_SOUNDSRC, IDS_COMP_SOUNDSRC, 0, 0, NULL

#define sSndWaveformPropsParamTemplate \
																											\
	kSndAllowChannelSelect, _T( "allowChannelSelect" ), TYPE_BOOL, 0, 0,									\
		p_default, 0,																						\
		p_ui, kSndWaveformParams, TYPE_SINGLECHEKBOX, IDC_SND_ISSTEREO_HIDDEN,								\
		p_enable_ctrls, 1, kSndChannelSelect,																\
		end,																								\
																											\
	/* Channel select for stereo sources */																	\
	kSndChannelSelect, _T( "sourceChannel" ), TYPE_INT, 0, 0,												\
		p_ui,	kSndWaveformParams, TYPE_RADIO, 2, IDC_SND_CHANSRC1, IDC_SND_CHANSRC2,						\
		p_default, 0,																						\
		end,																								\
																											\
	kSndPriority,			_T("sndPriority"), TYPE_INT, 0, 0,												\
		p_range, 0, 10,																						\
		p_ui,	kSndWaveformParams, TYPE_SPINNER, EDITTYPE_INT, IDC_SND_PRIORITY, IDC_SND_PRIORITY_SPIN, 1.f,	\
		p_default, 0,																						\
		end																								

//// Enums Source EAX Properties Rollout ////////////////////////////////////////////////////////

enum EAXRefs
{
	kEAXEnabled = 128,
	kEAXRoom,
	kEAXRoomHF,
	kEAXRoomAuto,
	kEAXRoomHFAuto,
	kEAXOutsideVolHF,
	kEAXAirAbsorptionFact,
	kEAXRoomRolloffFact,
	kEAXDopplerFact,
	kEAXRolloffFact,

	kEAXEnableOcclusion,
	kEAXOcclusionRegion,
	kEAXStartOcclusion,
	kEAXStartOcclusionLFRatio,
	kEAXStartOcclusionRoomRatio,
	kEAXStartOcclusionDirectRatio,
	kEAXEndOcclusion,
	kEAXEndOcclusionLFRatio,
	kEAXEndOcclusionRoomRatio,
	kEAXEndOcclusionDirectRatio,
	kEAXWhichOccSwapped,
	kEAXTempOcclusion,
	kEAXTempOcclusionLFRatio,
	kEAXTempOcclusionRoomRatio,
	kEAXTempOcclusionDirectRatio,
	kEAXTempOccSwapper
};

//// DialogProc for Source EAX Properties Rollout ///////////////////////////////////////////////

class plEAXPropsDlgProc : public plSingleCompSelProc
{
	IParamBlock2	*fLastBlockSwapped;

	void	ISwapOutOcclusion( IParamBlock2 *pb )
	{
		if( pb == nil )
			return;

		if( pb->GetInt( (ParamID)kEAXWhichOccSwapped ) == 0 )
		{
			// Swap out to start values
			pb->SetValue( (ParamID)kEAXStartOcclusion,				0, pb->GetInt( (ParamID)kEAXTempOcclusion ) );
			pb->SetValue( (ParamID)kEAXStartOcclusionLFRatio,		0, pb->GetFloat( (ParamID)kEAXTempOcclusionLFRatio ) );
			pb->SetValue( (ParamID)kEAXStartOcclusionRoomRatio,		0, pb->GetFloat( (ParamID)kEAXTempOcclusionRoomRatio ) );
			pb->SetValue( (ParamID)kEAXStartOcclusionDirectRatio,	0, pb->GetFloat( (ParamID)kEAXTempOcclusionDirectRatio ) );
		}
		else if( pb->GetInt( (ParamID)kEAXWhichOccSwapped ) == 1 )
		{
			// Swap out to end values
			pb->SetValue( (ParamID)kEAXEndOcclusion,				0, pb->GetInt( (ParamID)kEAXTempOcclusion ) );
			pb->SetValue( (ParamID)kEAXEndOcclusionLFRatio,		0, pb->GetFloat( (ParamID)kEAXTempOcclusionLFRatio ) );
			pb->SetValue( (ParamID)kEAXEndOcclusionRoomRatio,	0, pb->GetFloat( (ParamID)kEAXTempOcclusionRoomRatio ) );
			pb->SetValue( (ParamID)kEAXEndOcclusionDirectRatio,	0, pb->GetFloat( (ParamID)kEAXTempOcclusionDirectRatio ) );
		}

		// Set to "none swapped"
		pb->SetValue( (ParamID)kEAXWhichOccSwapped, 0, (int)-1 );

		fLastBlockSwapped = nil;
	}

	void	ISwapInOcclusion( IParamBlock2 *pb, int which )
	{
		if( pb == nil )
			return;

		if( which == 0 )
		{
			// Swap in from start values
			pb->SetValue( (ParamID)kEAXTempOcclusion,				0, pb->GetInt( (ParamID)kEAXStartOcclusion ) );
			pb->SetValue( (ParamID)kEAXTempOcclusionLFRatio,		0, pb->GetFloat( (ParamID)kEAXStartOcclusionLFRatio ) );
			pb->SetValue( (ParamID)kEAXTempOcclusionRoomRatio,		0, pb->GetFloat( (ParamID)kEAXStartOcclusionRoomRatio ) );
			pb->SetValue( (ParamID)kEAXTempOcclusionDirectRatio,	0, pb->GetFloat( (ParamID)kEAXStartOcclusionDirectRatio ) );
		}
		else
		{
			// Swap in from end values
			pb->SetValue( (ParamID)kEAXTempOcclusion,				0, pb->GetInt( (ParamID)kEAXEndOcclusion ) );
			pb->SetValue( (ParamID)kEAXTempOcclusionLFRatio,		0, pb->GetFloat( (ParamID)kEAXEndOcclusionLFRatio ) );
			pb->SetValue( (ParamID)kEAXTempOcclusionRoomRatio,	0, pb->GetFloat( (ParamID)kEAXEndOcclusionRoomRatio ) );
			pb->SetValue( (ParamID)kEAXTempOcclusionDirectRatio,	0, pb->GetFloat( (ParamID)kEAXEndOcclusionDirectRatio ) );
		}

		pb->SetValue( (ParamID)kEAXWhichOccSwapped, 0, (int)which );
		if( pb->GetMap() != nil )
			pb->GetMap()->UpdateUI( 0 );

		fLastBlockSwapped = pb;
	}

	class plOccPreset
	{
		public:
			char	*fName;
			Int16	fOcc;
			float	fLFRatio;
			float	fRoomRatio;
	};

	plOccPreset	fPresets[ 9 ];

	void	ILoadOccPresets( HWND hDlg )
	{
		HWND	combo = GetDlgItem( hDlg, IDC_EAX_OCCPRESET );
		int		i;

		
		ComboBox_ResetContent( combo );
		for( i = 0; i < 9; i++ )
			ComboBox_AddString( combo, fPresets[ i ].fName );

		ComboBox_SetCurSel( combo, 0 );
	}

public:

	void	FlushSwappedPBs( void )
	{
		if( fLastBlockSwapped != nil )
			ISwapOutOcclusion( fLastBlockSwapped );
	}

	plEAXPropsDlgProc() : plSingleCompSelProc( kEAXOcclusionRegion, IDC_EAX_OCCREGION, "Select the soft region to blend these EAX occlusion properties" )
	{
		int	i;

		// Annoyingly, the EAX headers don't have a convenient array, just some #defines
		static char	occNames[][ 64 ] = { "Single window", "Double window", "Thin door", "Thick door",
									"Wood wall", "Brick wall", "Stone wall", "Curtain" };
		Int16	occValues[] = { EAX_MATERIAL_SINGLEWINDOW, EAX_MATERIAL_DOUBLEWINDOW, EAX_MATERIAL_THINDOOR,
								EAX_MATERIAL_THICKDOOR, EAX_MATERIAL_WOODWALL, EAX_MATERIAL_BRICKWALL,
								EAX_MATERIAL_STONEWALL, EAX_MATERIAL_CURTAIN };
		float	occLFValues[] = { EAX_MATERIAL_SINGLEWINDOWLF, EAX_MATERIAL_DOUBLEWINDOWLF, EAX_MATERIAL_THINDOORLF,
								EAX_MATERIAL_THICKDOORLF, EAX_MATERIAL_WOODWALLLF, EAX_MATERIAL_BRICKWALLLF,
								EAX_MATERIAL_STONEWALLLF, EAX_MATERIAL_CURTAINLF };
		Int16	occRoomValues[] = { EAX_MATERIAL_SINGLEWINDOWROOMRATIO, EAX_MATERIAL_DOUBLEWINDOWROOMRATIO, EAX_MATERIAL_THINDOORROOMRATIO,
								EAX_MATERIAL_THICKDOORROOMRATIO, EAX_MATERIAL_WOODWALLROOMRATIO, EAX_MATERIAL_BRICKWALLROOMRATIO,
								EAX_MATERIAL_STONEWALLROOMRATIO, EAX_MATERIAL_CURTAINROOMRATIO };

		for( i = 1; i < 9; i++ )
		{
			fPresets[ i ].fName = occNames[ i - 1 ];
			fPresets[ i ].fOcc = occValues[ i - 1 ];
			fPresets[ i ].fLFRatio = occLFValues[ i - 1 ];
			fPresets[ i ].fRoomRatio = occRoomValues[ i - 1 ];
		}
		fPresets[ 0 ].fName = "None";
		fPresets[ 0 ].fOcc = 0;
		fPresets[ 0 ].fLFRatio = 0.25f;
		fPresets[ 0 ].fRoomRatio = 1.5f;
	}

	void DeleteThis() {}

	BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		IParamBlock2 *pblock = map->GetParamBlock();

		switch( msg )
		{
			case WM_INITDIALOG:
				pblock->SetValue( (ParamID)kEAXTempOccSwapper, 0, (int)0 );
				ISwapInOcclusion( pblock, 0 );
				ILoadOccPresets( hWnd );
				break;
			
			case WM_DESTROY:
				ISwapOutOcclusion( pblock );
				return 0;

			case WM_SHOWWINDOW:
				if( wParam )
				{
					pblock->SetValue( (ParamID)kEAXTempOccSwapper, 0, (int)0 );
					ISwapInOcclusion( pblock, 0 );
				}
				else
					ISwapOutOcclusion( pblock );
				return 0;

			case WM_COMMAND:
				if( LOWORD( wParam ) == IDC_EAX_STARTOCC || LOWORD( wParam ) == IDC_EAX_ENDOCC )
				{
					// Our radio button to switch between start and end values was hit. So swap out the values
					// from the temp ones
					ISwapOutOcclusion( pblock );
					ISwapInOcclusion( pblock, ( LOWORD( wParam ) == IDC_EAX_STARTOCC ) ? 0 : 1 );
					return true;
				}
				else if( LOWORD( wParam ) == IDC_EAX_OCCPRESET && HIWORD( wParam ) == CBN_SELCHANGE )
				{
					HWND combo = GetDlgItem( hWnd, IDC_EAX_OCCPRESET );
					int idx = ComboBox_GetCurSel( combo );
					if( idx != CB_ERR )
					{
						// Load from presets
						pblock->SetValue( (ParamID)kEAXTempOcclusion, 0, (int)fPresets[ idx ].fOcc );
						pblock->SetValue( (ParamID)kEAXTempOcclusionLFRatio, 0, fPresets[ idx ].fLFRatio );
						pblock->SetValue( (ParamID)kEAXTempOcclusionRoomRatio, 0, fPresets[ idx ].fRoomRatio );
					}
					return true;
				}
				break;
		}	
		return plSingleCompSelProc::DlgProc( t, map, hWnd, msg, wParam, lParam );
	}

};	

static plEAXPropsDlgProc	sEAXPropsDlgProc;

//// ParamBlock for Source EAX Properties Rollout ///////////////////////////////////////////////
//	Note: we can't make this a real ParamBlock and do P_INCLUDE_PARAMS because, in Discreet's 
//	amazing method of doing things, we can't INCLUDE more than one ParamBlock in any other PB.
//	So either we chain them together here (and thus make them dependent on one another, which
//	is lame) or we just make the whole damned thing a #define, which is all P_INCLUDE_PARAMS
//	really does anyway.

#define sSndEAXPropsParamHeader		kSndEAXParams, IDD_COMP_EAXBUFFER, IDS_COMP_EAXBUFFER, 0, APPENDROLL_CLOSED, &sEAXPropsDlgProc
//static ParamBlockDesc2	sSndEAXPropsParamTemplate
//(
	/// Main def
//	plComponent::kBlkComp + 1, _T("sndEAXProps"), 0, nil, P_AUTO_UI + P_MULTIMAP + P_AUTO_CONSTRUCT, plComponent::kRefComp, 

//	1, 
//	kSndEAXParams, IDD_COMP_EAXBUFFER, IDS_COMP_EAXBUFFER, 0, 0, nil,	

#define sSndEAXPropsParamTemplate \
																											\
	kEAXEnabled, _T("eaxEnabled"), TYPE_BOOL, 0, 0,															\
		p_default, 0,																						\
		p_ui, kSndEAXParams, TYPE_SINGLECHEKBOX, IDC_EAX_ENABLE,											\
		p_enable_ctrls, 10, kEAXRoom, kEAXRoomHF, kEAXRoomAuto, kEAXRoomHFAuto,								\
				kEAXOutsideVolHF, kEAXAirAbsorptionFact, kEAXRoomRolloffFact, kEAXDopplerFact, kEAXRolloffFact,			\
				kEAXEnableOcclusion,																		\
		end,																								\
																											\
	kEAXRoom, _T("eaxRoom"),	TYPE_INT,		0,	0,														\
		p_ui,	kSndEAXParams, TYPE_SLIDER,	EDITTYPE_INT, IDC_EAX_ROOM_EDIT, IDC_EAX_ROOM, 8,				\
		p_range, -10000, 1000,																				\
		p_default, 0,																						\
		end,																								\
																											\
	kEAXRoomHF, _T("eaxRoomHF"),	TYPE_INT,		0,	0,													\
		p_ui,	kSndEAXParams, TYPE_SLIDER,	EDITTYPE_INT, IDC_EAX_ROOMHF_EDIT, IDC_EAX_ROOMHF, 8,			\
		p_range, -10000, 0,																					\
		p_default, 0,																						\
		end,																								\
																											\
	kEAXRoomAuto,		_T( "eaxRoomAuto" ), TYPE_BOOL, 0, 0,												\
		p_default, 1,																						\
		p_ui, kSndEAXParams, TYPE_SINGLECHEKBOX, IDC_EAX_ROOMAUTO,											\
		end,																								\
																											\
	kEAXRoomHFAuto,		_T( "eaxRoomHFAuto" ), TYPE_BOOL, 0, 0,												\
		p_default, 1,																						\
		p_ui, kSndEAXParams, TYPE_SINGLECHEKBOX, IDC_EAX_ROOMHFAUTO,										\
		end,																									\
																											\
	kEAXOutsideVolHF, _T("eaxOutsideVolHF"),	TYPE_INT,		0,	0,													\
		p_ui,	kSndEAXParams, TYPE_SLIDER,	EDITTYPE_INT, IDC_EAX_OUTSIDEVOLHF_EDIT, IDC_EAX_OUTSIDEVOLHF, 8,			\
		p_range, -10000, 0,																					\
		p_default, 0,																						\
		end,																								\
																											\
	kEAXAirAbsorptionFact, _T("eaxAirAbsorptionFact"),	TYPE_FLOAT,		0,	0,													\
		p_ui,	kSndEAXParams, TYPE_SLIDER,	EDITTYPE_FLOAT, IDC_EAX_AIRABSORPTFACT_EDIT, IDC_EAX_AIRABSORPTFACT, 8,			\
		p_range, 0.f, 10.f,																					\
		p_default, 1.f,																						\
		end,																								\
																											\
	kEAXRoomRolloffFact, _T("eaxRoomRolloffFact"),	TYPE_FLOAT,		0,	0,													\
		p_ui,	kSndEAXParams, TYPE_SLIDER,	EDITTYPE_FLOAT, IDC_EAX_ROOMROLLOFFFACT_EDIT, IDC_EAX_ROOMROLLOFFFACT, 8,			\
		p_range, 0.f, 10.f,																					\
		p_default, 0.f,																						\
		end,																								\
																											\
	kEAXDopplerFact, _T("eaxDopplerFact"),	TYPE_FLOAT,		0,	0,													\
		p_ui,	kSndEAXParams, TYPE_SLIDER,	EDITTYPE_FLOAT, IDC_EAX_DOPPLERFACT_EDIT, IDC_EAX_DOPPLERFACT, 8,			\
		p_range, 0.f, 10.f,																					\
		p_default, 0.f,																						\
		end,																								\
																											\
	kEAXRolloffFact, _T("eaxRolloffFact"),	TYPE_FLOAT,		0,	0,													\
		p_ui,	kSndEAXParams, TYPE_SLIDER,	EDITTYPE_FLOAT, IDC_EAX_ROLLOFFFACT_EDIT, IDC_EAX_ROLLOFFFACT, 8,			\
		p_range, 0.f, 10.f,																					\
		p_default, 0.f,																						\
		end,																								\
																											\
	kEAXEnableOcclusion, _T("eaxEnableOcclusion"), TYPE_BOOL, 0, 0,											\
		p_default, 0,																						\
		p_ui, kSndEAXParams, TYPE_SINGLECHEKBOX, IDC_EAX_ENABLEOCCLUSION,									\
		p_enable_ctrls, 6, kEAXOcclusionRegion, kEAXTempOcclusion, kEAXTempOcclusionLFRatio, kEAXTempOcclusionRoomRatio,	\
				kEAXTempOcclusionDirectRatio, kEAXTempOccSwapper,											\
		end,																								\
																											\
	kEAXOcclusionRegion, _T("eaxOcclusionRegion"),	TYPE_INODE,		0,	0,										\
		end,																								\
																											\
	kEAXStartOcclusion, _T("eaxStartOcclusion"),	TYPE_INT,		0,	0,													\
		p_range, -10000, 0, p_default, 0,																	\
		end,																								\
																											\
	kEAXStartOcclusionLFRatio, _T("eaxStartOccLFRatio"),	TYPE_FLOAT,		0,	0,													\
		p_range, 0.f, 1.f, p_default, 0.25f,																\
		end,																								\
																											\
	kEAXStartOcclusionRoomRatio, _T("eaxStartOccRoomRatio"),	TYPE_FLOAT,		0,	0,													\
		p_range, 0.f, 10.f, p_default, 1.5f,																\
		end,																								\
																											\
	kEAXStartOcclusionDirectRatio, _T("eaxStartOccDirectRatio"),	TYPE_FLOAT,		0,	0,													\
		p_range, 0.f, 10.f, p_default, 1.0f,																\
		end,																								\
																											\
	kEAXEndOcclusion, _T("eaxEndOcclusion"),	TYPE_INT,		0,	0,													\
		p_range, -10000, 0, p_default, 0,																	\
		end,																								\
																											\
	kEAXEndOcclusionLFRatio, _T("eaxEndOccLFRatio"),	TYPE_FLOAT,		0,	0,													\
		p_range, 0.f, 1.f, p_default, 0.25f,																\
		end,																								\
																											\
	kEAXEndOcclusionRoomRatio, _T("eaxEndOccRoomRatio"),	TYPE_FLOAT,		0,	0,													\
		p_range, 0.f, 10.f, p_default, 1.5f,																\
		end,																								\
																											\
	kEAXEndOcclusionDirectRatio, _T("eaxEndOccDirectRatio"),	TYPE_FLOAT,		0,	0,													\
		p_range, 0.f, 10.f, p_default, 1.0f,																\
		end,																								\
																											\
	kEAXWhichOccSwapped, _T("eaxWhichOccSwapped"),	TYPE_INT,		0,	0,													\
		end,																								\
																											\
	kEAXTempOccSwapper, _T("eaxOccSwapper"),	TYPE_INT,		0,	0,										\
		p_ui, kSndEAXParams, TYPE_RADIO, 2, IDC_EAX_STARTOCC, IDC_EAX_ENDOCC,								\
		p_default, 0,																\
		end,																								\
																											\
	kEAXTempOcclusion, _T("eaxTempOcclusion"),	TYPE_INT,		0,	0,										\
		p_ui,	kSndEAXParams, TYPE_SLIDER,	EDITTYPE_INT, IDC_EAX_OCCLUSION_EDIT, IDC_EAX_OCCLUSION, 8,		\
		p_range, -10000, 0, p_default, 0,																	\
		end,																								\
																											\
	kEAXTempOcclusionLFRatio, _T("eaxTempOccLFRatio"),	TYPE_FLOAT,		0,	0,													\
		p_ui,	kSndEAXParams, TYPE_SLIDER,	EDITTYPE_FLOAT, IDC_EAX_OCCLFRATIO_EDIT, IDC_EAX_OCCLFRATIO, 8,			\
		p_range, 0.f, 1.f, p_default, 0.25f,																\
		end,																								\
																											\
	kEAXTempOcclusionRoomRatio, _T("eaxTempOccRoomRatio"),	TYPE_FLOAT,		0,	0,													\
		p_ui,	kSndEAXParams, TYPE_SLIDER,	EDITTYPE_FLOAT, IDC_EAX_OCCROOMRATIO_EDIT, IDC_EAX_OCCROOMRATIO, 8,			\
		p_range, 0.f, 10.f, p_default, 1.5f,																\
		end,																								\
																											\
	kEAXTempOcclusionDirectRatio, _T("eaxTempOccDirectRatio"),	TYPE_FLOAT,		0,	0,													\
		p_ui,	kSndEAXParams, TYPE_SLIDER,	EDITTYPE_FLOAT, IDC_EAX_OCCDIRECTRATIO_EDIT, IDC_EAX_OCCDIRECTRATIO, 8,			\
		p_range, 0.f, 10.f, p_default, 1.0f,																\
		end																								

//	, end
//);


void	plBaseSoundEmitterComponent::IGrabEAXParams( plSound *sound, plErrorMsg *pErrMsg )
{
	sEAXPropsDlgProc.FlushSwappedPBs();

	plEAXSourceSettings &settings = sound->GetEAXSettings();

	if( fCompPB->GetInt( (ParamID)kEAXEnabled ) )
	{
		settings.Enable( true );
		settings.SetRoomParams( fCompPB->GetInt( (ParamID)kEAXRoom ), fCompPB->GetInt( (ParamID)kEAXRoomHF ),
								fCompPB->GetInt( (ParamID)kEAXRoomAuto ), fCompPB->GetInt( (ParamID)kEAXRoomHFAuto ) );
		settings.SetOutsideVolHF( fCompPB->GetInt( (ParamID)kEAXOutsideVolHF ) );
		settings.SetFactors( fCompPB->GetFloat( (ParamID)kEAXAirAbsorptionFact ), 
							fCompPB->GetFloat( (ParamID)kEAXRoomRolloffFact ),
							fCompPB->GetFloat( (ParamID)kEAXDopplerFact ),
							fCompPB->GetFloat( (ParamID)kEAXRolloffFact ) );

		if( fCompPB->GetInt( (ParamID)kEAXEnableOcclusion ) )
		{
			settings.GetSoftStarts().SetOcclusion( fCompPB->GetInt( (ParamID)kEAXStartOcclusion ),
													fCompPB->GetFloat( (ParamID)kEAXStartOcclusionLFRatio ),
													fCompPB->GetFloat( (ParamID)kEAXStartOcclusionRoomRatio ),
													fCompPB->GetFloat( (ParamID)kEAXStartOcclusionDirectRatio ) );
			settings.GetSoftEnds().SetOcclusion( fCompPB->GetInt( (ParamID)kEAXEndOcclusion ),
													fCompPB->GetFloat( (ParamID)kEAXEndOcclusionLFRatio ),
													fCompPB->GetFloat( (ParamID)kEAXEndOcclusionRoomRatio ),
													fCompPB->GetFloat( (ParamID)kEAXEndOcclusionDirectRatio ) );

			plSoftVolBaseComponent* softComp = plSoftVolBaseComponent::GetSoftComponent( fCompPB->GetINode( (ParamID)kEAXOcclusionRegion ) );
			if( softComp != nil )
			{
				plKey softKey = softComp->GetSoftVolume();
				if( softKey != nil )
				{
					// Make sure we set checkListener on the sucker
					plSoftVolume *vol = plSoftVolume::ConvertNoRef( softKey->GetObjectPtr() );
					if( vol != nil )
					{
						vol->SetCheckListener();
						hsgResMgr::ResMgr()->AddViaNotify( softKey, TRACKED_NEW plGenRefMsg( sound->GetKey(), plRefMsg::kOnCreate, 0, plSound::kRefSoftOcclusionRegion ), plRefFlags::kActiveRef );
					}
				}
			}
			else
			{
				pErrMsg->Set(true, "Sound Emitter Error", "The Sound emitter component %s is checked to use an occlusion soft region, but no soft region is specified. Ignoring setting.", GetINode()->GetName() ).Show();
				pErrMsg->Set(false);
				settings.GetSoftStarts().Reset();
				settings.GetSoftEnds().Reset();
			}
		}
		else
		{
			settings.GetSoftStarts().Reset();
			settings.GetSoftEnds().Reset();
		}
	}
	else
		settings.Enable( false );
}


/////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SoundEmitter Component
//
//

/*
class plBaseComponentProc : public ParamMap2UserDlgProc
{
protected:
	void ILoadComboBox(HWND hComboBox, const char *names[])
	{
		SendMessage(hComboBox, CB_RESETCONTENT, 0, 0);
		for (int i = 0; names[i]; i++)
			SendMessage(hComboBox, CB_ADDSTRING, 0, (LPARAM)names[i]);
		SendMessage(hComboBox, CB_SETCURSEL, 0, 0);
	}

	void ILoadListBox(HWND hListBox, IParamBlock2 *pb, int param, const char *names[])
	{
		SendMessage(hListBox, LB_RESETCONTENT, 0, 0);
		for (int i = 0; i < pb->Count(param); i++)
		{
			int idx = pb->GetInt(param, 0, i);
			SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)names[idx]);
		}
	}

	void	IConvertOldVolume( IParamBlock2 *pb )
	{
		int oldVol = pb->GetInt( kOldSoundVolumeSlider, 0 );
		if( oldVol != 4999 )
		{
			float v = (float)( oldVol - 5000 ) / 5000.f;
			pb->SetValue( kSoundVolumeSlider, 0, v );
			pb->SetValue( kOldSoundVolumeSlider, 0, 4999 );
		}
	}

};
*/
class plSound3DEmitterComponent : public plBaseSoundEmitterComponent
{
public:
	plSound3DEmitterComponent();
	virtual ~plSound3DEmitterComponent();

	// Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	hsBool SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg);

	hsBool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg);
	hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);

	virtual hsBool	IsLocalOnly( void ) const { if( fCompPB->GetInt( (ParamID)kSndIsLocalOnly ) ) return true; else return false; }


	virtual hsBool	ConvertGrouped( plMaxNode *baseNode, hsTArray<plBaseSoundEmitterComponent *> &groupArray, plErrorMsg *pErrMsg );

protected:

	bool IValidate(plMaxNode *node, plErrorMsg *pErrMsg);

	virtual hsBool	IAllowStereoFiles( void ) const { return false; }

	void	ISetParameters( plWin32Sound *destSound, plErrorMsg *pErrMsg );

	virtual hsBool	IGetCategoryList( char **&catList, int *&catKonstantList );
};

class plSoundComponentProc : public plAudioBaseComponentProc
{
	hsBool	fHandleCategory;
	int		fCategoryCtrlID;
	ParamID	fCategoryParamID;

public:
	void DeleteThis() {}

	void ILoadLoops(HWND hLoop, IParamBlock2 *pb)
	{
		SendMessage(hLoop, CB_RESETCONTENT, 0, 0);
		SendMessage(hLoop, CB_ADDSTRING, 0, (LPARAM)"(Entire Sound)");

		const char *loop = pb->GetStr(kSoundLoopName);
		if (!loop)
			loop = "";

		SegmentMap *segMap = GetCompWaveSegmentMap(pb->GetStr(kSoundFileName));
		
		if (segMap)
		{
			for (SegmentMap::iterator it = segMap->begin(); it != segMap->end(); it++)
			{
				SegmentSpec *spec = it->second;
				int idx = SendMessage(hLoop, CB_ADDSTRING, 0, (LPARAM)spec->fName);
				SendMessage(hLoop, CB_SETITEMDATA, idx, 1);

				if (!strcmp(spec->fName, loop))
					SendMessage(hLoop, CB_SETCURSEL, idx, 0);
			}

			
			DeleteSegmentMap(segMap);
		}

		if (SendMessage(hLoop, CB_GETCURSEL, 0, 0) == CB_ERR)
			SendMessage(hLoop, CB_SETCURSEL, 0, 0);
	}

	BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch( msg )
		{
			case WM_INITDIALOG:
				{
					IParamBlock2 *pblock = map->GetParamBlock();

					plSound3DEmitterComponent *soundComp = (plSound3DEmitterComponent *)map->GetParamBlock()->GetOwner();
					hsAssert( soundComp != nil, "Problem trying to select a sound file" );

					IUpdateSoundButton( soundComp, hWnd, IDC_COMP_SOUND3D_FILENAME_BTN, plBaseSoundEmitterComponent::kBaseSound );
					
					IConvertOldVolume( pblock );

					fHandleCategory = soundComp->UpdateCategories( hWnd, fCategoryCtrlID, fCategoryParamID );
				}
				
				{
					ILoadLoops(GetDlgItem(hWnd, IDC_LOOP_COMBO), map->GetParamBlock());
	#if 0
					map->SetTooltip(kSoundFileName,			true, _T("A sound file name."));
					map->SetTooltip(kLoopBegin,				true, _T("The distance, in feet, at which the sound begins to be less audible."));
					map->SetTooltip(kLoopEnd,				true, _T("The distance, in feet, at which the sound is no longer audible."));
					map->SetTooltip(kSoundLoopSegBeg2,		true, _T("The distance, in feet, at which the sound begins to be less audible."));
					map->SetTooltip(kSoundLoopSegEnd2,		true, _T("The distance, in feet, at which the sound is no longer audible."));
					map->SetTooltip(kSoundAutoStartCkBx,	true, _T("Check to play the sound file upon game start."));
					map->SetTooltip(kSoundLoopSegBegDDList,	true, _T("The time, keyframe or percentage at which looping is to begin."));
					map->SetTooltip(kSoundLoopSegEndDDList,	true, _T("The time, keyframe or percentage at which looping is to end."));
	#endif
					
					
					break;
				}
			
		case WM_COMMAND:
			if (HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == IDC_LOOP_COMBO)
			{
				int idx = SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0);
				if (idx == CB_ERR || SendMessage((HWND)lParam, CB_GETITEMDATA, idx, 0) == 0)
					map->GetParamBlock()->SetValue((ParamID)kSoundLoopName, 0, "");
				else
				{
					char buf[256];
					SendMessage((HWND)lParam, CB_GETLBTEXT, idx, (LPARAM)buf);
					map->GetParamBlock()->SetValue((ParamID)kSoundLoopName, 0, buf);
				}
				return true;
			}
			else if( LOWORD( wParam ) == IDC_COMP_SOUND3D_FILENAME_BTN )
			{
				plSound3DEmitterComponent *soundComp = (plSound3DEmitterComponent *)map->GetParamBlock()->GetOwner();
				hsAssert( soundComp != nil, "Problem trying to select a sound file" );
				
				ISelectSoundFile( soundComp, hWnd, IDC_COMP_SOUND3D_FILENAME_BTN, plBaseSoundEmitterComponent::kBaseSound );
			}
			else if( fHandleCategory && LOWORD( wParam ) == fCategoryCtrlID )
			{
				HWND ctrl = GetDlgItem( hWnd, fCategoryCtrlID );
				int idx = ComboBox_GetCurSel( ctrl );
				if( idx != CB_ERR )
				{
					int cat = ComboBox_GetItemData( ctrl, idx );
					map->GetParamBlock()->SetValue( (ParamID)fCategoryParamID, 0, cat );
				}
				else
					map->GetParamBlock()->SetValue( (ParamID)fCategoryParamID, 0, (int)0 );
			}
			break;
		}
		
		return plAudioBaseComponentProc::DlgProc( t, map, hWnd, msg, wParam, lParam );
	}
	
};	

class plSoundFadeParamsDlgProc : public plAudioBaseComponentProc
{
	public:
		void DeleteThis() {}

		BOOL DlgProc( TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
		{
			const char			*types[] = { "Linear", "Logarithmic", "Exponential", NULL };
			IParamBlock2	*pb = map->GetParamBlock();
			BOOL			enable;


			switch( msg )
			{
				case WM_INITDIALOG:
					// Load fade types

					ILoadComboBox( GetDlgItem( hWnd, IDC_SOUND3D_INTYPE ), types );
					ILoadComboBox( GetDlgItem( hWnd, IDC_SOUND3D_OUTTYPE ), types );

					SendDlgItemMessage( hWnd, IDC_SOUND3D_INTYPE, CB_SETCURSEL, (WPARAM)pb->GetInt( (ParamID)kSndFadeInType, 0 ), 0 );
					SendDlgItemMessage( hWnd, IDC_SOUND3D_OUTTYPE, CB_SETCURSEL, (WPARAM)pb->GetInt( (ParamID)kSndFadeOutType, 0 ), 0 );

					enable = pb->GetInt( (ParamID)kSndFadeInEnable, 0 ) ? TRUE : FALSE;
					EnableWindow( GetDlgItem( hWnd, IDC_SOUND3D_INTYPE ), enable );
					EnableWindow( GetDlgItem( hWnd, IDC_SOUND3D_INLENGTH ), enable );

					enable = pb->GetInt( (ParamID)kSndFadeOutEnable, 0 ) ? TRUE : FALSE;
					EnableWindow( GetDlgItem( hWnd, IDC_SOUND3D_OUTTYPE ), enable );
					EnableWindow( GetDlgItem( hWnd, IDC_SOUND3D_OUTLENGTH ), enable );

					break;

				case WM_COMMAND:
					if( HIWORD( wParam ) == CBN_SELCHANGE )
					{ 
						if( LOWORD( wParam ) == IDC_SOUND3D_INTYPE )
							pb->SetValue( (ParamID)kSndFadeInType, 0, (int)SendDlgItemMessage( hWnd, IDC_SOUND3D_INTYPE, CB_GETCURSEL, 0, 0 ) );
						else if( LOWORD( wParam ) == IDC_SOUND3D_OUTTYPE )
							pb->SetValue( (ParamID)kSndFadeOutType, 0, (int)SendDlgItemMessage( hWnd, IDC_SOUND3D_OUTTYPE, CB_GETCURSEL, 0, 0 ) );
					}

					else if( LOWORD( wParam ) == IDC_SOUND3D_INENABLE )
					{
						// Enable/disable controls manually
						enable = pb->GetInt( (ParamID)kSndFadeInEnable, 0 ) ? TRUE : FALSE;
						EnableWindow( GetDlgItem( hWnd, IDC_SOUND3D_INTYPE ), enable );
						EnableWindow( GetDlgItem( hWnd, IDC_SOUND3D_INLENGTH ), enable );
					}
					else if( LOWORD( wParam ) == IDC_SOUND3D_OUTENABLE )
					{
						// Enable/disable controls manually
						enable = pb->GetInt( (ParamID)kSndFadeOutEnable, 0 ) ? TRUE : FALSE;
						EnableWindow( GetDlgItem( hWnd, IDC_SOUND3D_OUTTYPE ), enable );
						EnableWindow( GetDlgItem( hWnd, IDC_SOUND3D_OUTLENGTH ), enable );
					}
					break;

			}

			return false;
		}
};	

//  For the paramblock below.
static plSoundComponentProc		gSoundCompProc;
static plSoundFadeParamsDlgProc	gSoundFadeParamsProc;

//Max desc stuff necessary below.
CLASS_DESC(plSound3DEmitterComponent, gSound3DEmitterDesc, "Sound 3D",  "Sound3D", COMP_TYPE_AUDIO, SOUND_3D_COMPONENT_ID)

ParamBlockDesc2 gSound3DEmitterBk
(
	plComponent::kBlkComp, _T("3D Sound"), 0, &gSound3DEmitterDesc, P_AUTO_CONSTRUCT + P_AUTO_UI + P_MULTIMAP + P_INCLUDE_PARAMS, plComponent::kRefComp,

	6,	// Number of rollouts
	sSoundSharedPBHeader(IDS_COMP_SOUNDBASE),
	kS3DBaseParams,			IDD_COMP_SOUND3D,			IDS_COMP_SOUND3DS,			0, 0, &gSoundCompProc,
	kS3DSoftVolumeParams,	IDD_COMP_SOUND_SOFTPARAMS,	IDS_COMP_SOUNDSOFTPARAMS,	0, 0, &gSoundSoftVolumeSelProc,
	sSndWaveformPropsHeader,
	sSndEAXPropsParamHeader,

	// Included paramblock
	&sSoundSharedPB,

	// Waveform props define
	sSndWaveformPropsParamTemplate,

	// params

	kSndIsLocalOnly, _T("noNetworkSynch"),		TYPE_BOOL, 		0, 0,
		p_default,	FALSE,
		p_ui,	kS3DBaseParams, TYPE_SINGLECHEKBOX, IDC_SND_LOCALONLY,
		end,
	

	kMinFallOffRad, _T("minFalloff"),	TYPE_INT,	0, 0,	
		p_range, 1, 1000000000,
		p_default, 1,
		p_ui,	kS3DBaseParams, TYPE_SPINNER,	EDITTYPE_POS_INT,
		IDC_COMP_SOUND3D_EDIT3,	IDC_COMP_SOUND3D_SPIN3, SPIN_AUTOSCALE,
		end,

	kMaxFallOffRad, _T("maxFalloff"),	TYPE_INT,	0, 0,	
		p_range, 1, 1000000000,
		p_default, 1000000000,
		p_ui,	kS3DBaseParams, TYPE_SPINNER,	EDITTYPE_POS_INT,
		IDC_COMP_SOUND3D_EDIT4,	IDC_COMP_SOUND3D_SPIN4, SPIN_AUTOSCALE,
		end,

	kSoundConeBool, _T("SoundCone"),		TYPE_BOOL, 		0, 0,
		p_default,	FALSE,
		p_enable_ctrls, 3, kSoundIConeAngle, kSoundOConeAngle, kSoundOConeVolumeSlider,
		p_ui,	kS3DBaseParams, TYPE_SINGLECHEKBOX, IDC_COMP_SOUND3D_CONEEFFECT_CKBX,
		end,


	kSoundIConeAngle,	_T("insideConeAngle"), TYPE_INT,	0,	0,
		p_range, 0, 360,
		p_default, 360,
		p_ui,	kS3DBaseParams, TYPE_SPINNER, EDITTYPE_INT,
		IDC_COMP_SOUND3D_ICONEANGLE_EDIT, IDC_COMP_SOUND3D_ICONEANGLE_SPIN, 1.0f,
		end,

	kSoundOConeAngle,	_T("outsideConeAngle"), TYPE_INT,	0,	0,
		p_range, 0, 360,
		p_default, 360,
		p_ui,	kS3DBaseParams, TYPE_SPINNER, EDITTYPE_INT,
		IDC_COMP_SOUND3D_OCONEANGLE_EDIT, IDC_COMP_SOUND3D_OCONEANGLE_SPIN, 1.0f,
		end,
		
	kSoundOConeVolumeSlider, _T("outsideConeVolSlider"),	TYPE_INT,		0,	0,
		p_ui,	kS3DBaseParams, TYPE_SLIDER,	EDITTYPE_INT, IDC_COMP_SOUND3D_SLIDERVIEWER2, IDC_COMP_SOUND3D_VOLSLIDER2, 4,
		p_range, 5000,	10000,
		p_default, 5000,
		end,

	/// Soft Region/Volume Parameters rollout
	kSndSoftRegionEnable,	_T( "enableSoftRegion" ), TYPE_BOOL, 0, 0,
		p_ui,	kS3DSoftVolumeParams, TYPE_SINGLECHEKBOX, IDC_SOUND_SOFTENABLE,
		p_default, FALSE,
		end,

	kSndSoftRegion, _T("softRegion"),	TYPE_INODE,		0, 0,
		p_prompt, IDS_COMP_SOUNDSOFTSELECT,
		p_accessor, &gSoundSoftVolAccessor,
		end,
				
	kSndIncidental, _T("isIncidental"), TYPE_INT, 0, 0,
		p_default,	FALSE,
		p_ui,	kS3DBaseParams, TYPE_SINGLECHEKBOX, IDC_SND_INCIDENTAL,
		end,
		
	sSndEAXPropsParamTemplate,	// it's a #define

	end
);


plSound3DEmitterComponent::plSound3DEmitterComponent()
{
	fClassDesc = &gSound3DEmitterDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

plSound3DEmitterComponent::~plSound3DEmitterComponent()
{
}

//// IGetCategoryList ///////////////////////////////////////////////////////////////////////////
//	Returns a list of the categories and konstants supported for this type of sound

hsBool	plSound3DEmitterComponent::IGetCategoryList( char **&catList, int *&catKonstantList )
{
	static char	*cats[] = { "Background Music", "Ambience", "Sound FX", "GUI", "NPC Voice", "" };
	static int	catEnums[] = { plSound::kBackgroundMusic, plSound::kAmbience, plSound::kSoundFX, plSound::kGUISound, plSound::kNPCVoices };

	catList = cats;
	catKonstantList = catEnums;

	return true;
}

// Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
hsBool plSound3DEmitterComponent::SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg)
{
	return plBaseSoundEmitterComponent::SetupProperties( pNode, pErrMsg );
}

bool plSound3DEmitterComponent::IValidate(plMaxNode *node, plErrorMsg *pErrMsg)
{
	return plBaseSoundEmitterComponent::IValidate( node, pErrMsg );
}

hsBool plSound3DEmitterComponent::PreConvert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	return plBaseSoundEmitterComponent::PreConvert( node, pErrMsg, SOUND_3D_COMPONENT_ID );
}

void	plSound3DEmitterComponent::ISetParameters( plWin32Sound *destSound, plErrorMsg *pErrMsg )
{
	ISetBaseParameters( destSound, pErrMsg );

	int min = fCompPB->GetInt( (ParamID)kMinFallOffRad );
	int max = fCompPB->GetInt( (ParamID)kMaxFallOffRad );
	float Vol = IGetDigitalVolume();

	int OutVol, innerCone, outerCone;
	
	if( fCompPB->GetInt( (ParamID)kSoundConeBool ) )
	{
		OutVol = fCompPB->GetInt( (ParamID)kSoundOConeVolumeSlider );
		innerCone = fCompPB->GetInt( (ParamID)kSoundIConeAngle );
		outerCone = fCompPB->GetInt( (ParamID)kSoundOConeAngle );
	}
	else
	{
		OutVol = 0;
		innerCone = 360;
		outerCone = 360;
	}

	destSound->SetMax(max);
	destSound->SetMin(min);
	destSound->SetOuterVolume(OutVol - 10000);
	destSound->SetConeAngles(innerCone, outerCone);
	destSound->SetProperty( plSound::kPropLocalOnly, fCompPB->GetInt( (ParamID)kSndIsLocalOnly ) ? true : false );
	destSound->SetPriority( fCompPB->GetInt( (ParamID)kSndPriority ) );

	if( fCompPB->GetInt( (ParamID)kSndIncidental ) )
	{
		destSound->SetProperty( plSound::kPropIncidental, true );

		// Refactor the priority, since incidental priorities are a different range
		int pri = fCompPB->GetInt( (ParamID)kSndPriority );
		pri = pri < 1 ? 1 : pri;
		destSound->SetPriority( pri );
	}

	if( fCompPB->GetInt( (ParamID)kSndDisableLOD ) )
	{
		// Force LOD off on this sound
		destSound->SetProperty( plSound::kPropDisableLOD, true );
	}
	
	if( fCompPB->GetInt( (ParamID)kSndChannelSelect ) )
		destSound->SetChannelSelect( plWin32Sound::kRightChannel );
	else
		destSound->SetChannelSelect( plWin32Sound::kLeftChannel );

	IGrabSoftRegion( destSound, pErrMsg );
	IGrabEAXParams( destSound, pErrMsg );
}

hsBool plSound3DEmitterComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	if (!fValidNodes[node])
		return false;

	if( fCreateGrouped )
		return true;

	char* fileName = GetSoundFileName( kBaseSound );

	int fIndex = -1;
	if (fIndices.find(node) != fIndices.end())
		fIndex = fIndices[node];

	plSoundBuffer *srcBuffer = IProcessSourceBuffer( node, pErrMsg );
	if( srcBuffer == nil )
		return false;

	const plAudioInterface* ai = node->GetSceneObject()->GetAudioInterface();
	plWinAudible* pAudible = (plWinAudible*)ai->GetAudible();


	char	keyName[ 256 ];
	sprintf( keyName, "%s", GetINode()->GetName());

	plWin32Sound *sound = nil;

	if (!strcmp(node->GetName(), "LinkSoundSource"))
		sound = TRACKED_NEW plWin32LinkSound;
	else
	{
#if 0
		sound = TRACKED_NEW plWin32StaticSound;
#else
		/// New method, still in testing: any sounds over 4 seconds get made into streaming sounds
		if( srcBuffer->GetDataLengthInSecs() > 4.f )
			sound = TRACKED_NEW plWin32StreamingSound;
		else
			sound = TRACKED_NEW plWin32StaticSound;
	}
#endif

	hsgResMgr::ResMgr()->NewKey(keyName, sound, node->GetLocation(), node->GetLoadMask());
	hsgResMgr::ResMgr()->AddViaNotify( srcBuffer->GetKey(), TRACKED_NEW plGenRefMsg( sound->GetKey(), plRefMsg::kOnCreate, -1, plSound::kRefDataBuffer ), plRefFlags::kActiveRef );
	
	if( pAudible->AddSound( sound, fIndex, true ) )
	{
		ISetParameters( sound, pErrMsg );
	}

	return true;
}

// Converts an array of components into a single grouped sound
hsBool	plSound3DEmitterComponent::ConvertGrouped( plMaxNode *baseNode, hsTArray<plBaseSoundEmitterComponent *> &groupArray, plErrorMsg *pErrMsg )
{
	char	keyName[ 256 ];


	if( !fValidNodes[ baseNode ] || !fCreateGrouped )
		return false;


	// First, we need to grab the sound buffers from ALL the components and merge them into one big buffer.
	// Also build up an array of positions to feed to our groupedSound later.
	// Also also assign all the components the audioInterface index (will be the same one, so we need to
	// allocate it here).
	// Also also also build up a volume array parallel to startPoses that represents the individual volume
	// setting for each sound in the group
	hsTArray<UInt32>		startPoses;
	hsTArray<hsScalar>		volumes;
	hsLargeArray<UInt8>		mergedData;
	int						i;
	plWAVHeader				mergedHeader;

	for( i = 0; i < groupArray.GetCount(); i++ )
	{
		// Make sure they're all 3D sounds...
		if( groupArray[ i ]->ClassID() != SOUND_3D_COMPONENT_ID )
		{
			char msg[ 512 ];
			sprintf( msg, "The sound component %s isn't a 3D sound, which is necessary for making grouped sounds. "
						"Make sure all the sounds in this group are 3D sounds.", groupArray[ i ]->GetINode()->GetName() );
			IShowError( kSrcBufferInvalid, msg, baseNode->GetName(), pErrMsg );

			// Attempt to recover
			startPoses.Append( mergedData.GetCount() );
			volumes.Append( 1.f );
			continue;
		}

		// Grab the buffer for this sound directly from the original source
		char *fileName = groupArray[ i ]->GetSoundFileName( kBaseSound );

		plSoundBuffer	*buffer = TRACKED_NEW plSoundBuffer( fileName );
		if( !buffer->IsValid() || !buffer->EnsureInternal() )
		{
			// OK, because some *cough* machines are completely stupid and don't load AssetMan scenes with 
			// AssetMan plugins, we need to do a really stupid fallback search to the current exporting directory.
			const char *plasmaDir = plMaxConfig::GetClientPath();
			bool worked = false;
			if( plasmaDir != nil )
			{
				char newPath[ MAX_PATH ], *c;
				strcpy( newPath, plasmaDir );
				strcat( newPath, "sfx\\" );
				
				c = strrchr( fileName, '\\' );
				if( c == nil )
					c = strrchr( fileName, '/' );
				if( c == nil )
					c = fileName;
				else
					c++;
				strcat( newPath, c );

				// Got a path to try, so try it!
				delete buffer;
				buffer = TRACKED_NEW plSoundBuffer( newPath );
				if( buffer->IsValid() && buffer->EnsureInternal() )
					worked = true;
			}

			if( !worked )
			{
				char msg[ 512 ];
				sprintf( msg, "The sound file %s cannot be loaded for component %s.", fileName, groupArray[ i ]->GetINode()->GetName() );
				IShowError( kSrcBufferInvalid, msg, baseNode->GetName(), pErrMsg );
				delete buffer;

				// Attempt to recover
				startPoses.Append( mergedData.GetCount() );
				volumes.Append( 1.f );
				continue;
			}
		}
		
		// Get a header (they should all be the same)
		if( i == 0 )
			mergedHeader = buffer->GetHeader();
		else
		{
			if( memcmp( &mergedHeader, &buffer->GetHeader(), sizeof( mergedHeader ) ) != 0 )
			{
				char msg[ 512 ];
				sprintf( msg, "The format for sound file %s does not match the format for the other grouped sounds on node %s. "
							"Make sure the sounds are all the same format.", fileName, baseNode->GetName() );
				IShowError( kMergeSourceFormatMismatch, msg, baseNode->GetName(), pErrMsg );
				delete buffer;

				// Attempt to recover
				startPoses.Append( mergedData.GetCount() );
				volumes.Append( 1.f );
				continue;
			}
		}

		// Grab the data from this buffer and merge it
		// HACK: SetCount() won't copy the old data over, Expand() won't up the use count, so do
		// an expand-and-setCount combo.
		UInt32 pos = mergedData.GetCount();
		startPoses.Append( pos );
		mergedData.Expand( pos + buffer->GetDataLength() );
		mergedData.SetCount( pos + buffer->GetDataLength() );
		memcpy( &mergedData[ pos ], buffer->GetData(), buffer->GetDataLength() );

		delete buffer;

		// Also keep track of what the volume should be for this particular sound
		volumes.Append( groupArray[ i ]->GetSoundVolume() );
	}

	/// We got a merged buffer, so make a plSoundBuffer from it
	int index = -1;
	if( fIndices.find( baseNode ) != fIndices.end() )
		index = fIndices[ baseNode ];

	sprintf( keyName, "%s_MergedSound", GetINode()->GetName() );

	plKey buffKey = baseNode->FindPageKey( plSoundBuffer::Index(), keyName );	
	if( buffKey != nil )
		plPluginResManager::ResMgr()->NukeKeyAndObject( buffKey );

	// Create a new one...
	plSoundBuffer	*mergedBuffer = TRACKED_NEW plSoundBuffer();
	mergedBuffer->SetInternalData( mergedHeader, mergedData.GetCount(), mergedData.AcquireArray() );
	mergedData.Reset();
	// The buffer may be shared across multiple sources. We could or together the LoadMasks of all
	// the nodes that use it, or we can just go with the default loadmask of Always load, and
	// count on it never getting dragged into memory if nothing that references it does.
	hsgResMgr::ResMgr()->NewKey( keyName, mergedBuffer, baseNode->GetLocation() );


	/// We got the sound buffer, now just create a groupedSound for it

	const plAudioInterface* ai = baseNode->GetSceneObject()->GetAudioInterface();
	plWinAudible* pAudible = (plWinAudible*)ai->GetAudible();

	sprintf( keyName, "%s", GetINode()->GetName());
	plWin32GroupedSound *sound = TRACKED_NEW plWin32GroupedSound;
	sound->SetPositionArray( startPoses.GetCount(), startPoses.AcquireArray(), volumes.AcquireArray() );
	sound->SetProperty( plSound::kPropLoadOnlyOnCall, true );

	hsgResMgr::ResMgr()->NewKey( keyName, sound, baseNode->GetLocation(), baseNode->GetLoadMask() );
	hsgResMgr::ResMgr()->AddViaNotify( mergedBuffer->GetKey(), TRACKED_NEW plGenRefMsg( sound->GetKey(), plRefMsg::kOnCreate, -1, plSound::kRefDataBuffer ), plRefFlags::kActiveRef );
	
	if( pAudible->AddSound( sound, index, true ) )
	{
		// Just use the first component
		plSound3DEmitterComponent *grpComp = (plSound3DEmitterComponent *)groupArray[ 0 ];
		grpComp->ISetParameters( sound, pErrMsg );
	}
	
	/// All done!
	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Background Music Component
//

class plBackgroundMusicComponent : public plBaseSoundEmitterComponent
{
public:
	plBackgroundMusicComponent();
	virtual ~plBackgroundMusicComponent();

	// Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	hsBool SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg);

	hsBool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg);
	hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);

	virtual hsBool	IsLocalOnly( void ) const { if( fCompPB->GetInt( (ParamID)kSndIsLocalOnly ) ) return true; else return false; }

protected:
	virtual UInt32 ICalcSourceBufferFlags() const;

	bool IValidate(plMaxNode *node, plErrorMsg *pErrMsg);
	virtual hsBool	IGetCategoryList( char **&catList, int *&catKonstantList );
};

//Max desc stuff necessary below.
CLASS_DESC(plBackgroundMusicComponent, gBgndMusicEmitterDesc, "Nonspatial Sound",  "NonspatSound", COMP_TYPE_AUDIO, BGND_MUSIC_COMPONENT_ID)


ParamBlockDesc2 gBgndMusicEmitterBk
(
	plComponent::kBlkComp, _T("Bgnd Music"), 0, &gBgndMusicEmitterDesc, P_AUTO_CONSTRUCT + P_AUTO_UI + P_MULTIMAP + P_INCLUDE_PARAMS, plComponent::kRefComp,

	5,	// Number of rollouts
	sSoundSharedPBHeader(IDS_COMP_SOUNDBASE),
	kS3DBaseParams,			IDD_COMP_SOUNDBGND,			IDS_COMP_SOUNDBGND,			0, 0, &gSoundCompProc,
	sSndWaveformPropsHeader,
	kS3DSoftVolumeParams,	IDD_COMP_SOUND_SOFTPARAMS,	IDS_COMP_SOUNDSOFTPARAMS,	0, 0, &gSoundSoftVolumeSelProc,

	// Included paramblock
	&sSoundSharedPB,

	// Waveform props define
	sSndWaveformPropsParamTemplate,

	// params

	kSndIsLocalOnly, _T("noNetworkSynch"),		TYPE_BOOL, 		0, 0,
		p_default,	FALSE,
		p_ui,	kS3DBaseParams, TYPE_SINGLECHEKBOX, IDC_SND_LOCALONLY,
		end,

	kSndStreamCompressed,	_T("stream"),		TYPE_BOOL,		0, 0,
		p_ui,	kS3DBaseParams, TYPE_SINGLECHEKBOX, IDC_CHECK_STREAM,
		end,

	/// Soft Region/Volume Parameters rollout
	kSndSoftRegionEnable,	_T( "enableSoftRegion" ), TYPE_BOOL, 0, 0,
		p_ui,	kS3DSoftVolumeParams, TYPE_SINGLECHEKBOX, IDC_SOUND_SOFTENABLE,
		p_default, FALSE,
		end,

	kSndSoftRegion, _T("softRegion"),	TYPE_INODE,		0, 0,
		p_prompt, IDS_COMP_SOUNDSOFTSELECT,
		p_accessor, &gSoundSoftVolAccessor,
		end,

	end
);


plBackgroundMusicComponent::plBackgroundMusicComponent()
{
	fClassDesc = &gBgndMusicEmitterDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

plBackgroundMusicComponent::~plBackgroundMusicComponent()
{
}

// Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
hsBool plBackgroundMusicComponent::SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg)
{
	return plBaseSoundEmitterComponent::SetupProperties( pNode, pErrMsg );
}

UInt32 plBackgroundMusicComponent::ICalcSourceBufferFlags() const
{
	UInt32 ourFlags = 0;
	if (fCompPB->GetInt(kSndStreamCompressed))
		ourFlags |= plSoundBuffer::kStreamCompressed;

	return plBaseSoundEmitterComponent::ICalcSourceBufferFlags() | ourFlags;
}

bool plBackgroundMusicComponent::IValidate(plMaxNode *node, plErrorMsg *pErrMsg)
{
	return plBaseSoundEmitterComponent::IValidate( node, pErrMsg );
}

hsBool plBackgroundMusicComponent::PreConvert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	return plBaseSoundEmitterComponent::PreConvert( node, pErrMsg, BGND_MUSIC_COMPONENT_ID );
}

hsBool plBackgroundMusicComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	if (!fValidNodes[node])
		return false;

	char* fileName = GetSoundFileName( kBaseSound );

	int fIndex = -1;
	if (fIndices.find(node) != fIndices.end())
		fIndex = fIndices[node];

	const plAudioInterface* ai = node->GetSceneObject()->GetAudioInterface();
	plWinAudible* pAudible = (plWinAudible*)ai->GetAudible();

	plSoundBuffer *srcBuffer = IProcessSourceBuffer( node, pErrMsg );
	if( srcBuffer == nil )
		return false;

	char	keyName[ 256 ];
	sprintf( keyName, "%s_Win32BgndSnd", GetINode()->GetName() );
	plWin32Sound *sound = nil;

	if( srcBuffer->GetDataLengthInSecs() > 4.f )
		sound = TRACKED_NEW plWin32StreamingSound;
	else
		sound = TRACKED_NEW plWin32StaticSound;

	hsgResMgr::ResMgr()->NewKey(keyName, sound, node->GetLocation(), node->GetLoadMask());

	srcBuffer->SetFlag( plSoundBuffer::kAlwaysExternal );
	hsgResMgr::ResMgr()->AddViaNotify( srcBuffer->GetKey(), TRACKED_NEW plGenRefMsg( sound->GetKey(), plRefMsg::kOnCreate, -1, plSound::kRefDataBuffer ), plRefFlags::kActiveRef );
	
	if (pAudible->AddSound( sound, fIndex, false))
	{
		ISetBaseParameters( sound, pErrMsg );

		sound->SetProperty( plSound::kPropLocalOnly, fCompPB->GetInt( (ParamID)kSndIsLocalOnly ) ? true : false );
		sound->SetPriority( fCompPB->GetInt( (ParamID)kSndPriority ) );

		if( fCompPB->GetInt( (ParamID)kSndDisableLOD ) )
		{
			// Force LOD off on this sound
			sound->SetProperty( plSound::kPropDisableLOD, true );
		}

		IGrabSoftRegion( sound, pErrMsg );
		sound->GetEAXSettings().Enable( false );
	}

	return true;
}

//// IGetCategoryList ///////////////////////////////////////////////////////////////////////////
//	Returns a list of the categories and konstants supported for this type of sound

hsBool	plBackgroundMusicComponent::IGetCategoryList( char **&catList, int *&catKonstantList )
{
	static char	*cats[] = { "Background Music", "Ambience", "Sound FX", "GUI", "NPC Voice", "" };
	static int	catEnums[] = { plSound::kBackgroundMusic, plSound::kAmbience, plSound::kSoundFX, plSound::kGUISound, plSound::kNPCVoices };

	catList = cats;
	catKonstantList = catEnums;

	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////
//
//	GUI Sound Component
//

class plGUISoundComponent : public plBaseSoundEmitterComponent
{
public:
	plGUISoundComponent();
	virtual ~plGUISoundComponent();

	// Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	hsBool SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg);

	hsBool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg);
	hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);

	virtual void	UpdateSoundFileSelection( void ) { ; }

protected:

	bool	IValidate(plMaxNode *node, plErrorMsg *pErrMsg);

	virtual hsBool	IGetCategoryList( char **&catList, int *&catKonstantList );

	virtual hsBool	IHasWaveformProps( void ) const { return false; }
};

//Max desc stuff necessary below.
CLASS_DESC(plGUISoundComponent, gGUISoundEmitterDesc, "GUI Sound",  "GUISound", COMP_TYPE_AUDIO, GUI_SOUND_COMPONENT_ID)


ParamBlockDesc2 gGUISoundEmitterBk
(
	plComponent::kBlkComp, _T("GUI Sound"), 0, &gGUISoundEmitterDesc, P_AUTO_CONSTRUCT + P_AUTO_UI + P_MULTIMAP + P_INCLUDE_PARAMS, plComponent::kRefComp,

	2,	// Number of rollouts
	sSoundSharedPBHeader(IDS_COMP_SOUNDGUI),

	// Included paramblock
	&sSoundSharedPB,

	end
);


plGUISoundComponent::plGUISoundComponent()
{
	fClassDesc = &gGUISoundEmitterDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

plGUISoundComponent::~plGUISoundComponent()
{
}

//// IGetCategoryList ///////////////////////////////////////////////////////////////////////////
//	Returns a list of the categories and konstants supported for this type of sound

hsBool	plGUISoundComponent::IGetCategoryList( char **&catList, int *&catKonstantList )
{
	static char	*cats[] = { "GUI", "" };
	static int	catEnums[] = { plSound::kGUISound };

	catList = cats;
	catKonstantList = catEnums;

	return true;
}

// Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
hsBool plGUISoundComponent::SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg)
{
	return plBaseSoundEmitterComponent::SetupProperties( pNode, pErrMsg );
}

bool plGUISoundComponent::IValidate(plMaxNode *node, plErrorMsg *pErrMsg)
{
	return plBaseSoundEmitterComponent::IValidate( node, pErrMsg );
}

hsBool plGUISoundComponent::PreConvert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	return plBaseSoundEmitterComponent::PreConvert( node, pErrMsg, GUI_SOUND_COMPONENT_ID );
}

hsBool plGUISoundComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	if (!fValidNodes[node])
		return false;

	char* fileName = GetSoundFileName( kBaseSound );

	int fIndex = -1;
	if (fIndices.find(node) != fIndices.end())
		fIndex = fIndices[node];

	const plAudioInterface* ai = node->GetSceneObject()->GetAudioInterface();
	plWinAudible* pAudible = (plWinAudible*)ai->GetAudible();

	plSoundBuffer *srcBuffer = GetSourceBuffer( fileName, node, ICalcSourceBufferFlags() );
	if( srcBuffer == nil )
	{
		pErrMsg->Set( true, node->GetName(), "The file specified for the sound 3D component %s is invalid. This emitter will not be exported.", GetINode()->GetName() ).Show();
		pErrMsg->Set( false );
		return false;
	}

	char	keyName[ 256 ];
	sprintf( keyName, "%s_Win32GUISound", GetINode()->GetName() );

	plWin32StaticSound *sound = TRACKED_NEW plWin32StaticSound;
	hsgResMgr::ResMgr()->NewKey(keyName, sound, node->GetLocation(), node->GetLoadMask());

	hsgResMgr::ResMgr()->AddViaNotify( srcBuffer->GetKey(), TRACKED_NEW plGenRefMsg( sound->GetKey(), plRefMsg::kOnCreate, -1, plSound::kRefDataBuffer ), plRefFlags::kActiveRef );
	
	if (pAudible->AddSound( sound, fIndex, false))
	{
		ISetBaseParameters( sound, pErrMsg );

		sound->SetProperty( plSound::kPropLocalOnly, true );	// GUI sounds are always local-only

		if( fCompPB->GetInt( (ParamID)kSndDisableLOD ) )
		{
			// Force LOD off on this sound
			sound->SetProperty( plSound::kPropDisableLOD, true );
		}
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//
//	EAX Listener Soft Region component
//

class plEAXListenerComponent : public plComponent
{
public:
	enum 
	{
		kRefSoftRegion,
		kRefWhichSettings,
		kRefPreset,
		kRefCustFile,

		// The following are the parameters for the listener as defined in eax.h, minus the panning
		kRefEnvironmentSize,		// float
		kRefEnvironmentDiffusion,	// float
		kRefRoom,					// long
		kRefRoomHF,					// long
		kRefRoomLF,					// long
		kRefDecayTime,				// float
		kRefDecayHFRatio,			// float
		kRefDecayLFRatio,			// float
		kRefReflections,			// long
		kRefReflectionsDelay,		// float
		// panning goes here
		kRefReverb,					// long
		kRefReverbDelay,			// float
		// Reverb pan
		kRefEchoTime,				// float
		kRefEchoDepth,
		kRefModulationTime,
		kRefModulationDepth,
		kRefAirAbsorptionHF,
		kRefHFReference,
		kRefLFReference,
		kRefRoomRolloffFactor,
		kRefFlags,				// unsigned long
	};

public:
	plEAXListenerComponent();
	void DeleteThis() { delete this; }

	// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	hsBool SetupProperties(plMaxNode *pNode, plErrorMsg *errMsg);

	hsBool PreConvert(plMaxNode *pNode, plErrorMsg *errMsg);
	hsBool Convert(plMaxNode *node, plErrorMsg *errMsg);

	const char *GetCustFileName( void ) const;
	void		SetCustFile( const char *path );
};

// When one of our parameters that is a ref changes, send out the component ref
// changed message.  Normally, messages from component refs are ignored since
// they pass along all the messages of the ref, which generates a lot of false
// converts.
class plEAXListenerAccessor : public PBAccessor
{
public:
	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)
	{
		if (id == plEAXListenerComponent::kRefSoftRegion )
		{
			plEAXListenerComponent *comp = (plEAXListenerComponent*)owner;
			comp->NotifyDependents(FOREVER, PART_ALL, REFMSG_USER_COMP_REF_CHANGED);
		}
	}
};
static plEAXListenerAccessor gEAXListenerAccessor;

//// DialogProc for EAXListenerComponent ////////////////////////////////////////////////////////

class plEAXListenerDlgProc : public plSingleCompSelProc
{
protected:

	hsBool	IGetCustFileName( plEAXListenerComponent *listenerComp )
	{
		TCHAR	fileName[ MAX_PATH ], dirName[ MAX_PATH ];

	
		const char *name = listenerComp->GetCustFileName();
		if( name != nil )
			strcpy( fileName, name );
		else
			strcpy( fileName, _T( "" ) );

		strcpy( dirName, fileName );
		::PathRemoveFileSpec( dirName );

		OPENFILENAME ofn = {0};
		ofn.lStructSize = sizeof( OPENFILENAME );
		ofn.hwndOwner = GetCOREInterface()->GetMAXHWnd();
		ofn.lpstrFilter = "EAX Preset Files (*.eax)\0*.eax\0All Files\0*.*\0";
		ofn.lpstrFile = fileName;
		ofn.nMaxFile = sizeof( fileName );
		ofn.lpstrInitialDir = dirName;
		ofn.lpstrTitle = "Choose a sound file";
		ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;
		ofn.lpstrDefExt = "eax";

		if( GetOpenFileName( &ofn ) )
		{
			listenerComp->SetCustFile( fileName );
			return true;
		}
		else
			return false;
	}
public:

	plEAXListenerDlgProc() 
		: plSingleCompSelProc( plEAXListenerComponent::kRefSoftRegion, IDC_EAX_SOFTREGION, "Select the soft region to apply these EAX listener properties to" )
	{
	}

	BOOL DlgProc( TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
	{
		int				i;
		IParamBlock2	*pb = map->GetParamBlock();


		switch ( msg )
		{
			case WM_INITDIALOG:
				{

					// Load the preset combo with preset names
					HWND comboBox = GetDlgItem( hWnd, IDC_EAX_PRESET_COMBO );
					ComboBox_ResetContent( comboBox );

					for( i = 0; i < /*sizeof( EAX30_ORIGINAL_PRESETS ) 
						/ sizeof( EAXLISTENERPROPERTIES )*/26 ; i++ )
						ComboBox_AddString( comboBox, EAX30_ORIGINAL_PRESET_NAMES[ i ] );

					ComboBox_SetCurSel( comboBox, pb->GetInt( (ParamID)plEAXListenerComponent::kRefPreset ) );

					ICustButton *custButton = GetICustButton( GetDlgItem( hWnd, IDC_EAX_CUSTFILE ) );
					if( custButton != nil )
					{
						custButton->SetText( pb->GetStr( (ParamID)plEAXListenerComponent::kRefCustFile ) );
						ReleaseICustButton( custButton );
					}
				}
				break;

			case WM_COMMAND:	
				if( LOWORD( wParam ) == IDC_EAX_PRESET_COMBO )
				{
					int sel = SendDlgItemMessage( hWnd, IDC_EAX_PRESET_COMBO, CB_GETCURSEL, 0, 0 );
					if( sel != CB_ERR )
						pb->SetValue( (ParamID)plEAXListenerComponent::kRefPreset, 0, sel );
					return true;
				}
				if( ( HIWORD( wParam ) == BN_CLICKED ) && LOWORD( wParam ) == IDC_EAX_CUSTFILE )
				{
					// Get the file to load
					plEAXListenerComponent *comp = (plEAXListenerComponent *)map->GetParamBlock()->GetOwner();
					if( IGetCustFileName( comp ) )
					{
						ICustButton *custButton = GetICustButton( GetDlgItem( hWnd, IDC_EAX_CUSTFILE ) );
						if( custButton != nil )
						{
							custButton->SetText( pb->GetStr( (ParamID)plEAXListenerComponent::kRefCustFile ) );
							ReleaseICustButton( custButton );
						}
					}
				}
				break;
		}

		return plSingleCompSelProc::DlgProc( t, map, hWnd, msg, wParam, lParam );
	}

	void DeleteThis() {}
};

static plEAXListenerDlgProc	gEAXListenerDlgProc;

//Max desc stuff necessary below.
CLASS_DESC(plEAXListenerComponent, gEAXListenerDesc, "EAX Listener",  "EAXListener", COMP_TYPE_AUDIO, EAX_LISTENER_COMPONENT_ID)

ParamBlockDesc2 gEAXListenerBlk
(	// KLUDGE: not the defined block ID, but kept for backwards compat.
 plComponent::kBlkComp, _T("EAXListener"), 0, &gEAXListenerDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	IDD_COMP_EAXLISTENER, IDS_COMP_EAXLISTENER, 0, 0, &gEAXListenerDlgProc,

	plEAXListenerComponent::kRefSoftRegion, _T("SoftRegion"),	TYPE_INODE,		0, 0,
		p_accessor,		&gEAXListenerAccessor,
		end,
	
	plEAXListenerComponent::kRefWhichSettings, _T("whichSettings"),	TYPE_INT,		0, 0,
		p_ui, TYPE_RADIO, 2, IDC_EAX_PRESET, IDC_EAX_CUSTOM,
		p_default, 0,
		end,

	plEAXListenerComponent::kRefPreset, _T("preset"),	TYPE_INT,		0, 0,
		p_default, 0,
		end,

	// This is just a label for now, so the users know what file the presets came from
	plEAXListenerComponent::kRefCustFile, _T("custFile"), TYPE_STRING, 0, 0,
		p_default, _T(""),
		end,

	// EAX listener params (should be private)
	plEAXListenerComponent::kRefEnvironmentSize,		_T(""),	TYPE_FLOAT, 0, 0, end,		// float
	plEAXListenerComponent::kRefEnvironmentDiffusion,	_T(""),	TYPE_FLOAT, 0, 0, end,// float
	plEAXListenerComponent::kRefRoom,					_T(""),	TYPE_INT, 0, 0, end,// long
	plEAXListenerComponent::kRefRoomHF,					_T(""),	TYPE_INT, 0, 0, end,// long
	plEAXListenerComponent::kRefRoomLF,					_T(""),	TYPE_INT, 0, 0, end,// long
	plEAXListenerComponent::kRefDecayTime,				_T(""),	TYPE_FLOAT, 0, 0, end,// float
	plEAXListenerComponent::kRefDecayHFRatio,			_T(""),	TYPE_FLOAT, 0, 0, end,// float
	plEAXListenerComponent::kRefDecayLFRatio,			_T(""),	TYPE_FLOAT, 0, 0, end,// float
	plEAXListenerComponent::kRefReflections,			_T(""),	TYPE_INT, 0, 0, end,// long
	plEAXListenerComponent::kRefReflectionsDelay,		_T(""),	TYPE_FLOAT, 0, 0, end,// float
		// panning goes here
	plEAXListenerComponent::kRefReverb,					_T(""),	TYPE_INT, 0, 0, end,// long
	plEAXListenerComponent::kRefReverbDelay,			_T(""),	TYPE_FLOAT, 0, 0, end,// float
		// Reverb pan
	plEAXListenerComponent::kRefEchoTime,				_T(""),	TYPE_FLOAT, 0, 0, end,// float
	plEAXListenerComponent::kRefEchoDepth,				_T(""),	TYPE_FLOAT, 0, 0, end,
	plEAXListenerComponent::kRefModulationTime,			_T(""),	TYPE_FLOAT, 0, 0, end,
	plEAXListenerComponent::kRefModulationDepth,		_T(""),	TYPE_FLOAT, 0, 0, end,
	plEAXListenerComponent::kRefAirAbsorptionHF,		_T(""),	TYPE_FLOAT, 0, 0, end,
	plEAXListenerComponent::kRefHFReference,			_T(""),	TYPE_FLOAT, 0, 0, end,
	plEAXListenerComponent::kRefLFReference,			_T(""),	TYPE_FLOAT, 0, 0, end,
	plEAXListenerComponent::kRefRoomRolloffFactor,		_T(""),	TYPE_FLOAT, 0, 0, end,
	plEAXListenerComponent::kRefFlags,					_T(""),	TYPE_INT, 0, 0, end,// unsigned long

	end
);

plEAXListenerComponent::plEAXListenerComponent()
{
	fClassDesc = &gEAXListenerDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

hsBool plEAXListenerComponent::Convert(plMaxNode *node, plErrorMsg *errMsg)
{
	if( !fCompPB->GetINode((ParamID)kRefSoftRegion) )
		return true;

	plSceneObject* sceneObj = node->GetSceneObject();
	if( !sceneObj )
		return true;
/*
	plLightInfo* li = plLightInfo::ConvertNoRef(sceneObj->GetGenericInterface(plLightInfo::Index()));
	if( !li )
		return true;
*/
	plSoftVolBaseComponent* softComp = plSoftVolBaseComponent::GetSoftComponent( fCompPB->GetINode( (ParamID)kRefSoftRegion ) );
	if( !softComp )
		return true;

	plKey softKey = softComp->GetSoftVolume();
	if( !softKey )
		return true;

	// Create a listener mod to handle these things
	plEAXListenerMod *listener = TRACKED_NEW plEAXListenerMod();
	node->AddModifier( listener, IGetUniqueName(node) );

	// Add the soft region
	hsgResMgr::ResMgr()->AddViaNotify( softKey, TRACKED_NEW plGenRefMsg( listener->GetKey(), plRefMsg::kOnCreate, 0, plEAXListenerMod::kRefSoftRegion ), plRefFlags::kActiveRef );

	// Set up the parameters of the listener mod
	EAXLISTENERPROPERTIES *listenerProps = listener->GetListenerProps();
	if( fCompPB->GetInt( (ParamID)kRefWhichSettings ) == 0 )
	{
		// Set params based on a preset
		listener->SetFromPreset( fCompPB->GetInt( (ParamID)kRefPreset ) );
	}
	else
	{
		// Get the raw params
		listenerProps->flEnvironmentSize = fCompPB->GetFloat( (ParamID)kRefEnvironmentSize );
		listenerProps->flEnvironmentDiffusion = fCompPB->GetFloat( (ParamID)kRefEnvironmentDiffusion );
		listenerProps->lRoom = fCompPB->GetInt( (ParamID)kRefRoom );
		listenerProps->lRoomHF = fCompPB->GetInt( (ParamID)kRefRoomHF );
		listenerProps->lRoomLF = fCompPB->GetInt( (ParamID)kRefRoomLF );
		listenerProps->flDecayTime = fCompPB->GetFloat( (ParamID)kRefDecayTime );
		listenerProps->flDecayHFRatio = fCompPB->GetFloat( (ParamID)kRefDecayHFRatio );
		listenerProps->flDecayLFRatio = fCompPB->GetFloat( (ParamID)kRefDecayLFRatio );
		listenerProps->lReflections = fCompPB->GetInt( (ParamID)kRefReflections );
		listenerProps->flReflectionsDelay = fCompPB->GetFloat( (ParamID)kRefReflectionsDelay );
		//listenerProps->vReflectionsPan;     // early reflections panning vector
		listenerProps->lReverb = fCompPB->GetInt( (ParamID)kRefReverb );                  // late reverberation level relative to room effect
		listenerProps->flReverbDelay = fCompPB->GetFloat( (ParamID)kRefReverbDelay );
		//listenerProps->vReverbPan;          // late reverberation panning vector
		listenerProps->flEchoTime = fCompPB->GetFloat( (ParamID)kRefEchoTime );
		listenerProps->flEchoDepth = fCompPB->GetFloat( (ParamID)kRefEchoDepth );
		listenerProps->flModulationTime = fCompPB->GetFloat( (ParamID)kRefModulationTime );
		listenerProps->flModulationDepth = fCompPB->GetFloat( (ParamID)kRefModulationDepth );
		listenerProps->flAirAbsorptionHF = fCompPB->GetFloat( (ParamID)kRefAirAbsorptionHF );
		listenerProps->flHFReference = fCompPB->GetFloat( (ParamID)kRefHFReference );
		listenerProps->flLFReference = fCompPB->GetFloat( (ParamID)kRefLFReference );
		listenerProps->flRoomRolloffFactor = fCompPB->GetFloat( (ParamID)kRefRoomRolloffFactor );
		listenerProps->ulFlags = fCompPB->GetInt( (ParamID)kRefFlags );
	}
	
	return true;
}

hsBool plEAXListenerComponent::PreConvert(plMaxNode *pNode,  plErrorMsg *errMsg)
{

	return true;
}

// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
hsBool plEAXListenerComponent::SetupProperties(plMaxNode *pNode,  plErrorMsg *errMsg)
{

	return true;
}

const char *plEAXListenerComponent::GetCustFileName( void ) const
{
	return (const char *)fCompPB->GetStr( (ParamID)kRefCustFile );
}

void	plEAXListenerComponent::SetCustFile( const char *path )
{
	char	file[ MAX_PATH ];
	int		i;
	hsUNIXStream	presetFile;

	// Map of PB values to file entries
	struct FilePBMap
	{
		char	*fKeyword;
		ParamID	fParamID;
		UInt8	fType;	// 0 is int, 1 is float for now
	} myMap[] = { 
		{ "flEnvironmentSize", kRefEnvironmentSize, 1 },
		{ "flEnvironmentDiffusion", kRefEnvironmentDiffusion, 1 },
		{ "lRoom", kRefRoom, 0 },
		{ "lRoomHF", kRefRoomHF, 0 },
		{ "lRoomLF", kRefRoomLF, 0 },
		{ "flDecayTime", kRefDecayTime, 1 },
		{ "flDecayHFRatio", kRefDecayHFRatio, 1 },
		{ "flDecayLFRatio", kRefDecayLFRatio, 1 },
		{ "lReflections", kRefReflections, 0 },
		{ "flReflectionsDelay", kRefReflectionsDelay, 1 },
		{ "lReverb", kRefReverb, 0 },
		{ "flReverbDelay", kRefReverbDelay, 1 },
		{ "flEchoTime", kRefEchoTime, 1 },
		{ "flEchoDepth", kRefEchoDepth, 1 },
		{ "flModulationTime", kRefModulationTime, 1 },
		{ "flModulationDepth", kRefModulationDepth, 1 },
		{ "flAirAbsorptionHF", kRefAirAbsorptionHF, 1 },
		{ "flHFReference", kRefHFReference, 1 },
		{ "flLFReference", kRefLFReference, 1 },
		{ "flRoomRolloffFactor", kRefRoomRolloffFactor, 1 },
		{ "dwFlags", kRefFlags, 0 },
		{ nil, 0, 0 } };

	// Read the file and set settings from it
	if( !presetFile.Open( path, "rt" ) )
	{
		// Oops
		hsAssert( false, "can't open file" );
		return;
	}

	// Loop and find our keywords
	for( i = 0; myMap[ i ].fKeyword != nil && !presetFile.AtEnd(); )
	{
		char line[ 512 ];

		// Read a line from the file until we find our keyword
		presetFile.ReadLn( line, sizeof( line ) );
		if( strstr( line, myMap[ i ].fKeyword ) == nil )
			continue;

		// Read the next line, with our value
		presetFile.ReadLn( line, sizeof( line ) );
		float value = atof( line );
		if( myMap[ i ].fType == 0 )
			fCompPB->SetValue( myMap[ i ].fParamID, 0, (int)value );
		else
			fCompPB->SetValue( myMap[ i ].fParamID, 0, (float)value );

		i++;
	}

	if( myMap[ i ].fKeyword != nil )
	{
		hsAssert( false, "Couldn't find all of the keywords in the settings file. Oh well" );
	}

	// All done!
	presetFile.Close();

	// Update our helper reminder string
	_splitpath( path, nil, nil, file, nil );
	fCompPB->SetValue( (ParamID)kRefCustFile, 0, file );
}

/// Obsolete SFX components (made obsolete by the new EAX support)

OBSOLETE_CLASS(plSoundReverbComponent, gSoundReverbDesc, "Audio Region",  "AudioRegion", COMP_TYPE_AUDIO, Class_ID(0x50507200, 0x48651c4c))
OBSOLETE_CLASS(plSoundChorusModComponent,gSoundChorusModDesc , "Chorus Effect",  "ChorusEffect", COMP_TYPE_AUDIO, Class_ID(0x10f91101, 0x28cb21b9))
OBSOLETE_CLASS(plSoundCompressorModComponent,gSoundCompressorModDesc , "Compressor Effect",  "CompressEffect", COMP_TYPE_AUDIO, Class_ID(0x443d2167, 0x4ca42eb))
OBSOLETE_CLASS(plSoundDistortModComponent,gSoundDistortModDesc , "Distort Effect",  "DistortEffect", COMP_TYPE_AUDIO, Class_ID(0x7cb45868, 0x61220227))
OBSOLETE_CLASS(plSoundEchoModComponent,gSoundEchoModDesc , "Echo Effect",  "EchoEffect", COMP_TYPE_AUDIO,Class_ID(0x2948347e, 0x30ba0be3))
OBSOLETE_CLASS(plSoundFlangerModComponent,gSoundFlangerModDesc , "Flanger Effect",  "FlangerEffect", COMP_TYPE_AUDIO, Class_ID(0x25034090, 0x361a08d7) )
OBSOLETE_CLASS(plSoundGargleModComponent,gSoundGargleModDesc , "Gargle Effect",  "GargleEffect", COMP_TYPE_AUDIO, Class_ID(0x639b6a41, 0x24da2462))
OBSOLETE_CLASS(plSoundReverbModComponent,gSoundReverbModDesc , "Reverb Effect",  "ReverbEffect", COMP_TYPE_AUDIO, Class_ID(0x1bef33fc, 0x5c763858))



#if 1 // Waiting... mf
/////////////////////////////////////////////////////////////////////////////////////////////////
//
//	RandomSound Component
//
//

plKey plAudioComp::GetRandomSoundKey(plComponentBase *comp, plMaxNode *node)
{
	if (comp->ClassID() == RANDOM_SOUND_COMPONENT_ID)
	{
		plRandomSoundComponent *rndSnd = (plRandomSoundComponent*)comp;
		if (rndSnd->fSoundMods.find(node) != rndSnd->fSoundMods.end())
			return rndSnd->fSoundMods[node]->GetKey();
	}

	return nil;
}

/////////////////////////////////////////////////////////////////////////////////////////
enum
{
	kAutoStart,
	kSelectMode,
	kDelayMode,
	kMinDelay,
	kMaxDelay,
	kUseAll,
	kGroupIdx,
	kSoundList,
	kGroupTotals,
	kLastPick,
	kCombineSounds
};

enum
{
	kNormal = 0,
	kNoRepeats,
	kFullSetRepeat,
	kFullSetStop,
	kSequential
};

enum
{
	kDelayFromStart = 0,
	kDelayFromEnd,
	kDelayInfinite
};

enum
{
	kRandomSoundMain,
	kRandomSoundGroup,
};

static const kMaxGroups = 10;

class plRandomSoundComponentProc : public ParamMap2UserDlgProc
{
public:
	plRandomSoundComponentProc() {}

	BOOL DlgProc(TimeValue t, IParamMap2 *pm, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	void DeleteThis() {}
	void UpdateDisplay(IParamMap2 *pm);
	virtual void Update(TimeValue t, Interval& valid, IParamMap2* pmap) { UpdateDisplay(pmap); }
};

static plRandomSoundComponentProc gRandomSoundComponentProc;

void plRandomSoundComponentProc::UpdateDisplay(IParamMap2 *pm)
{
	HWND hWnd = pm->GetHWnd();
	HWND hList = GetDlgItem(hWnd, IDC_COMP_RS_GROUPLIST);
	IParamBlock2 *pb = pm->GetParamBlock();
	plRandomSoundComponent *comp = (plRandomSoundComponent *)pb->GetOwner();
	
	ListBox_ResetContent(hList);
	int group = comp->GetCurGroupIdx();
	int startIdx = comp->GetStartIndex(group);
	int endIdx = comp->GetEndIndex(group);

	while (startIdx < endIdx)
	{
		INode *curNode = pb->GetINode(ParamID(kSoundList), 0, startIdx);
		if (curNode == nil)
		{
			comp->RemoveSound(startIdx);
			endIdx--;
			continue;
		}
		ListBox_AddString(hList, curNode->GetName());
		startIdx++;
	}
}

BOOL plRandomSoundComponentProc::DlgProc(TimeValue t, IParamMap2 *pm, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	IParamBlock2 *pb = pm->GetParamBlock();
	HWND hList = GetDlgItem(hWnd, IDC_COMP_RS_GROUPLIST);
	plRandomSoundComponent *comp = (plRandomSoundComponent *)pb->GetOwner();

	switch (msg)
	{
	case WM_INITDIALOG:
		//UpdateDisplay(pm);
		return TRUE;

	case WM_COMMAND:
		if (HIWORD(wParam) == BN_CLICKED)
		{
			if (LOWORD(wParam) == IDC_COMP_RS_GROUP_ADD)
			{
				std::vector<Class_ID> cids;
				cids.push_back(SOUND_3D_COMPONENT_ID);
				if (plPick::NodeRefKludge(pb, kLastPick, &cids, true, false))			
					comp->AddSelectedSound();

				return TRUE;
			}
			// Remove the currently selected material
			else if (LOWORD(wParam) == IDC_COMP_RS_GROUP_REMOVE)
			{
				int curSel = ListBox_GetCurSel(hList);
				if (curSel >= 0)
					comp->RemoveSound(curSel);

				return TRUE;
			}
		}
	}

	return FALSE;
}

//Max desc stuff necessary below.
CLASS_DESC(plRandomSoundComponent, gRandomSoundDesc, "Random Sound",  "RandomSound", COMP_TYPE_AUDIO, RANDOM_SOUND_COMPONENT_ID)

//
// Block not necessary, kept for backwards compat.
//

ParamBlockDesc2 gRandomSoundBk
(
 plComponent::kBlkComp, _T("RandomSound"), 0, &gRandomSoundDesc, P_AUTO_CONSTRUCT + P_AUTO_UI + P_MULTIMAP, plComponent::kRefComp,

 	2, 
	kRandomSoundMain, IDD_COMP_RANDOMSOUND, IDS_COMP_RANDOMSOUNDS, 0, 0, NULL,
	kRandomSoundGroup, IDD_COMP_RANDOMSOUND_GROUPS, IDS_COMP_RANDOMSOUNDS_GROUPS, 0, APPENDROLL_CLOSED, &gRandomSoundComponentProc,

	// Main rollout
	kAutoStart,  _T("AutoStart"), TYPE_BOOL, 		0, 0,
		p_default,	TRUE,
		p_ui, kRandomSoundMain, TYPE_SINGLECHEKBOX, IDC_COMP_RS_AUTOSTART,
		end,

	kSelectMode,	_T("SelectMode"),		TYPE_INT, 		0, 0,
		p_ui, kRandomSoundMain, TYPE_RADIO, 5,	IDC_RADIO_RS_NORMAL,	IDC_RADIO_RS_NOREP,	IDC_RADIO_RS_FSREP,	IDC_RADIO_RS_FSSTOP, IDC_RADIO_RS_SEQ,
		end,

	kDelayMode,	_T("DelayMode"),		TYPE_INT, 		0, 0,
		p_ui, kRandomSoundMain, TYPE_RADIO, 3,	IDC_RADIO_RS_DELAYSTART,	IDC_RADIO_RS_DELAYEND,	IDC_RADIO_RS_DELAYNEVER, 
		end,

	kMinDelay,		_T("MinDelay"),		TYPE_FLOAT, 	0, 0,	
		p_default, 0.0,
		p_range, -500.0, 1000.0,
		p_ui, kRandomSoundMain, TYPE_SPINNER,	EDITTYPE_FLOAT,	
		IDC_COMP_RS_DELAYMIN, IDC_COMP_RS_DELAYMIN_SPIN, 1.0,
		end,

	kMaxDelay,		_T("MaxDelay"),		TYPE_FLOAT, 	0, 0,	
		p_default, 0.0,
		p_range, -500.0, 1000.0,
		p_ui, kRandomSoundMain, TYPE_SPINNER,	EDITTYPE_FLOAT,	
		IDC_COMP_RS_DELAYMAX, IDC_COMP_RS_DELAYMAX_SPIN, 0.1,
		end,

	// Group rollout
	kUseAll,	_T("UseAll"),	TYPE_BOOL,	0, 0,
		p_default,	TRUE,
		p_ui, kRandomSoundGroup, TYPE_SINGLECHEKBOX, IDC_COMP_RS_USEALL,
		end,

	kGroupIdx,	_T("GroupIndex"),	TYPE_INT,	0, 0,
		p_default, 1,
		p_range, 1, kMaxGroups,
		p_ui, kRandomSoundGroup, TYPE_SPINNER,	EDITTYPE_INT,
		IDC_COMP_RS_GROUP, IDC_COMP_RS_GROUP_SPIN, 1.f,
		end,

	kSoundList, _T("Sounds"), TYPE_INODE_TAB, 0,	0, 0,
		end,

	kGroupTotals, _T("Totals"), TYPE_INT_TAB, kMaxGroups,	0, 0,
		p_default, 0,
		end,

	kLastPick, _T("LastPick"), TYPE_INODE,	0, 0, // Temp storage space for the comp picker
		end,

	kCombineSounds, _T("combineSounds"), TYPE_BOOL, 0, 0,
		p_default, FALSE,
		p_ui, kRandomSoundGroup, TYPE_SINGLECHEKBOX, IDC_RAND_COMBINESOUNDS,
		end,

	end
);

plRandomSoundComponent::plRandomSoundComponent()
{
	fClassDesc = &gRandomSoundDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

int plRandomSoundComponent::GetCurGroupIdx()
{
	return fCompPB->GetInt(ParamID(kGroupIdx)) - 1;
}

int plRandomSoundComponent::GetStartIndex(int group)
{
	int result = 0;
	int i;
	for (i = 0; i < group; i++)
		result += fCompPB->GetInt(ParamID(kGroupTotals), 0, i);

	return result;
}

int plRandomSoundComponent::GetEndIndex(int group)
{
	return GetStartIndex(group) + fCompPB->GetInt(ParamID(kGroupTotals), 0, group);
}

void plRandomSoundComponent::AddSelectedSound()
{
	int group = GetCurGroupIdx();
	int soundIdx = GetEndIndex(group);

	INode *node = fCompPB->GetINode(ParamID(kLastPick));
	fCompPB->Insert(ParamID(kSoundList), soundIdx, 1, &node);

	fCompPB->SetValue(ParamID(kGroupTotals), 0, fCompPB->GetInt(ParamID(kGroupTotals), 0, group) + 1, group);
}

void plRandomSoundComponent::RemoveSound(int index)
{
	int group = GetCurGroupIdx();
	int soundIdx = GetStartIndex(group) + index;
	
	fCompPB->Delete(ParamID(kSoundList), soundIdx, 1);
	fCompPB->SetValue(ParamID(kGroupTotals), 0, fCompPB->GetInt(ParamID(kGroupTotals), 0, group) - 1, group);
}

hsBool plRandomSoundComponent::ICheckForSounds(plMaxNode* node)
{
	if (!node->CanConvert())
		return false;

	int nSounds = 0;
	UInt32 numComp = node->NumAttachedComponents(false);
	for(int i = 0; i < numComp; i++)
	{
		plComponentBase* comp = node->GetAttachedComponent(i);
		if (plAudioComp::IsSoundComponent(comp))
			nSounds++;
	}

	return nSounds > 0;
}

hsBool plRandomSoundComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	if( !ICheckForSounds(node) )
	{
		// Warning that there's no sounds to be played?
		return true;
	}

	plRandomSoundMod* mod = fSoundMods[node];
	plSound *pSound = nil;
	const plAudioInterface* ai = nil;
	plWinAudible* pAudible = nil;


	if( fCompPB->GetInt((ParamID)kAutoStart) )
		mod->SetState(0);
	else
		mod->SetState(plRandomSoundMod::kStopped);

	UInt8 mode = plRandomSoundMod::kNormal;
	switch( fCompPB->GetInt((ParamID)kSelectMode) )
	{
	// random, repeats okay, play until stopped				- Normal
	case kNormal:
		mode = plRandomSoundMod::kNormal;
		break;
	// random, no repeats, play until stopped				- NoRepeats
	case kNoRepeats:
		mode = plRandomSoundMod::kNoRepeats;
		break;
	// random, play full cycle before repeating				- FullSetRepeat
	case kFullSetRepeat:
		mode = plRandomSoundMod::kCoverall | plRandomSoundMod::kNoRepeats;
		break;
	// random, play full cycle, then stop					- FullSetStop
	case kFullSetStop:
		mode = plRandomSoundMod::kCoverall | plRandomSoundMod::kOneCycle | plRandomSoundMod::kNoRepeats;
		break;
	case kSequential:
		mode = plRandomSoundMod::kSequential;
		break;
	}

	switch( fCompPB->GetInt((ParamID)kDelayMode) )
	{
	case kDelayFromStart:
		break;
	case kDelayFromEnd:
		mode |= plRandomSoundMod::kDelayFromEnd;
		break;
	case kDelayInfinite:
		mode |= plRandomSoundMod::kOneCmd;
		break;
	}

	mod->SetMode(mode);

	float minDel = fCompPB->GetFloat((ParamID)kMinDelay);
	float maxDel = fCompPB->GetFloat((ParamID)kMaxDelay);
	if( minDel > maxDel )
	{
		float t = maxDel;
		maxDel = minDel;
		minDel = t;
	}

	mod->SetMinDelay(minDel);
	mod->SetMaxDelay(maxDel);

	node->AddModifier(mod, IGetUniqueName(node));


	if (!fCompPB->GetInt(ParamID(kUseAll))) // Actually using separate groups
	{
		ai = node->GetSceneObject()->GetAudioInterface();
		pAudible = (plWinAudible*)ai->GetAudible();
		hsTArray<plBaseSoundEmitterComponent *> comps;

		plRandomSoundModGroup *groups = TRACKED_NEW plRandomSoundModGroup[kMaxGroups];
		int i;
		int numSoFar = 0;
		for (i = 0; i < kMaxGroups; i++)
		{
			int numSounds = fCompPB->GetInt(ParamID(kGroupTotals), 0, i);
			if( numSounds == 0 )
			{
				groups[i].fGroupedIdx = -1; 
				groups[i].fNumSounds = 0;
				groups[i].fIndices = nil;
				continue;
			}

			groups[i].fIndices = TRACKED_NEW UInt16[numSounds];

			hsTArray<UInt16> indices;
			int j;

			if( !fCompPB->GetInt( (ParamID)kCombineSounds ) )
			{
				for (j = 0; j < numSounds; j++)
				{
					plMaxNode *compNode = (plMaxNode*)fCompPB->GetINode(ParamID(kSoundList), 0, numSoFar + j);
					if (compNode)
					{
						plBaseSoundEmitterComponent *comp = (plBaseSoundEmitterComponent *)compNode->ConvertToComponent();
						int idx = comp->GetSoundIdx((plMaxNode*)node);
						if (idx >= 0)
						{
							indices.Append(idx);
						}
					}
				}
				groups[i].fNumSounds = indices.GetCount();
				for (j = 0; j < indices.GetCount(); j++)
				{	
					groups[i].fIndices[j] = indices[j];
				}
			}
			else
			{
				// Build array of components to give to ConvertGrouped()
				for (j = 0; j < numSounds; j++)
				{
					plMaxNode *compNode = (plMaxNode*)fCompPB->GetINode(ParamID(kSoundList), 0, numSoFar + j);
					if (compNode)
					{
						plBaseSoundEmitterComponent *comp = (plBaseSoundEmitterComponent *)compNode->ConvertToComponent();
						comps.Append( comp );
						// Stupid, i know. Leave me alone, PG is playing.
						indices.Append( comps.GetCount() - 1 );
					}
				}

				// Get index from first (should be the same for all of 'em)
				groups[i].fGroupedIdx = comps[ 0 ]->GetSoundIdx( (plMaxNode *)node );
				groups[i].fNumSounds = indices.GetCount();
				for (j = 0; j < indices.GetCount(); j++)
				{
					groups[i].fIndices[j] = indices[ j ];
				}
			}
				
			numSoFar += groups[i].fNumSounds;
		}
		mod->SetGroupInfo(kMaxGroups, groups);

		if( fCompPB->GetInt( (ParamID)kCombineSounds ) )
		{
			// Convert (use pointer to first comp so we get the virtual call)
			if( !comps[ 0 ]->ConvertGrouped( node, comps, pErrMsg ) )
			{
				return false;
			}
		}
	}

	// Non-grouped random sounds - give priority to each sound
	else
	{
		ai = node->GetSceneObject()->GetAudioInterface();
		pAudible = (plWinAudible*)ai->GetAudible();
		int numSounds = pAudible->GetNumSounds();
		
		if(numSounds == 0) return true;
		
		pSound = pAudible->GetSound(0);	// Get sound ptr
		int highestPriority = pSound->GetPriority();

		// Distance to lowest priority
		int distToLowest = 9 - highestPriority;
		if( distToLowest <= 0) distToLowest = 1;	// just incase

		for( int i = 0; i < numSounds; i++)
		{
			pSound = pAudible->GetSound(i);	// Get sound ptr

			// Give the first random sound highest priority
			if(i == 0)
				pSound->SetPriority(highestPriority);

			else
			{
				pSound->SetPriority(highestPriority+((i-1)%distToLowest)+1);
			}
		}
	}
	
	return true;
}

hsBool plRandomSoundComponent::PreConvert(plMaxNode *pNode,  plErrorMsg *pErrMsg)
{
	if (ICheckForSounds(pNode))
	{
		plRandomSoundMod* mod = TRACKED_NEW plRandomSoundMod;
		hsgResMgr::ResMgr()->NewKey(IGetUniqueName(pNode), mod, pNode->GetLocation());
		fSoundMods[pNode] = mod;
	}

	return true;
}

// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
hsBool plRandomSoundComponent::SetupProperties(plMaxNode *pNode,  plErrorMsg *pErrMsg)
{
	fSoundMods.clear();

	// Tell some (all?) of the sound components we point to that they're going to be 
	// grouped sounds instead
	if( fCompPB->GetInt( (ParamID)kCombineSounds ) )
	{
		if (!fCompPB->GetInt(ParamID(kUseAll))) // Actually using separate groups
		{
			// Get a sound index to assign to all the components, since they get the same one as a grouped sound
			int idx = pNode->GetNextSoundIdx();

			int i, numSoFar = 0;
			for (i = 0; i < kMaxGroups; i++)
			{
				int numSounds = fCompPB->GetInt(ParamID(kGroupTotals), 0, i);

				if( numSounds <= 0 )
					continue;

				int j;
				for (j = 0; j < numSounds; j++)
				{
					plMaxNode *compNode = (plMaxNode*)fCompPB->GetINode(ParamID(kSoundList), 0, numSoFar + j);
					if (compNode)
					{
						plBaseSoundEmitterComponent *comp = (plBaseSoundEmitterComponent *)compNode->ConvertToComponent();
						comp->SetCreateGrouped( pNode, idx );
					}
				}
				numSoFar += numSounds;
			}
		}
	}
	
	return true;
}



#endif // Waiting... mf




/////////////////////////////////////////////////////////////////////////////////////////////////
/// Physics Sound Group Component ///////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

class plPhysicsSndGroupCompProc;
class plPhysicsSndGroupComp : public plComponent
{
protected:

	friend class plPhysicsSndGroupCompProc;

public:
	plPhysicsSndGroupComp();
	void DeleteThis() { delete this; }

	hsBool SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg);
	hsBool PreConvert(plMaxNode *pNode, plErrorMsg *pErrMsg);
	hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);

	enum Refs
	{
		kRefGroup,
		kRefImpactSoundsOld,
		kRefSlideSoundsOld,
		kRefDummyPickNode,
		kRefImpactSounds,
		kRefSlideSounds,
	};
};

class plPhysicsSndGroupCompProc : public ParamMap2UserDlgProc
{
public:
	plPhysicsSndGroupCompProc() {}

	BOOL DlgProc(TimeValue t, IParamMap2 *pm, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	void DeleteThis() {}
	virtual void Update(TimeValue t, Interval& valid, IParamMap2* pmap) {  }

protected:

	void	IInitList( HWND hList, int currSel, hsBool allowAll )
	{
		int		i, toSet = -1;
		struct plSndGrp
		{
			char name[ 64 ];
			int group;
		} groups[] = {  { "Metal",  plPhysicalSndGroup::kMetal },
						{ "Grass",  plPhysicalSndGroup::kGrass },
						{ "Wood",   plPhysicalSndGroup::kWood },
						{ "Stone",  plPhysicalSndGroup::kWood + 1 },
						{ "Water",  plPhysicalSndGroup::kWood + 2 },
						{ "Bone",   plPhysicalSndGroup::kWood + 3 },
						{ "Dirt",   plPhysicalSndGroup::kWood + 4 },
						{ "Rug",    plPhysicalSndGroup::kWood + 5 },
						{ "Cone",   plPhysicalSndGroup::kWood + 6 },
						{ "User 1", plPhysicalSndGroup::kWood + 7 },
						{ "User 2", plPhysicalSndGroup::kWood + 8 },
						{ "User 3", plPhysicalSndGroup::kWood + 9 },
						{ "", plPhysicalSndGroup::kNone } };

		SendMessage( hList, CB_RESETCONTENT, 0, 0 );
		
		if( allowAll )
		{
			int idx = SendMessage( hList, CB_ADDSTRING, 0, (LPARAM)"* All *" );
			SendMessage( hList, CB_SETITEMDATA, idx, (LPARAM)-1 );
			if( currSel == -1 )
				toSet = idx;
		}

		for( i = 0; groups[ i ].group != plPhysicalSndGroup::kNone; i++ )
		{
			int idx = SendMessage( hList, CB_ADDSTRING, 0, (LPARAM)groups[ i ].name );
			SendMessage( hList, CB_SETITEMDATA, idx, (LPARAM)groups[ i ].group );

			if( groups[ i ].group == currSel )
				toSet = idx;
		}

		if( toSet != -1 )
			SendMessage( hList, CB_SETCURSEL, toSet, 0 );
	}

	void	IUpdateBtns( HWND hWnd, int idx, plPhysicsSndGroupComp *comp )
	{
		// Update da buttons
		if( idx == -1 )
			idx = 0;

		INode *impact = IGet( comp->GetParamBlock( 0 ), plPhysicsSndGroupComp::kRefImpactSounds, idx );
		::SetWindowText( GetDlgItem( hWnd, IDC_SND_IMPACT ), ( impact != nil ) ? impact->GetName() : "<none>" );
		
		INode *slide = IGet( comp->GetParamBlock( 0 ), plPhysicsSndGroupComp::kRefSlideSounds, idx );
		::SetWindowText( GetDlgItem( hWnd, IDC_SND_SLIDE ), ( slide != nil ) ? slide->GetName() : "<none>" );
	}
	
	void	ISet( IParamBlock2 *pb, ParamID which, int idx, INode *node )
	{
		if( pb->Count( which ) <= idx )
		{
			pb->SetCount( (ParamID)which, idx + 1 );
			pb->Resize( (ParamID)which, idx + 1 );
		}

		if( idx == -1 )
		{
			pb->SetCount( (ParamID)which, plPhysicalSndGroup::kWood + 9 );
			pb->Resize( which, plPhysicalSndGroup::kWood + 9 );
			int i;
			for( i = 0; i < plPhysicalSndGroup::kWood + 9; i++ )
				pb->SetValue( which, 0, node, i );
		}
		else
			pb->SetValue( which, 0, node, idx );
	}

	INode	*IGet( IParamBlock2 *pb, ParamID which, int idx )
	{
		if( pb->Count( which ) <= idx )
			return nil;
			
		return pb->GetINode( which, 0, idx );
	}
};

static plPhysicsSndGroupCompProc gPhysicsSndGroupCompProc;


BOOL plPhysicsSndGroupCompProc::DlgProc(TimeValue t, IParamMap2 *pm, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	IParamBlock2 *pb = pm->GetParamBlock();
	HWND hList = GetDlgItem( hWnd, IDC_SND_GROUP );
	HWND hAgainst = GetDlgItem( hWnd, IDC_SND_AGAINST );
	plPhysicsSndGroupComp *comp = (plPhysicsSndGroupComp *)pb->GetOwner();

	switch( msg )
	{
		case WM_INITDIALOG:
			{
				IInitList( GetDlgItem( hWnd, IDC_SND_GROUP ), pb->GetInt( plPhysicsSndGroupComp::kRefGroup ), false );
				IInitList( GetDlgItem( hWnd, IDC_SND_AGAINST ), -1, true );

				int idx = SendMessage( hAgainst, CB_GETCURSEL, 0, 0 );
				if( idx != CB_ERR )
				{
					idx = (int)SendMessage( hAgainst, CB_GETITEMDATA, idx, 0 );
					IUpdateBtns( hWnd, idx, comp );
				}
			}

			return TRUE;

		case WM_COMMAND:
			if( HIWORD( wParam ) == CBN_SELCHANGE )
			{
				if( LOWORD( wParam ) == IDC_SND_GROUP )
				{
					int idx = SendMessage( hList, CB_GETCURSEL, 0, 0 );
					if( idx != CB_ERR )
					{
						pb->SetValue( (ParamID)plPhysicsSndGroupComp::kRefGroup, 0, (int)SendMessage( hList, CB_GETITEMDATA, idx, 0 ) );
					}
					return true;
				}
				else if( LOWORD( wParam ) == IDC_SND_AGAINST )
				{
					int idx = SendMessage( hAgainst, CB_GETCURSEL, 0, 0 );
					if( idx != CB_ERR )
					{
						idx = (int)SendMessage( hAgainst, CB_GETITEMDATA, idx, 0 );
						IUpdateBtns( hWnd, idx, comp );
					}
				}
			}
			else if( LOWORD( wParam ) == IDC_SND_CLEAR_IMPACT )
			{
				int idx = SendMessage( hAgainst, CB_GETCURSEL, 0, 0 );
				if( idx != CB_ERR )
				{
					idx = (int)SendMessage( hAgainst, CB_GETITEMDATA, idx, 0 );
					if( idx == -1 )
					{
						pb->Resize( (ParamID)plPhysicsSndGroupComp::kRefImpactSounds, 0 ); 
					}
					else
						ISet( pb, plPhysicsSndGroupComp::kRefImpactSounds, idx, nil );
					IUpdateBtns( hWnd, idx, comp );
				}
			}
			else if( LOWORD( wParam ) == IDC_SND_CLEAR_SLIDE )
			{
				int idx = SendMessage( hAgainst, CB_GETCURSEL, 0, 0 );
				if( idx != CB_ERR )
				{
					idx = (int)SendMessage( hAgainst, CB_GETITEMDATA, idx, 0 );
					if( idx == -1 )
						pb->Resize( (ParamID)plPhysicsSndGroupComp::kRefSlideSounds, 0 ); 
					else
						ISet( pb, plPhysicsSndGroupComp::kRefSlideSounds, idx, nil );
					IUpdateBtns( hWnd, idx, comp );
				}
			}
			else if( LOWORD( wParam ) == IDC_SND_IMPACT )
			{
				int idx = SendMessage( hAgainst, CB_GETCURSEL, 0, 0 );
				if( idx != CB_ERR )
				{
					idx = (int)SendMessage( hAgainst, CB_GETITEMDATA, idx, 0 );

					std::vector<Class_ID> cids;
					cids.push_back( RANDOM_SOUND_COMPONENT_ID );
					if( plPick::NodeRefKludge( pb, plPhysicsSndGroupComp::kRefDummyPickNode, &cids, true, false ) )
						ISet( comp->GetParamBlock( 0 ), plPhysicsSndGroupComp::kRefImpactSounds, idx, pb->GetINode( plPhysicsSndGroupComp::kRefDummyPickNode ) );
					
					IUpdateBtns( hWnd, idx, comp );
				}
			}
			else if( LOWORD( wParam ) == IDC_SND_SLIDE )
			{
				int idx = SendMessage( hAgainst, CB_GETCURSEL, 0, 0 );
				if( idx != CB_ERR )
				{
					idx = (int)SendMessage( hAgainst, CB_GETITEMDATA, idx, 0 );
					
					std::vector<Class_ID> cids;
					cids.push_back( RANDOM_SOUND_COMPONENT_ID );
					if( plPick::NodeRefKludge( pb, plPhysicsSndGroupComp::kRefDummyPickNode, &cids, true, false ) )				
						ISet( pb, plPhysicsSndGroupComp::kRefSlideSounds, idx, pb->GetINode( plPhysicsSndGroupComp::kRefDummyPickNode ) );
						
					IUpdateBtns( hWnd, idx, comp );
				}
			}

	}

	return FALSE;
}

// Simple accessor
class plPhysicsSndGroupAccessor : public PBAccessor
{
public:
	void Set( PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t )
	{
		if( id == plPhysicsSndGroupComp::kRefImpactSounds || id == plPhysicsSndGroupComp::kRefSlideSounds )
		{
			plPhysicsSndGroupComp *comp = (plPhysicsSndGroupComp *)owner;
			comp->NotifyDependents( FOREVER, PART_ALL, REFMSG_USER_COMP_REF_CHANGED );
		}
	}
};
static plPhysicsSndGroupAccessor	glPhysicsSndGroupAccessor;

//Max desc stuff necessary below.
CLASS_DESC(plPhysicsSndGroupComp, gPhysSndGrpDesc, "Physics Sound Group",  "PhysSndGroup", COMP_TYPE_AUDIO, SOUND_PHYS_COMP_ID)

ParamBlockDesc2 gPhysSndGrpBk
(
	plComponent::kBlkComp, _T("PhysSndGroup"), 0, &gPhysSndGrpDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	IDD_COMP_SOUNDPHYS, IDS_COMP_SOUNDPHYS, 0, 0, &gPhysicsSndGroupCompProc,

	plPhysicsSndGroupComp::kRefGroup,  _T("Group"), TYPE_INT, 		0, 0,
		p_default,	(int)plPhysicalSndGroup::kNone,
		end,

	plPhysicsSndGroupComp::kRefDummyPickNode, _T( "Dummy" ), TYPE_INODE, 0, 0,
		end,
		
	plPhysicsSndGroupComp::kRefImpactSounds,  _T("Impacts"), TYPE_INODE_TAB, 		0, 0, 0,
//		p_accessor, glPhysicsSndGroupAccessor,
		end,

	plPhysicsSndGroupComp::kRefSlideSounds,  _T("Slides"), TYPE_INODE_TAB, 		0, 0, 0,
//		p_accessor, glPhysicsSndGroupAccessor,
		end,

	end
);

plPhysicsSndGroupComp::plPhysicsSndGroupComp() 
{
	fClassDesc = &gPhysSndGrpDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

hsBool plPhysicsSndGroupComp::Convert( plMaxNode *node, plErrorMsg *pErrMsg )
{
	plMaxNode *pNode;
	plKey RandSoundKey;

	// Try to grab the SI from the current scene object. This'll have the pointer we want
	plSceneObject *obj = node->GetSceneObject();
	if( obj != nil )
	{
		const plSimulationInterface* si = obj->GetSimulationInterface();
		if (si)
		{
			// Create a new sound group
			plPhysicalSndGroup *grp = TRACKED_NEW plPhysicalSndGroup( fCompPB->GetInt( (ParamID)kRefGroup ) );
			hsgResMgr::ResMgr()->NewKey( IGetUniqueName( node ), grp, node->GetLocation(), node->GetLoadMask() );

			// Convert each sound into a plWin32StaticSound and store onto the sound group
			int i;
			for( i = 0; i < fCompPB->Count( (ParamID)kRefImpactSounds ); i++ )
			{
				plMaxNode *targNode = (plMaxNode *)fCompPB->GetINode( (ParamID)kRefImpactSounds, 0, i );
				if( targNode != nil )
				{
					plComponentBase *comp = targNode->ConvertToComponent();
					if( comp != nil )
					{
						// Check root node for random sound component
						RandSoundKey = plAudioComp::GetRandomSoundKey( comp, node );
						if(RandSoundKey)
							grp->AddImpactSound( i, RandSoundKey );
						
						// If not in root node check children
						else
						{
							for(int j = 0; j < node->NumChildren(); j++)
							{
								pNode = (plMaxNode *)node->GetChildNode(j);
								RandSoundKey = plAudioComp::GetRandomSoundKey( comp, pNode );
								if(!RandSoundKey) continue;

								grp->AddImpactSound( i, RandSoundKey );
								break;
							}
						}
					}
				}
			}

			for( i = 0; i < fCompPB->Count( (ParamID)kRefSlideSounds ); i++ )
			{
				plMaxNode *targNode = (plMaxNode *)fCompPB->GetINode( (ParamID)kRefSlideSounds, 0, i );
				if( targNode != nil )
				{
					plComponentBase *comp = targNode->ConvertToComponent();
					if( comp != nil )
					{
						// Check root node for random sound component
						RandSoundKey = plAudioComp::GetRandomSoundKey( comp, node );
						if(RandSoundKey)
							grp->AddSlideSound( i, RandSoundKey );
						else
						{
							for(int j = 0; j < node->NumChildren(); j++)
							{
								pNode = (plMaxNode *)node->GetChildNode(j);
								RandSoundKey = plAudioComp::GetRandomSoundKey( comp, pNode );
								if(!RandSoundKey) continue;

								grp->AddSlideSound( i, RandSoundKey );
								break;
							}
						}
					}
				}
			}

			// Attach the sound group to the physical
			hsgResMgr::ResMgr()->AddViaNotify( grp->GetKey(), TRACKED_NEW plGenRefMsg( si->GetPhysical()->GetKey(), plRefMsg::kOnCreate, 0, plPXPhysical::kPhysRefSndGroup ), plRefFlags::kActiveRef );
		}
	}

	return true;
}

hsBool plPhysicsSndGroupComp::PreConvert( plMaxNode *pNode,  plErrorMsg *pErrMsg )
{
	return true;
}

hsBool plPhysicsSndGroupComp::SetupProperties( plMaxNode *pNode,  plErrorMsg *pErrMsg )
{
	return true;
}

