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
#include "HeadSpin.h"
#include "hsGeometry3.h"
#include "hsResMgr.h"
#include "plgDispatch.h"
#include "plProfile.h"
#include "plWin32StaticSound.h"
#include "plWin32Sound.h"
#include "plDSoundBuffer.h"
#include "plAudioSystem.h"
#include "plAudioCore/plSoundBuffer.h"
#include "plAudioCore/plSoundDeswizzler.h"
#include "pnMessage/plEventCallbackMsg.h"
#include "pnMessage/plAudioSysMsg.h"
#include "plMessage/plLinkToAgeMsg.h"
#include "plMessage/plAvatarMsg.h"
#include "pnNetCommon/plNetApp.h"

#include "plPipeline/plPlates.h"
#include "plStatusLog/plStatusLog.h"

plProfile_Extern(MemSounds);
plProfile_CreateAsynchTimer( "Static Shove Time", "Sound", StaticSndShoveTime );
plProfile_CreateAsynchTimer( "Static Swizzle Time", "Sound", StaticSwizzleTime );

plWin32StaticSound::~plWin32StaticSound()
{
    DeActivate();
    IUnloadDataBuffer();
}

void plWin32StaticSound::Activate( bool forcePlay )
{
    plWin32Sound::Activate( forcePlay );
}

void plWin32StaticSound::DeActivate()
{
    plWin32Sound::DeActivate();
}

bool plWin32StaticSound::LoadSound( bool is3D )
{
    if (fFailed)
        return false;

    if( fPriority > plgAudioSys::GetPriorityCutoff() )
        return false;   // Don't set the failed flag, just return

    if (plgAudioSys::Active() && !fDSoundBuffer)
    {
        // Debug flag #1
        if( fChannelSelect > 0 && plgAudioSys::IsDebugFlagSet( plgAudioSys::kDisableRightSelect ) )
        {
            // Force a fail
            fFailed = true;
            return false;
        }

        // We need it to be resident to read in
        plSoundBuffer::ELoadReturnVal retVal = IPreLoadBuffer(true);
        plSoundBuffer *buffer = (plSoundBuffer *)fDataBufferKey->ObjectIsLoaded();  
        if(!buffer)
        {
            return plSoundBuffer::kError;
        }

        if( retVal == plSoundBuffer::kPending)  // we are still reading data. 
        {
            return true;
        }

        if( retVal == plSoundBuffer::kError )
        {
            ST::string str = ST::format("Unable to open .wav file {}", fDataBufferKey ? fDataBufferKey->GetName() : "nil");
            IPrintDbgMessage(str, true);
            fFailed = true;
            return false;
        }
        
        SetProperty( kPropIs3DSound, is3D );

        plWAVHeader header = buffer->GetHeader();

        // Debug flag #2
        if( fChannelSelect == 0 && header.fNumChannels > 1 && plgAudioSys::IsDebugFlagSet( plgAudioSys::kDisableLeftSelect ) )
        {
            // Force a fail
            fFailed = true;
            return false;
        }
        uint32_t bufferSize = buffer->GetDataLength();

        if( header.fNumChannels > 1 && is3D )
        {
            // We can only do a single channel of 3D sound. So copy over one (later)
            bufferSize              /= header.fNumChannels;
            header.fBlockAlign      /= header.fNumChannels;
            header.fAvgBytesPerSec  /= header.fNumChannels;
            header.fNumChannels = 1;
        }

        bool tryStatic = true;
        // If we want FX, we can't use a static voice, but EAX doesn't fit under that limitation :)
        if( 0 )
            tryStatic = false;

        // Create our DSound buffer (or rather, the wrapper around it)
        fDSoundBuffer = new plDSoundBuffer( bufferSize, header, is3D, IsPropertySet( kPropLooping ), tryStatic );
        if( !fDSoundBuffer->IsValid() )
        {
            ST::string str = ST::format("Can't create sound buffer for {}.wav. This could happen if the wav file is a stereo file."
                                        " Stereo files are not supported on 3D sounds. If the file is not stereo then please report this error.",
                                        GetFileName());
            IPrintDbgMessage(str, true);
            fFailed = true;

            delete fDSoundBuffer;
            fDSoundBuffer = nullptr;

            return false;
        }
    
        plProfile_BeginTiming( StaticSndShoveTime );

        if(!fDSoundBuffer->FillBuffer(buffer->GetData(), buffer->GetDataLength(), &header))
        {
            delete fDSoundBuffer;
            fDSoundBuffer = nullptr;
            plStatusLog::AddLineSF("audio.log", "Could not play static sound, no voices left {}", GetKeyName());
            return false;
        }

        plProfile_EndTiming( StaticSndShoveTime );
        IRefreshEAXSettings( true );

        fTotalBytes = bufferSize;

        plProfile_NewMem(MemSounds, fTotalBytes);

        // get pertinent info
        float length = (float)bufferSize / (float)header.fAvgBytesPerSec;
        SetLength(length);

        if( fLoadFromDiskOnDemand && !IsPropertySet( kPropLoadOnlyOnCall ) )
            FreeSoundData();

        return true;
    }
    return false;
}

void plWin32StaticSound::Update()
{
    plWin32Sound::Update();
    if(fDSoundBuffer)
    {
        if(fPlaying)    // we think we are playing
        {
            if(!fDSoundBuffer->IsPlaying()) // are we actually playing
            {
                Stop();
            }
        }
    }
}

void plWin32StaticSound::IDerivedActuallyPlay()
{
    // Ensure there's a stop notify for us
    if( !fReallyPlaying )
    {   
        for(;;)
        {
            if(IsPropertySet(kPropIncidental))
            {
                if(fIncidentalsPlaying >= MAX_INCIDENTALS)
                    break;
                ++fIncidentalsPlaying;
            }
        
            fDSoundBuffer->Play();
            fReallyPlaying = true;
            break;
        }
    }

    plSoundEvent    *event = IFindEvent( plSoundEvent::kStart );
    if (event != nullptr)
        event->SendCallbacks();
}

float plWin32StaticSound::GetActualTimeSec()
{
    if(fDSoundBuffer)
        return fDSoundBuffer->GetTimeOffsetSec();
    return 0.0f;
}

void plWin32StaticSound::ISetActualTime(double t)
{
    if( !fDSoundBuffer && plgAudioSys::Active())
        LoadSound( IsPropertySet( kPropIs3DSound ) );
    if( fDSoundBuffer )
    {
        if(!t)
            fDSoundBuffer->SetTimeOffsetSec((float)t);
    }
}

bool plWin32StaticSound::MsgReceive( plMessage* pMsg )
{
    return plWin32Sound::MsgReceive( pMsg );
}

void plWin32StaticSound::IRemoveCallback( plEventCallbackMsg *pCBMsg )
{
    plWin32Sound::IRemoveCallback( pCBMsg );
}

void plWin32StaticSound::IAddCallback( plEventCallbackMsg *pCBMsg )
{
    if( plSoundEvent::GetTypeFromCallbackMsg( pCBMsg ) != plSoundEvent::kStop &&
        plSoundEvent::GetTypeFromCallbackMsg( pCBMsg ) != plSoundEvent::kStart )
    {
        hsAssert( false, "Static sounds only support start and stop callbacks at this time." );
        return;
    }
    plWin32Sound::IAddCallback( pCBMsg );
}


plWin32LinkSound::plWin32LinkSound()
{
    SetLocalOnly(true); // linking sounds already synch at a higher level
    SetProperty( kPropDontFade, true );
}

void plWin32LinkSound::Read(hsStream* s, hsResMgr* mgr)
{
    plWin32StaticSound::Read(s, mgr);

    plgDispatch::Dispatch()->RegisterForExactType(plLinkEffectBCMsg::Index(), GetKey());
    plgDispatch::Dispatch()->RegisterForExactType(plAvatarStealthModeMsg::Index(), GetKey());
    plgDispatch::Dispatch()->RegisterForExactType(plPseudoLinkEffectMsg::Index(), GetKey());
    
    SetLocalOnly(true); // linking sounds already synch at a higher level
    SetProperty( kPropDontFade, true );
}

void plWin32LinkSound::Write(hsStream* s, hsResMgr* mgr)
{
    plWin32StaticSound::Write(s, mgr);
}

bool plWin32LinkSound::MsgReceive( plMessage* pMsg )
{
    plLinkEffectBCMsg *msg = plLinkEffectBCMsg::ConvertNoRef( pMsg );
    if (msg != nullptr && !msg->HasLinkFlag(plLinkEffectBCMsg::kMute))
    {
        if (msg->fLinkKey->GetUoid().GetClonePlayerID() == GetKey()->GetUoid().GetClonePlayerID())
        {
            if (!IsPropertySet(kPropFullyDisabled))
            {
                ISetActualTime(0);
                Play();
                //Activate(true);
            }
        }
        return true;
    }

    plPseudoLinkEffectMsg *psmsg = plPseudoLinkEffectMsg::ConvertNoRef( pMsg );
    if (psmsg != nullptr)
    {
        if (psmsg->fAvatarKey->GetUoid().GetClonePlayerID() == GetKey()->GetUoid().GetClonePlayerID())
        {
            if (!IsPropertySet(kPropFullyDisabled))
            {
                ISetActualTime(0);
                //Play();
                Activate(true);
            }
        }
        return true;
    }

    plAvatarStealthModeMsg *sMsg = plAvatarStealthModeMsg::ConvertNoRef(pMsg);
    if (sMsg)
    {
        if (sMsg->GetSender()->GetUoid().GetClonePlayerID() == GetKey()->GetUoid().GetClonePlayerID())
        {
            SetProperty(kPropFullyDisabled, (sMsg->fMode == plAvatarStealthModeMsg::kStealthCloaked));
            plNetApp::StaticDebugMsg("plWin32LinkSound: rcvd avatarStealth msg, cloaked={}", sMsg->fMode == plAvatarStealthModeMsg::kStealthCloaked);
        }
        return true;
    }
    
    return plWin32StaticSound::MsgReceive( pMsg );
}
