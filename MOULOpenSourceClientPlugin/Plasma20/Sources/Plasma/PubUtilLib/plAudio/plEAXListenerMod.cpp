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
//	plEAXListenerMod														//
//																			//
//////////////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "plEAXListenerMod.h"
#include "../plIntersect/plSoftVolume.h"
#include "hsResMgr.h"
#include "plgDispatch.h"
#include "plAudioSystem.h"
#include "../pnMessage/plAudioSysMsg.h" 

#include <eax-util.h>


plEAXListenerMod::plEAXListenerMod()
{
	fListenerProps = TRACKED_NEW EAXREVERBPROPERTIES;
	fSoftRegion = nil;
	fRegistered = false;
	fGetsMessages = false;

	memcpy( fListenerProps, &REVERB_ORIGINAL_PRESETS[ ORIGINAL_GENERIC ], sizeof( EAXREVERBPROPERTIES ) );
}

plEAXListenerMod::~plEAXListenerMod()
{
	// Tell the audio sys we're going away
	IUnRegister();

	// Unregister for audioSys messages
	if( fGetsMessages )
	{
		plgDispatch::Dispatch()->UnRegisterForExactType( plAudioSysMsg::Index(), GetKey() );
		fGetsMessages = false;
	}

	delete fListenerProps;
}

void	plEAXListenerMod::IRegister( void )
{
	if( !fGetsMessages )
	{
		plgDispatch::Dispatch()->RegisterForExactType( plAudioSysMsg::Index(), GetKey() );
		fGetsMessages = true;
	}

	if( fRegistered || GetKey() == nil )
		return;

	plKey sysKey = hsgResMgr::ResMgr()->FindKey( plUoid( kAudioSystem_KEY ) );
	if( sysKey != nil )
	{
		plGenRefMsg *refMsg = TRACKED_NEW plGenRefMsg( sysKey, plRefMsg::kOnCreate, 0, plAudioSystem::kRefEAXRegion );
		hsgResMgr::ResMgr()->AddViaNotify( GetKey(), refMsg, plRefFlags::kPassiveRef );
		fRegistered = true;
	}
}

void	plEAXListenerMod::IUnRegister( void )
{
	if( !fRegistered || GetKey() == nil )
		return;

	plKey sysKey = hsgResMgr::ResMgr()->FindKey( plUoid( kAudioSystem_KEY ) );
	if( sysKey != nil && GetKey() != nil )
		sysKey->Release( GetKey() );

	fRegistered = false;
}

hsBool plEAXListenerMod::IEval( double secs, hsScalar del, UInt32 dirty )
{
	IRegister();
	return false;
}

hsBool	plEAXListenerMod::MsgReceive( plMessage* pMsg )
{
	plGenRefMsg *refMsg = plGenRefMsg::ConvertNoRef( pMsg );
	if( refMsg != nil )
	{
		switch( refMsg->fType )
		{
			case kRefSoftRegion:
				if( refMsg->GetContext() & ( plRefMsg::kOnCreate | plRefMsg::kOnRequest | plRefMsg::kOnReplace ) )
				{
					fSoftRegion = plSoftVolume::ConvertNoRef( refMsg->GetRef() );
					fSoftRegion->SetCheckListener();
				}
				else if( refMsg->GetContext() & ( plRefMsg::kOnRemove | plRefMsg::kOnDestroy ) )
				{
					fSoftRegion = nil;
				}
				break;
		}
	}

	plAudioSysMsg *sysMsg = plAudioSysMsg::ConvertNoRef( pMsg );
	if( sysMsg != nil )
	{
		if( sysMsg->GetAudFlag() == plAudioSysMsg::kActivate )
		{
			IRegister();
		}
		else if( sysMsg->GetAudFlag() == plAudioSysMsg::kDeActivate )
		{
			IUnRegister();
		}

		return true;
	}

	return plSingleModifier::MsgReceive( pMsg );
}
		
void plEAXListenerMod::Read( hsStream* s, hsResMgr* mgr )
{
	plSingleModifier::Read( s, mgr );

	// Read in the soft region
	mgr->ReadKeyNotifyMe( s, TRACKED_NEW plGenRefMsg( GetKey(), plRefMsg::kOnCreate, 0, kRefSoftRegion ), plRefFlags::kActiveRef );

	// Read the listener params
	fListenerProps->ulEnvironment = s->ReadSwap32();
	fListenerProps->flEnvironmentSize = s->ReadSwapFloat();
	fListenerProps->flEnvironmentDiffusion = s->ReadSwapFloat();
	fListenerProps->lRoom = s->ReadSwap32();
	fListenerProps->lRoomHF = s->ReadSwap32();
	fListenerProps->lRoomLF = s->ReadSwap32();
	fListenerProps->flDecayTime = s->ReadSwapFloat();
	fListenerProps->flDecayHFRatio = s->ReadSwapFloat();
	fListenerProps->flDecayLFRatio = s->ReadSwapFloat();
	fListenerProps->lReflections = s->ReadSwap32();
	fListenerProps->flReflectionsDelay = s->ReadSwapFloat();
	//fListenerProps->vReflectionsPan;     // early reflections panning vector
	fListenerProps->lReverb = s->ReadSwap32();                  // late reverberation level relative to room effect
	fListenerProps->flReverbDelay = s->ReadSwapFloat();
	//fListenerProps->vReverbPan;          // late reverberation panning vector
	fListenerProps->flEchoTime = s->ReadSwapFloat();
	fListenerProps->flEchoDepth = s->ReadSwapFloat();
	fListenerProps->flModulationTime = s->ReadSwapFloat();
	fListenerProps->flModulationDepth = s->ReadSwapFloat();
	fListenerProps->flAirAbsorptionHF = s->ReadSwapFloat();
	fListenerProps->flHFReference = s->ReadSwapFloat();
	fListenerProps->flLFReference = s->ReadSwapFloat();
	fListenerProps->flRoomRolloffFactor = s->ReadSwapFloat();
	fListenerProps->ulFlags = s->ReadSwap32();

	// Done reading, time to tell the audio sys we exist
	IRegister();
}

void plEAXListenerMod::Write( hsStream* s, hsResMgr* mgr )
{
	plSingleModifier::Write( s, mgr );

	// Write the soft region key
	mgr->WriteKey( s, fSoftRegion );

	// Write the listener params
	s->WriteSwap32( fListenerProps->ulEnvironment );
	s->WriteSwapFloat( fListenerProps->flEnvironmentSize );
	s->WriteSwapFloat( fListenerProps->flEnvironmentDiffusion );
	s->WriteSwap32( fListenerProps->lRoom );
	s->WriteSwap32( fListenerProps->lRoomHF );
	s->WriteSwap32( fListenerProps->lRoomLF );
	s->WriteSwapFloat( fListenerProps->flDecayTime );
	s->WriteSwapFloat( fListenerProps->flDecayHFRatio );
	s->WriteSwapFloat( fListenerProps->flDecayLFRatio );
	s->WriteSwap32( fListenerProps->lReflections );
	s->WriteSwapFloat( fListenerProps->flReflectionsDelay );
	//s->WriteSwapFloat( fListenerProps->vReflectionsPan;     // early reflections panning vector
	s->WriteSwap32( fListenerProps->lReverb );                  // late reverberation level relative to room effect
	s->WriteSwapFloat( fListenerProps->flReverbDelay );
	//s->WriteSwapFloat( fListenerProps->vReverbPan;          // late reverberation panning vector
	s->WriteSwapFloat( fListenerProps->flEchoTime );
	s->WriteSwapFloat( fListenerProps->flEchoDepth );
	s->WriteSwapFloat( fListenerProps->flModulationTime );
	s->WriteSwapFloat( fListenerProps->flModulationDepth );
	s->WriteSwapFloat( fListenerProps->flAirAbsorptionHF );
	s->WriteSwapFloat( fListenerProps->flHFReference );
	s->WriteSwapFloat( fListenerProps->flLFReference );
	s->WriteSwapFloat( fListenerProps->flRoomRolloffFactor );
	s->WriteSwap32( fListenerProps->ulFlags );
}


void	plEAXListenerMod::SetFromPreset( UInt32 preset )
{
	memcpy( fListenerProps, &REVERB_ORIGINAL_PRESETS[ preset ], sizeof( EAXREVERBPROPERTIES ) );
}

float	plEAXListenerMod::GetStrength( void )
{
	if( fSoftRegion == nil )
		return 0.f;

	return fSoftRegion->GetListenerStrength();
}
