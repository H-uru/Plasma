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

#include "HeadSpin.h"
#include "plEAXListenerMod.h"
#include "plEAXStructures.h"
#include "plIntersect/plSoftVolume.h"
#include "hsResMgr.h"
#include "plgDispatch.h"
#include "plAudioSystem.h"
#include "pnMessage/plAudioSysMsg.h"
#include "pnMessage/plRefMsg.h"

#include <math.h>


plEAXListenerMod::plEAXListenerMod()
    : fListenerProps(new EFXEAXREVERBPROPERTIES),
    fSoftRegion(nullptr), fRegistered(false), fGetsMessages(false)
{
    *fListenerProps = EFX_REVERB_PRESET_GENERIC;
}

plEAXListenerMod::~plEAXListenerMod()
{
    // Tell the audio sys we're going away
    IUnRegister();

    // Unregister for audioSys messages
    if(fGetsMessages) {
        plgDispatch::Dispatch()->UnRegisterForExactType(plAudioSysMsg::Index(), GetKey());
        fGetsMessages = false;
    }

    delete fListenerProps;
}

void plEAXListenerMod::IRegister()
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

void plEAXListenerMod::IUnRegister()
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

    EAXREVERBPROPERTIES* eaxListenerProps = new EAXREVERBPROPERTIES;

    // Read the listener params
    eaxListenerProps->ulEnvironment = s->ReadLE32();
    eaxListenerProps->flEnvironmentSize = s->ReadLEFloat();
    eaxListenerProps->flEnvironmentDiffusion = s->ReadLEFloat();
    eaxListenerProps->lRoom = s->ReadLEFloat();
    eaxListenerProps->lRoomHF = s->ReadLEFloat();
    eaxListenerProps->lRoomLF = s->ReadLEFloat();
    eaxListenerProps->flDecayTime = s->ReadLEFloat();
    eaxListenerProps->flDecayHFRatio = s->ReadLEFloat();
    eaxListenerProps->flDecayLFRatio = s->ReadLEFloat();
    eaxListenerProps->lReflections = s->ReadLEFloat();
    eaxListenerProps->flReflectionsDelay = s->ReadLEFloat();
    //eaxListenerProps->vReflectionsPan;     // early reflections panning vector
    eaxListenerProps->lReverb = s->ReadLEFloat();                  // late reverberation level relative to room effect
    eaxListenerProps->flReverbDelay = s->ReadLEFloat();
    //eaxListenerProps->vReverbPan;          // late reverberation panning vector
    eaxListenerProps->flEchoTime = s->ReadLEFloat();
    eaxListenerProps->flEchoDepth = s->ReadLEFloat();
    eaxListenerProps->flModulationTime = s->ReadLEFloat();
    eaxListenerProps->flModulationDepth = s->ReadLEFloat();
    eaxListenerProps->flAirAbsorptionHF = s->ReadLEFloat();
    eaxListenerProps->flHFReference = s->ReadLEFloat();
    eaxListenerProps->flLFReference = s->ReadLEFloat();
    eaxListenerProps->flRoomRolloffFactor = s->ReadLEFloat();
    eaxListenerProps->ulFlags = s->ReadLE32();

    ConvertEAXToEFX(eaxListenerProps, fListenerProps);
    delete eaxListenerProps;

    // Done reading, time to tell the audio sys we exist
    IRegister();
}

void plEAXListenerMod::Write( hsStream* s, hsResMgr* mgr )
{
    plSingleModifier::Write( s, mgr );

    // Write the soft region key
    mgr->WriteKey( s, fSoftRegion );


    EAXREVERBPROPERTIES* eaxListenerProps = new EAXREVERBPROPERTIES;
    ConvertEFXToEAX(fListenerProps, eaxListenerProps);

    // Write the listener params
    s->WriteLE32((uint32_t)eaxListenerProps->ulEnvironment);
    s->WriteLEFloat(eaxListenerProps->flEnvironmentSize);
    s->WriteLEFloat(eaxListenerProps->flEnvironmentDiffusion);
    s->WriteLE32((int32_t)eaxListenerProps->lRoom);
    s->WriteLE32((int32_t)eaxListenerProps->lRoomHF);
    s->WriteLE32((int32_t)eaxListenerProps->lRoomLF);
    s->WriteLEFloat(eaxListenerProps->flDecayTime);
    s->WriteLEFloat(eaxListenerProps->flDecayHFRatio);
    s->WriteLEFloat(eaxListenerProps->flDecayLFRatio);
    s->WriteLE32((int32_t)eaxListenerProps->lReflections);
    s->WriteLEFloat(eaxListenerProps->flReflectionsDelay);
    //s->WriteLEFloat( eaxListenerProps->vReflectionsPan;     // early reflections panning vector
    s->WriteLE32((int32_t)eaxListenerProps->lReverb);         // late reverberation level relative to room effect
    s->WriteLEFloat(eaxListenerProps->flReverbDelay);
    //s->WriteLEFloat( eaxListenerProps->vReverbPan;          // late reverberation panning vector
    s->WriteLEFloat(eaxListenerProps->flEchoTime);
    s->WriteLEFloat(eaxListenerProps->flEchoDepth);
    s->WriteLEFloat(eaxListenerProps->flModulationTime);
    s->WriteLEFloat(eaxListenerProps->flModulationDepth);
    s->WriteLEFloat(eaxListenerProps->flAirAbsorptionHF);
    s->WriteLEFloat(eaxListenerProps->flHFReference);
    s->WriteLEFloat(eaxListenerProps->flLFReference);
    s->WriteLEFloat(eaxListenerProps->flRoomRolloffFactor);
    s->WriteLE32((uint32_t)eaxListenerProps->ulFlags);

    delete eaxListenerProps;
}

void plEAXListenerMod::SetFromEFXPreset(EFXEAXREVERBPROPERTIES preset)
{
    *fListenerProps = preset;
}

float plEAXListenerMod::GetStrength()
{
    if (fSoftRegion == nullptr)
        return 0.f;

    return fSoftRegion->GetListenerStrength();
}


/* Helper functions for converting to EFX from deprecated EAX values */

inline float mB_to_gain(float mb)
{
    return powf(10.0f, mb / 2000.0f);
}

inline float gain_to_mB(float gain)
{
    return 2000.0f * log10f(gain);
}

void plEAXListenerMod::ConvertEAXToEFX(const EAXREVERBPROPERTIES* eax,
    EFXEAXREVERBPROPERTIES* efx)
{
    float density;

    density = powf(eax->flEnvironmentSize, 3.0f) / 16.0f;
    efx->flDensity = (density > 1.0f) ? 1.0f : density;
    efx->flDiffusion = eax->flEnvironmentDiffusion;
    efx->flGain = mB_to_gain(eax->lRoom);
    efx->flGainHF = mB_to_gain(eax->lRoomHF);
    efx->flGainLF = mB_to_gain(eax->lRoomLF);
    efx->flDecayTime = eax->flDecayTime;
    efx->flDecayHFRatio = eax->flDecayHFRatio;
    efx->flDecayLFRatio = eax->flDecayLFRatio;
    efx->flReflectionsGain = mB_to_gain(eax->lReflections);
    efx->flReflectionsDelay = eax->flReflectionsDelay;
    efx->flReflectionsPan[0] = eax->vReflectionsPan.x;
    efx->flReflectionsPan[1] = eax->vReflectionsPan.y;
    efx->flReflectionsPan[2] = eax->vReflectionsPan.z;
    efx->flLateReverbGain = mB_to_gain(eax->lReverb);
    efx->flLateReverbDelay = eax->flReverbDelay;
    efx->flLateReverbPan[0] = eax->vReverbPan.x;
    efx->flLateReverbPan[1] = eax->vReverbPan.y;
    efx->flLateReverbPan[2] = eax->vReverbPan.z;
    efx->flEchoTime = eax->flEchoTime;
    efx->flEchoDepth = eax->flEchoDepth;
    efx->flModulationTime = eax->flModulationTime;
    efx->flModulationDepth = eax->flModulationDepth;
    efx->flAirAbsorptionGainHF = mB_to_gain(eax->flAirAbsorptionHF);
    efx->flHFReference = eax->flHFReference;
    efx->flLFReference = eax->flLFReference;
    efx->flRoomRolloffFactor = eax->flRoomRolloffFactor;
    efx->iDecayHFLimit =
        (eax->ulFlags & EAXLISTENERFLAGS_DECAYHFLIMIT) ? 1 : 0;
}

void plEAXListenerMod::ConvertEFXToEAX(const EFXEAXREVERBPROPERTIES* efx,
    EAXREVERBPROPERTIES* eax)
{
    eax->flEnvironmentSize = cbrtf(16.0f * efx->flDensity);
    eax->flEnvironmentDiffusion = efx->flDiffusion;
    eax->lRoom = gain_to_mB(efx->flGain);
    eax->lRoomHF = gain_to_mB(efx->flGainHF);
    eax->lRoomLF = gain_to_mB(efx->flGainLF);
    eax->flDecayTime = efx->flDecayTime;
    eax->flDecayHFRatio = efx->flDecayHFRatio;
    eax->flDecayLFRatio = efx->flDecayLFRatio;
    eax->lReflections = gain_to_mB(efx->flReflectionsGain);
    eax->flReflectionsDelay = efx->flReflectionsDelay;
    eax->vReflectionsPan.x = efx->flReflectionsPan[0];
    eax->vReflectionsPan.y = efx->flReflectionsPan[1];
    eax->vReflectionsPan.z = efx->flReflectionsPan[2];
    eax->lReverb = gain_to_mB(efx->flLateReverbGain);
    eax->flReverbDelay = efx->flLateReverbDelay;
    eax->vReverbPan.x = efx->flLateReverbPan[0];
    eax->vReverbPan.y = efx->flLateReverbPan[1];
    eax->vReverbPan.z = efx->flLateReverbPan[2];
    eax->flEchoTime = efx->flEchoTime;
    eax->flEchoDepth = efx->flEchoDepth;
    eax->flModulationTime = efx->flModulationTime;
    eax->flModulationDepth = efx->flModulationDepth;
    eax->flAirAbsorptionHF = gain_to_mB(efx->flAirAbsorptionGainHF);
    eax->flHFReference = efx->flHFReference;
    eax->flLFReference = efx->flLFReference;
    eax->flRoomRolloffFactor = efx->flRoomRolloffFactor;
    if (efx->iDecayHFLimit == 1)
        eax->ulFlags |= EAXLISTENERFLAGS_DECAYHFLIMIT;
}
