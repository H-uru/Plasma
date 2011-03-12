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
//////////////////////////////////////////////////////////////////////////////
//																			//
//	plEAXEffects - Various classes and wrappers to support EAX				//
//					acceleration.											//
//																			//
//////////////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "hsThread.h"
#include "plEAXEffects.h"
#include "../plAudioCore/plAudioCore.h"
#include "plDSoundBuffer.h"
#include "hsTemplates.h"
#include "plEAXListenerMod.h"
#include "hsStream.h"
#include "plAudioSystem.h"
#include <al.h>

#include <dmusici.h>
#include <dxerr9.h>
#include <eax.h>
#include <eax-util.h>
#include <eaxlegacy.h>
#include "../plStatusLog/plStatusLog.h"

#define kDebugLog	if( myLog != nil ) myLog->AddLineF(

static EAXGet			s_EAXGet;
static EAXSet			s_EAXSet;


//// GetInstance /////////////////////////////////////////////////////////////

plEAXListener	&plEAXListener::GetInstance( void )
{
	static plEAXListener	instance;
	return instance;
}

//// Constructor/Destructor //////////////////////////////////////////////////

plEAXListener::plEAXListener()
{
	fInited = false;
	ClearProcessCache();
}

plEAXListener::~plEAXListener()
{
	Shutdown();
}

//// Init ////////////////////////////////////////////////////////////////////

hsBool	plEAXListener::Init( void )
{
	if( fInited )
		return true;

	if(!alIsExtensionPresent((ALchar *)"EAX4.0"))		// is eax 4 supported
	{
		if(!alIsExtensionPresent((ALchar *) "EAX4.0Emulated"))		// is an earlier version of eax supported
		{
			plStatusLog::AddLineS("audio.log", "EAX not supported");
			return false;
		}
		else
		{
			plStatusLog::AddLineS("audio.log", "EAX 4 Emulated supported");
		}
	}
	else
	{
		plStatusLog::AddLineS("audio.log", "EAX 4 available");
	}
	
	// EAX is supported 
	s_EAXGet = (EAXGet)alGetProcAddress((ALchar *)"EAXGet");
	s_EAXSet = (EAXSet)alGetProcAddress((ALchar *)"EAXSet");
	if(!s_EAXGet || ! s_EAXSet)
	{
		IFail( "EAX initialization failed", true );
		return false;
	}
	fInited = true;

#if 1
	try
	{
		// Make an EAX call here to prevent problems on WDM driver
		unsigned int lRoom = -10000;

		SetGlobalEAXProperty(DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_ROOM, &lRoom, sizeof( unsigned int ));
	}
	catch ( ... )
	{
		plStatusLog::AddLineS("audio.log", "Unable to set EAX Property Set, disabling EAX...");
		plgAudioSys::EnableEAX(false);
		return false;
	}
#endif

	ClearProcessCache();

	return true;
}

//// Shutdown ////////////////////////////////////////////////////////////////

void	plEAXListener::Shutdown( void )
{
	if( !fInited )
		return;

	s_EAXSet = nil;
	s_EAXGet = nil;
	IRelease();
}


bool plEAXListener::SetGlobalEAXProperty(GUID guid, unsigned long ulProperty, void *pData, unsigned long ulDataSize )
{
	if(fInited)
	{
		return s_EAXSet(&guid, ulProperty, 0, pData, ulDataSize) == AL_NO_ERROR;
	}
	return false;
}

bool plEAXListener::GetGlobalEAXProperty(GUID guid, unsigned long ulProperty, void *pData, unsigned long ulDataSize)
{
	if(fInited)
	{
		return s_EAXGet(&guid, ulProperty, 0, pData, ulDataSize) == AL_NO_ERROR;
	}
	return false;
}

bool plEAXSource::SetSourceEAXProperty(unsigned source, GUID guid, unsigned long ulProperty, void *pData, unsigned long ulDataSize)
{
	return s_EAXSet(&guid, ulProperty, source, pData, ulDataSize) == AL_NO_ERROR;
}

bool plEAXSource::GetSourceEAXProperty(unsigned source, GUID guid, unsigned long ulProperty, void *pData, unsigned long ulDataSize)
{
	return s_EAXGet(&guid, ulProperty, source, pData, ulDataSize) == AL_NO_ERROR;
}


//// IRelease ////////////////////////////////////////////////////////////////

void	plEAXListener::IRelease( void )
{
	fInited = false;
}

//// IFail ///////////////////////////////////////////////////////////////////

void	plEAXListener::IFail(  hsBool major )
{
	plStatusLog::AddLineS( "audio.log", plStatusLog::kRed,
							"ERROR in plEAXListener: Could not set global eax params");

	if( major )
		IRelease();
}

void	plEAXListener::IFail( const char *msg, hsBool major )
{
	plStatusLog::AddLineS( "audio.log", plStatusLog::kRed,
							"ERROR in plEAXListener: %s", msg );

	if( major )
		IRelease();
}

//// IMuteProperties /////////////////////////////////////////////////////////
//	Mutes the given properties, so if you have some props that you want
//	half strength, this function will do it for ya.

void	plEAXListener::IMuteProperties( EAXLISTENERPROPERTIES *props, hsScalar percent )
{
	// We only mute the room, roomHF and roomLF, since those control the overall effect
	// application. All three are a direct linear blend as defined by eax-util.cpp, so
	// this should be rather easy

	hsScalar invPercent = 1.f - percent;

	// The old way, as dictated by EAX sample code...
	props->lRoom   = (int)( ( (float)EAXLISTENER_MINROOM   * invPercent ) + ( (float)props->lRoom   * percent ) );
	// The new way, as suggested by EAX guys...
//	props->lRoom = (int)( 2000.f * log( invPercent ) ) + props->lRoom;

//	props->lRoomLF = (int)( ( (float)EAXLISTENER_MINROOMLF * invPercent ) + ( (float)props->lRoomLF * percent ) );
//	props->lRoomHF = (int)( ( (float)EAXLISTENER_MINROOMHF * invPercent ) + ( (float)props->lRoomHF * percent ) );
}

//// ClearProcessCache ///////////////////////////////////////////////////////
//	Clears the cache settings used to speed up ProcessMods(). Call whenever
//	the list of mods changed.

void	plEAXListener::ClearProcessCache( void )
{
	fLastBigRegion = nil;
	fLastModCount = -1;
	fLastWasEmpty = false;
	fLastSingleStrength = -1.f;
}

//// ProcessMods /////////////////////////////////////////////////////////////
//	9.13.02 mcn - Updated the caching method. Now fLastBigRegion will point
//	to a region iff it's the only region from the last pass that had a
//	strength > 0. The reason we can't do our trick before is because even if
//	we have a region w/ strength 1, another region might power up from 1 and
//	thus suddenly alter the total reverb settings. Thus, the only time we 
//	can wisely skip is if our current big region == fLastBigRegion *and*
//	the total strength is the same.

void	plEAXListener::ProcessMods( hsTArray<plEAXListenerMod *> &modArray )
{
	int		i;
	float	totalStrength;
	hsBool	firstOne;

	plEAXListenerMod		*thisBigRegion = nil;
	EAXLISTENERPROPERTIES	finalProps;
	static int oldTime = timeGetTime();		// Get starting time
	int newTime;
	hsBool bMorphing = false;

	static plStatusLog	*myLog = nil;

	if( myLog == nil && plgAudioSys::AreExtendedLogsEnabled() )
		myLog = plStatusLogMgr::GetInstance().CreateStatusLog( 30, "EAX Reverbs", plStatusLog::kFilledBackground | plStatusLog::kDeleteForMe | plStatusLog::kDontWriteFile );
	else if( myLog != nil && !plgAudioSys::AreExtendedLogsEnabled() )
	{
		delete myLog;
		myLog = nil;
	}

	if( myLog != nil )
		myLog->Clear();

	if( modArray.GetCount() != fLastModCount )
	{
		kDebugLog "Clearing cache..." );
		ClearProcessCache();	// Code path changed, clear the entire cache
		fLastModCount = modArray.GetCount();
	}
	else
	{
		kDebugLog "" );
	}

	if( modArray.GetCount() > 0 )
	{
		kDebugLog "%d regions to calc", modArray.GetCount() );

		// Reset and find a new one if applicable
		thisBigRegion = nil;

		// Accumulate settings from all the active listener regions (shouldn't be too many, we hope)
		totalStrength = 0.f;
		firstOne = true;
		for( i = 0; i < modArray.GetCount(); i++ )
		{
			float strength = modArray[ i ]->GetStrength();
			kDebugLog "%4.2f - %s", strength, modArray[ i ]->GetKey()->GetUoid().GetObjectName() );
			if( strength > 0.f )
			{
				// fLastBigRegion will point to a region iff it's the only region w/ strength > 0
				if( totalStrength == 0.f )
					thisBigRegion = modArray[ i ];
				else
					thisBigRegion = nil;

				if( firstOne )
				{
					// First one, just init to it
					memcpy( &finalProps, modArray[ i ]->GetListenerProps(), sizeof( finalProps ) );
					totalStrength = strength;
					firstOne = false;
				}
				else
				{
					hsScalar scale = strength / ( totalStrength + strength );
					EAX3ListenerInterpolate( &finalProps, modArray[ i ]->GetListenerProps(), scale, &finalProps, false );
					totalStrength += strength;
					bMorphing = true;
				}
				
				if( totalStrength >= 1.f )
					break;
			}
		}

		if( firstOne )
		{
			// No regions of strength > 0, so just make it quiet
			kDebugLog "Reverb should be quiet" );
			if( fLastWasEmpty )
				return;

			memcpy( &finalProps, &EAX30_ORIGINAL_PRESETS[ ORIGINAL_GENERIC ], sizeof( EAXLISTENERPROPERTIES ) );
			finalProps.lRoom = EAXLISTENER_MINROOM;
//			finalProps.lRoomLF = EAXLISTENER_MINROOMLF;
//			finalProps.lRoomHF = EAXLISTENER_MINROOMHF;
			fLastWasEmpty = true;
			fLastBigRegion = nil;
			fLastSingleStrength = -1.f;
		}
		else 
		{
			fLastWasEmpty = false;

			if( thisBigRegion == fLastBigRegion && totalStrength == fLastSingleStrength )
				// Cached values should be the same, so we can bail at this point
				return;

			fLastBigRegion = thisBigRegion;
			fLastSingleStrength = ( thisBigRegion != nil ) ? totalStrength : -1.f;

			if( totalStrength < 1.f )
			{
				kDebugLog "Total strength < 1; muting result" );
				// All of them together is less than full strength, so mute our result
				IMuteProperties( &finalProps, totalStrength );
			}
		}
	}
	else
	{
		kDebugLog "No regions at all; disabling reverb" );
		// No regions whatsoever, so disable listener props entirely
		if( fLastWasEmpty )
			return;

		memcpy( &finalProps, &EAX30_ORIGINAL_PRESETS[ ORIGINAL_GENERIC ], sizeof( EAXLISTENERPROPERTIES ) );
		finalProps.lRoom = EAXLISTENER_MINROOM;
//		finalProps.lRoomLF = EAXLISTENER_MINROOMLF;
//		finalProps.lRoomHF = EAXLISTENER_MINROOMHF;
		fLastWasEmpty = true;
	}

	// if were morphing between regions, do 10th of a second check, otherwise just let it 
	// change due to opt out(caching) feature.
	if(bMorphing)
	{
		newTime = timeGetTime();

		// Update, at most, ten times per second
		if((newTime - oldTime) < 100) return;
			
		oldTime = newTime;		// update time
	}
//finalProps.flAirAbsorptionHF *= 0.3048f; // Convert to feet
	//kDebugLog "** Updating property set **" );


	if(!SetGlobalEAXProperty(DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_ALLPARAMETERS, &finalProps, sizeof( finalProps )))
	{
		IFail(  false );
	}
}


//////////////////////////////////////////////////////////////////////////////
//// Source Settings /////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//// Constructor/Destructor //////////////////////////////////////////////////

plEAXSourceSettings::plEAXSourceSettings()
{
	fDirtyParams = kAll;
	Enable( false );
}

plEAXSourceSettings::~plEAXSourceSettings()
{
}

//// Read/Write/Set //////////////////////////////////////////////////////////

void	plEAXSourceSettings::Read( hsStream *s )
{
	fEnabled = s->ReadBool();
	if( fEnabled )
	{
		fRoom = s->ReadSwap16();
		fRoomHF = s->ReadSwap16();
		fRoomAuto = s->ReadBool();
		fRoomHFAuto = s->ReadBool();

		fOutsideVolHF = s->ReadSwap16();
		
		fAirAbsorptionFactor = s->ReadSwapFloat();
		fRoomRolloffFactor = s->ReadSwapFloat();
		fDopplerFactor = s->ReadSwapFloat();
		fRolloffFactor = s->ReadSwapFloat();

		fSoftStarts.Read( s );
		fSoftEnds.Read( s );

		fOcclusionSoftValue = -1.f;
		SetOcclusionSoftValue( s->ReadSwapFloat() );

		fDirtyParams = kAll;
	}
	else
		Enable( false );	// Force init of params
}

void	plEAXSourceSettings::Write( hsStream *s )
{
	s->WriteBool( fEnabled );
	if( fEnabled )
	{
		s->WriteSwap16( fRoom );
		s->WriteSwap16( fRoomHF );
		s->WriteBool( fRoomAuto );
		s->WriteBool( fRoomHFAuto );

		s->WriteSwap16( fOutsideVolHF );
		
		s->WriteSwapFloat( fAirAbsorptionFactor );
		s->WriteSwapFloat( fRoomRolloffFactor );
		s->WriteSwapFloat( fDopplerFactor );
		s->WriteSwapFloat( fRolloffFactor );

		fSoftStarts.Write( s );
		fSoftEnds.Write( s );

		s->WriteSwapFloat( fOcclusionSoftValue );
	}
}

void	plEAXSourceSettings::SetRoomParams( Int16 room, Int16 roomHF, hsBool roomAuto, hsBool roomHFAuto )
{
	fRoom = room;
	fRoomHF = roomHF;
	fRoomAuto = roomAuto;
	fRoomHFAuto = roomHFAuto;
	fDirtyParams |= kRoom;
}

void	plEAXSourceSettings::Enable( hsBool e )
{
	fEnabled = e;
	if( !e )
	{
		fRoom = EAXBUFFER_MINROOM;
		fRoomHF = EAXBUFFER_MINROOMHF;
		fRoomAuto = true;
		fRoomHFAuto = true;

		fOutsideVolHF = 0;

		fAirAbsorptionFactor = 1.f;
		fRoomRolloffFactor = 0.f;
		fDopplerFactor = 0.f;
		fRolloffFactor = 0.f;

		fOcclusionSoftValue = 0.f;
		fSoftStarts.Reset();
		fSoftEnds.Reset();
		fCurrSoftValues.Reset();
		fDirtyParams = kAll;
	}
}

void	plEAXSourceSettings::SetOutsideVolHF( Int16 vol )
{
	fOutsideVolHF = vol;
	fDirtyParams |= kOutsideVolHF;
}

void	plEAXSourceSettings::SetFactors( hsScalar airAbsorption, hsScalar roomRolloff, hsScalar doppler, hsScalar rolloff )
{
	fAirAbsorptionFactor = airAbsorption;
	fRoomRolloffFactor = roomRolloff;
	fDopplerFactor = doppler;
	fRolloffFactor = rolloff;
	fDirtyParams |= kFactors;
}

void	plEAXSourceSettings::SetOcclusionSoftValue( hsScalar value )
{
	if( fOcclusionSoftValue != value )
	{
		fOcclusionSoftValue = value;
		IRecalcSofts( kOcclusion );
		fDirtyParams |= kOcclusion;
	}
}

void	plEAXSourceSettings::IRecalcSofts( UInt8 whichOnes )
{
	hsScalar	percent, invPercent;

	if( whichOnes & kOcclusion )
	{
		percent = fOcclusionSoftValue;
		invPercent = 1.f - percent;

		Int16		occ	= (Int16)( ( (float)fSoftStarts.GetOcclusion() * invPercent ) + ( (float)fSoftEnds.GetOcclusion() * percent ) );
		hsScalar	lfRatio = (hsScalar)( ( fSoftStarts.GetOcclusionLFRatio() * invPercent ) + ( fSoftEnds.GetOcclusionLFRatio() * percent ) );
		hsScalar	roomRatio = (hsScalar)( ( fSoftStarts.GetOcclusionRoomRatio() * invPercent ) + ( fSoftEnds.GetOcclusionRoomRatio() * percent ) );
		hsScalar	directRatio = (hsScalar)( ( fSoftStarts.GetOcclusionDirectRatio() * invPercent ) + ( fSoftEnds.GetOcclusionDirectRatio() * percent ) );

		fCurrSoftValues.SetOcclusion( occ, lfRatio, roomRatio, directRatio );
	}
}


//////////////////////////////////////////////////////////////////////////////
//// Source Soft Settings ////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

void	plEAXSourceSoftSettings::Reset( void )
{
	fOcclusion = 0;
	fOcclusionLFRatio = 0.25f;
	fOcclusionRoomRatio = 1.5f;
	fOcclusionDirectRatio = 1.f;
}

void	plEAXSourceSoftSettings::Read( hsStream *s )
{
	s->ReadSwap( &fOcclusion );
	s->ReadSwap( &fOcclusionLFRatio );
	s->ReadSwap( &fOcclusionRoomRatio );
	s->ReadSwap( &fOcclusionDirectRatio );
}

void	plEAXSourceSoftSettings::Write( hsStream *s )
{
	s->WriteSwap( fOcclusion );
	s->WriteSwap( fOcclusionLFRatio );
	s->WriteSwap( fOcclusionRoomRatio );
	s->WriteSwap( fOcclusionDirectRatio );
}

void	plEAXSourceSoftSettings::SetOcclusion( Int16 occ, hsScalar lfRatio, hsScalar roomRatio, hsScalar directRatio )
{
	fOcclusion = occ;
	fOcclusionLFRatio = lfRatio;
	fOcclusionRoomRatio = roomRatio;
	fOcclusionDirectRatio = directRatio;
}

//// Constructor/Destructor //////////////////////////////////////////////////

plEAXSource::plEAXSource()
{	
	fInit = false;
	
}

plEAXSource::~plEAXSource()
{
	Release();
}

//// Init/Release ////////////////////////////////////////////////////////////

void	plEAXSource::Init( plDSoundBuffer *parent )
{
	fInit = true;
	// Init some default params
	plEAXSourceSettings	defaultParams;
	SetFrom( &defaultParams, parent->GetSource() );
}

void	plEAXSource::Release( void )
{
	fInit = false;
}

hsBool	plEAXSource::IsValid( void ) const
{
	return true;
}

//// SetFrom /////////////////////////////////////////////////////////////////

void	plEAXSource::SetFrom( plEAXSourceSettings *settings, unsigned source, hsBool force )
{
	UInt32 dirtyParams;
	if(source == 0 || !fInit) 
		return;

	if( force )
		dirtyParams = plEAXSourceSettings::kAll;
	else
		dirtyParams = settings->fDirtyParams;
	
	// Do the params
	if( dirtyParams & plEAXSourceSettings::kRoom )
	{
		SetSourceEAXProperty(source, DSPROPSETID_EAX_BufferProperties, DSPROPERTY_EAXBUFFER_ROOM, &settings->fRoom, sizeof(settings->fRoom));
		SetSourceEAXProperty(source, DSPROPSETID_EAX_BufferProperties, DSPROPERTY_EAXBUFFER_ROOMHF, &settings->fRoomHF, sizeof(settings->fRoomHF));
	}

	if( dirtyParams & plEAXSourceSettings::kOutsideVolHF )
	{
		SetSourceEAXProperty(source, DSPROPSETID_EAX_BufferProperties, DSPROPERTY_EAXBUFFER_OUTSIDEVOLUMEHF, &settings->fOutsideVolHF, sizeof(settings->fOutsideVolHF));
	}
	
	if( dirtyParams & plEAXSourceSettings::kFactors )
	{
		SetSourceEAXProperty(source, DSPROPSETID_EAX_BufferProperties, DSPROPERTY_EAXBUFFER_DOPPLERFACTOR, &settings->fDopplerFactor, sizeof(settings->fDopplerFactor));
		SetSourceEAXProperty(source, DSPROPSETID_EAX_BufferProperties, DSPROPERTY_EAXBUFFER_ROLLOFFFACTOR, &settings->fRolloffFactor, sizeof(settings->fRolloffFactor));
		SetSourceEAXProperty(source, DSPROPSETID_EAX_BufferProperties, DSPROPERTY_EAXBUFFER_ROOMROLLOFFFACTOR, &settings->fRoomRolloffFactor, sizeof(settings->fRoomRolloffFactor));
		SetSourceEAXProperty(source, DSPROPSETID_EAX_BufferProperties, DSPROPERTY_EAXBUFFER_AIRABSORPTIONFACTOR, &settings->fAirAbsorptionFactor, sizeof(settings->fAirAbsorptionFactor));
	}

	if( dirtyParams & plEAXSourceSettings::kOcclusion )
	{
		SetSourceEAXProperty(source, DSPROPSETID_EAX_BufferProperties, DSPROPERTY_EAXBUFFER_OCCLUSION, &settings->GetCurrSofts().fOcclusion, sizeof(settings->GetCurrSofts().fOcclusion));
		SetSourceEAXProperty(source, DSPROPSETID_EAX_BufferProperties, DSPROPERTY_EAXBUFFER_OCCLUSIONLFRATIO, &settings->GetCurrSofts().fOcclusionLFRatio, sizeof(settings->GetCurrSofts().fOcclusionLFRatio));
		SetSourceEAXProperty(source, DSPROPSETID_EAX_BufferProperties, DSPROPERTY_EAXBUFFER_OCCLUSIONROOMRATIO, &settings->GetCurrSofts().fOcclusionRoomRatio, sizeof(settings->GetCurrSofts().fOcclusionRoomRatio));
		SetSourceEAXProperty(source, DSPROPSETID_EAX_BufferProperties, DSPROPERTY_EAXBUFFER_OCCLUSIONDIRECTRATIO, &settings->GetCurrSofts().fOcclusionDirectRatio, sizeof(settings->GetCurrSofts().fOcclusionDirectRatio));
	}

	settings->ClearDirtyParams();

	// Do all the flags in one pass
	DWORD	flags;
	

	if( GetSourceEAXProperty( source, DSPROPSETID_EAX_BufferProperties, DSPROPERTY_EAXBUFFER_FLAGS, &flags, sizeof( DWORD )) ) 
	{
		if( settings->GetRoomAuto() )
			flags |= EAXBUFFERFLAGS_ROOMAUTO;
		else
			flags &= ~EAXBUFFERFLAGS_ROOMAUTO;

		if( settings->GetRoomHFAuto() )
			flags |= EAXBUFFERFLAGS_ROOMHFAUTO;
		else
			flags &= ~EAXBUFFERFLAGS_ROOMHFAUTO;

		if( SetSourceEAXProperty( source, DSPROPSETID_EAX_BufferProperties, DSPROPERTY_EAXBUFFER_FLAGS, &flags, sizeof( DWORD ) ) ) 
		{
			return;	// All worked, return here
		}	
		
		// Flag setting failed somehow
		hsAssert( false, "Unable to set EAX buffer flags" );
	}
}


