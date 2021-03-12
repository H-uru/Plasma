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

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//  plEAXListenerMod                                                        //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifndef EAX_SDK_AVAILABLE
#include "plEAXStructures.h"
#endif
#include "HeadSpin.h"
#include "plEAXListenerMod.h"
#include "plIntersect/plSoftVolume.h"
#include "hsResMgr.h"
#include "plgDispatch.h"
#include "plAudioSystem.h"
#include "pnMessage/plAudioSysMsg.h"
#include "pnMessage/plRefMsg.h"

#ifdef EAX_SDK_AVAILABLE
#include <eax-util.h>
#endif


plEAXListenerMod::plEAXListenerMod()
    : fSoftRegion(), fRegistered(), fGetsMessages()
{
    fListenerProps = new EAXREVERBPROPERTIES;

#ifdef EAX_SDK_AVAILABLE
    memcpy( fListenerProps, &REVERB_ORIGINAL_PRESETS[ ORIGINAL_GENERIC ], sizeof( EAXREVERBPROPERTIES ) );
#endif
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

void    plEAXListenerMod::IRegister()
{
    if( !fGetsMessages )
    {
        plgDispatch::Dispatch()->RegisterForExactType( plAudioSysMsg::Index(), GetKey() );
        fGetsMessages = true;
    }

    if (fRegistered || GetKey() == nullptr)
        return;

    plKey sysKey = hsgResMgr::ResMgr()->FindKey( plUoid( kAudioSystem_KEY ) );
    if (sysKey != nullptr)
    {
        plGenRefMsg *refMsg = new plGenRefMsg( sysKey, plRefMsg::kOnCreate, 0, 0 );
        hsgResMgr::ResMgr()->AddViaNotify( GetKey(), refMsg, plRefFlags::kPassiveRef );
        fRegistered = true;
    }
}

void    plEAXListenerMod::IUnRegister()
{
    if (!fRegistered || GetKey() == nullptr)
        return;

    plKey sysKey = hsgResMgr::ResMgr()->FindKey( plUoid( kAudioSystem_KEY ) );
    if (sysKey != nullptr && GetKey() != nullptr)
        sysKey->Release( GetKey() );

    fRegistered = false;
}

bool plEAXListenerMod::IEval( double secs, float del, uint32_t dirty )
{
    IRegister();
    return false;
}

bool    plEAXListenerMod::MsgReceive( plMessage* pMsg )
{
    plGenRefMsg *refMsg = plGenRefMsg::ConvertNoRef( pMsg );
    if (refMsg != nullptr)
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
                    fSoftRegion = nullptr;
                }
                break;
        }
    }

    plAudioSysMsg *sysMsg = plAudioSysMsg::ConvertNoRef( pMsg );
    if (sysMsg != nullptr)
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
    mgr->ReadKeyNotifyMe( s, new plGenRefMsg( GetKey(), plRefMsg::kOnCreate, 0, kRefSoftRegion ), plRefFlags::kActiveRef );

    // Read the listener params
    fListenerProps->ulEnvironment = s->ReadLE32();
    fListenerProps->flEnvironmentSize = s->ReadLEFloat();
    fListenerProps->flEnvironmentDiffusion = s->ReadLEFloat();
    fListenerProps->lRoom = s->ReadLE32();
    fListenerProps->lRoomHF = s->ReadLE32();
    fListenerProps->lRoomLF = s->ReadLE32();
    fListenerProps->flDecayTime = s->ReadLEFloat();
    fListenerProps->flDecayHFRatio = s->ReadLEFloat();
    fListenerProps->flDecayLFRatio = s->ReadLEFloat();
    fListenerProps->lReflections = s->ReadLE32();
    fListenerProps->flReflectionsDelay = s->ReadLEFloat();
    //fListenerProps->vReflectionsPan;     // early reflections panning vector
    fListenerProps->lReverb = s->ReadLE32();                  // late reverberation level relative to room effect
    fListenerProps->flReverbDelay = s->ReadLEFloat();
    //fListenerProps->vReverbPan;          // late reverberation panning vector
    fListenerProps->flEchoTime = s->ReadLEFloat();
    fListenerProps->flEchoDepth = s->ReadLEFloat();
    fListenerProps->flModulationTime = s->ReadLEFloat();
    fListenerProps->flModulationDepth = s->ReadLEFloat();
    fListenerProps->flAirAbsorptionHF = s->ReadLEFloat();
    fListenerProps->flHFReference = s->ReadLEFloat();
    fListenerProps->flLFReference = s->ReadLEFloat();
    fListenerProps->flRoomRolloffFactor = s->ReadLEFloat();
    fListenerProps->ulFlags = s->ReadLE32();

    // Done reading, time to tell the audio sys we exist
    IRegister();
}

void plEAXListenerMod::Write( hsStream* s, hsResMgr* mgr )
{
    plSingleModifier::Write( s, mgr );

    // Write the soft region key
    mgr->WriteKey( s, fSoftRegion );

    // Write the listener params
    s->WriteLE32((uint32_t)fListenerProps->ulEnvironment);
    s->WriteLEFloat( fListenerProps->flEnvironmentSize );
    s->WriteLEFloat( fListenerProps->flEnvironmentDiffusion );
    s->WriteLE32((int32_t)fListenerProps->lRoom);
    s->WriteLE32((int32_t)fListenerProps->lRoomHF);
    s->WriteLE32((int32_t)fListenerProps->lRoomLF);
    s->WriteLEFloat( fListenerProps->flDecayTime );
    s->WriteLEFloat( fListenerProps->flDecayHFRatio );
    s->WriteLEFloat( fListenerProps->flDecayLFRatio );
    s->WriteLE32((int32_t)fListenerProps->lReflections);
    s->WriteLEFloat( fListenerProps->flReflectionsDelay );
    //s->WriteLEFloat( fListenerProps->vReflectionsPan;     // early reflections panning vector
    s->WriteLE32((int32_t)fListenerProps->lReverb);         // late reverberation level relative to room effect
    s->WriteLEFloat( fListenerProps->flReverbDelay );
    //s->WriteLEFloat( fListenerProps->vReverbPan;          // late reverberation panning vector
    s->WriteLEFloat( fListenerProps->flEchoTime );
    s->WriteLEFloat( fListenerProps->flEchoDepth );
    s->WriteLEFloat( fListenerProps->flModulationTime );
    s->WriteLEFloat( fListenerProps->flModulationDepth );
    s->WriteLEFloat( fListenerProps->flAirAbsorptionHF );
    s->WriteLEFloat( fListenerProps->flHFReference );
    s->WriteLEFloat( fListenerProps->flLFReference );
    s->WriteLEFloat( fListenerProps->flRoomRolloffFactor );
    s->WriteLE32((uint32_t)fListenerProps->ulFlags);
}


void    plEAXListenerMod::SetFromPreset( uint32_t preset )
{
#ifdef EAX_SDK_AVAILABLE
    memcpy( fListenerProps, &REVERB_ORIGINAL_PRESETS[ preset ], sizeof( EAXREVERBPROPERTIES ) );
#endif
}

float   plEAXListenerMod::GetStrength()
{
    if (fSoftRegion == nullptr)
        return 0.f;

    return fSoftRegion->GetListenerStrength();
}
