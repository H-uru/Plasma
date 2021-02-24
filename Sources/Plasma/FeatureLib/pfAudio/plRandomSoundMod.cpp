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

#include "plRandomSoundMod.h"

#include "HeadSpin.h"
#include "plgDispatch.h"
#include "hsTimer.h"

#include "plMessage/plAnimCmdMsg.h"
#include "pnMessage/plSoundMsg.h"
#include "pnSceneObject/plSceneObject.h"
#include "pnSceneObject/plAudioInterface.h"

#include "plAudio/plAudioSystem.h"
#include "plAudio/plSound.h"
#include "plAudio/plWin32GroupedSound.h"        // EEK BAD
#include "plStatusLog/plStatusLog.h"

plRandomSoundModGroup::plRandomSoundModGroup()
    : fNumSounds(), fIndices(), fGroupedIdx(-1), fCurrent(-1)
{
}

plRandomSoundModGroup::~plRandomSoundModGroup()
{
    delete [] fIndices;
}

void plRandomSoundModGroup::Read(hsStream *s)
{
    fNumSounds = s->ReadLE16();
    fGroupedIdx = s->ReadLE16();
    fIndices = new uint16_t[fNumSounds];

    int i;
    for (i = 0; i < fNumSounds; i++)
        fIndices[i] = s->ReadLE16();
}

void plRandomSoundModGroup::Write(hsStream *s)
{
    s->WriteLE16(fNumSounds);
    s->WriteLE16(fGroupedIdx);
    
    int i;
    for (i = 0; i < fNumSounds; i++)
        s->WriteLE16(fIndices[i]);
}

///////////////////////////////////////////////////////////////////////////////////////

plRandomSoundMod::plRandomSoundMod()
    : fCurrentGroup(), fNumGroups(), fGroups(), fOldPriority(-1), fFirstTimePlay(true)
{
}

plRandomSoundMod::~plRandomSoundMod()
{
    delete [] fGroups;
}

void plRandomSoundMod::IPlayNextIfMaster()
{
    if( !fTarget )
        IRetry(2.f);
    
    if( IStopped() )
        return;

    IPlayNext();
}

// If we recieve a stop message, actually stop the sound
void plRandomSoundMod::IStop()
{
    plRandomCommandMod::IStop();
    
    plAudioInterface *ai = nullptr;
    
    if( !plgAudioSys::Active() ) return;

    ai = IGetTargetAudioInterface(0);
    if(!ai) return;

    if (fGroups != nullptr && fGroups[fCurrentGroup].fGroupedIdx != -1)
    {
        plSoundMsg *msg = new plSoundMsg();
        msg->SetCmd(plSoundMsg::kStop);
        msg->fIndex = fGroups[ fCurrentGroup ].fIndices[ fCurrent ];
        plgDispatch::MsgSend(msg);
    }
    else
    {
        if(fCurrent == -1) return;
        uint16_t currentSndIdx = (fGroups != nullptr) ? fGroups[fCurrentGroup].fIndices[fCurrent] : fActiveList[fCurrent];
        plSoundMsg* snd = new plSoundMsg(GetKey(), GetTarget()->GetKey(), nullptr);
        snd->SetCmd(plSoundMsg::kStop);
        snd->fIndex = currentSndIdx;
        plgDispatch::MsgSend(snd);
    }
}

void plRandomSoundMod::IPlayNext()
{
    if( !plgAudioSys::Active() )
    {
        IRetry(10.f);
        return;
    }

    plAudioInterface* ai = IGetTargetAudioInterface(0);
    if( !ai )
    {
        IRetry(2.f);
        return;
    }

    int i;
    uint16_t currentSndIdx;
    size_t nSounds = (fGroups == nullptr ? ai->GetNumSounds() : fGroups[fCurrentGroup].fNumSounds);
    if (nSounds > fEndTimes.size())
        fEndTimes.resize(nSounds);
    plSound *pSound = nullptr;

    // The global sound priority has changed, update the active random sounds list
    if (fOldPriority != plgAudioSys::GetPriorityCutoff() && fGroups == nullptr)
    {
        fActiveList.clear();
        fOldPriority = plgAudioSys::GetPriorityCutoff();
        for(i = 0; i < nSounds; i++)
        {
            pSound =  ai->GetSound(i);
            if(pSound && pSound->GetPriority() <= plgAudioSys::GetPriorityCutoff())
            {
                fActiveList.push_back(i);
            }
        }
        // There are no sounds that should play
        if (fGroups == nullptr && fActiveList.empty() && nSounds)
        {
            // If no sounds in this component even attempt to play this component gets mad and will never play any sounds again.
            // So, give it a zero to make it happy. This sound will still be rejected when it tries to play which is exactly what 
            // we want since if we get here no sounds in this component should play.
            fActiveList.push_back(0); 
        }
    }

    // if this is the first time this component is going to play a sound check to see if it has a delay time
    if(fFirstTimePlay)
    {
        fFirstTimePlay = false;
        if( !(fMode & kOneCmd) )
        {
            float delay = IGetDelay(0);
            double t = hsTimer::GetSysSeconds() + delay;

            plAnimCmdMsg* anim = new plAnimCmdMsg(GetKey(), GetKey(), &t);
            anim->SetCmd(plAnimCmdMsg::kContinue);
            plgDispatch::MsgSend(anim);
            return;
        }
    }
    
    if (!ISelectNext(fGroups == nullptr ? fActiveList.size() : nSounds))
    {
        plRandomCommandMod::IStop();
        return;
    }

    // We don't want random sounds to synch, since we don't synch the randomness. So force this next
    // sound to not synch
    float currLen;
    if (fGroups != nullptr && fGroups[fCurrentGroup].fGroupedIdx != -1)
    {
        currentSndIdx = fGroups[ fCurrentGroup ].fIndices[ fCurrent ];
        plWin32GroupedSound *sound = plWin32GroupedSound::ConvertNoRef( ai->GetSound( fGroups[ fCurrentGroup ].fGroupedIdx ) );
        
        if (!sound)
        {   
            hsAssert(sound != nullptr, "Invalid sound type in plRandomSoundMod");
            return;
        }
        sound->SetLocalOnly(true);

        // Send msg to the grouped sound to switch sounds
        plSoundMsg *snd = new plSoundMsg();
        snd->SetCmd( plSoundMsg::kSelectFromGroup );
        snd->fIndex = currentSndIdx;
        snd->Send( sound->GetKey() );

        // Now tell the audio interface to play the sound (probably should change this....)
        snd = new plSoundMsg(GetKey(), GetTarget()->GetKey(), nullptr);
        snd->SetCmd(plSoundMsg::kGoToTime);
        snd->fTime = (0);
        snd->SetCmd(plSoundMsg::kStop);
        snd->SetCmd(plSoundMsg::kPlay);
        snd->fIndex = fGroups[ fCurrentGroup ].fGroupedIdx;
        plgDispatch::MsgSend(snd);

        currLen = sound->GetSoundLength( currentSndIdx );
    }
    else
    {
        currentSndIdx = (fGroups != nullptr) ? fGroups[fCurrentGroup].fIndices[fCurrent] : fActiveList[fCurrent];
        if (ai->GetSound(currentSndIdx))
        {
            ai->GetSound( currentSndIdx )->SetLocalOnly(true);

            ai->GetSound(currentSndIdx)->Stop();
            ai->GetSound(currentSndIdx)->Play();
        }

        if (ai->GetSound(currentSndIdx))
            currLen = (float)(ai->GetSound(currentSndIdx)->GetLength());
        else
            currLen = 0;
    }

    if (plgAudioSys::AreExtendedLogsEnabled())
    {
        if (fGroups)
            plStatusLog::AddLineSF("audio.log", "{}: Playing sound #{} from group {}", GetTarget(0)->GetKeyName(), fCurrent, fCurrentGroup);
        else
            plStatusLog::AddLineSF("audio.log", "{}: Playing sound #{}", GetTarget(0)->GetKeyName(), fCurrent);
    }

    fEndTimes[fCurrent] = hsTimer::GetSysSeconds() + currLen;

    if( !(fMode & kOneCmd) )
    {
        float delay = IGetDelay(currLen);

        double t = hsTimer::GetSysSeconds() + delay;

        plAnimCmdMsg* anim = new plAnimCmdMsg(GetKey(), GetKey(), &t);
        anim->SetCmd(plAnimCmdMsg::kContinue);
        plgDispatch::MsgSend(anim);
    }
    else
    {
        plRandomCommandMod::IStop();
    }
}

void plRandomSoundMod::SetCurrentGroup(uint16_t group)
{
    hsAssert(group < fNumGroups, "Setting an invalid group on a random sound modifier");

    if (group != fCurrentGroup && group < fNumGroups)
    {
        fGroups[fCurrentGroup].fExcluded = fExcluded;
        fGroups[fCurrentGroup].fCurrent = fCurrent;
        fExcluded = fGroups[group].fExcluded;
        fCurrent = fGroups[group].fCurrent;
        fCurrentGroup = group;
    }
}

void plRandomSoundMod::Read(hsStream *s, hsResMgr *mgr)
{
    plRandomCommandMod::Read(s, mgr);

    fNumGroups = s->ReadLE16();
    if (fNumGroups > 0)
    {
        fGroups = new plRandomSoundModGroup[fNumGroups];
        int i;
        for (i = 0; i < fNumGroups; i++)
            fGroups[i].Read(s);
    }
}

void plRandomSoundMod::Write(hsStream *s, hsResMgr *mgr)
{
    plRandomCommandMod::Write(s, mgr);

    s->WriteLE16(fNumGroups);
    if (fNumGroups > 0)
    {
        int i;
        for (i = 0; i < fNumGroups; i++)
            fGroups[i].Write(s);
    }
}

void    plRandomSoundMod::ForceSoundLoadState( bool loaded )
{
    uint16_t  i, j;

    plAudioInterface* ai = IGetTargetAudioInterface(0);
    if (ai == nullptr)
        return;

    if (fGroups != nullptr)
    {
        for( i = 0; i < fNumGroups; i++ )
        {
            if( fGroups[ i ].fGroupedIdx != -1 )
            {
                plSound *sound = ai->GetSound( fGroups[ i ].fGroupedIdx );
                if (!sound)
                    return;
                if( loaded )
                    sound->ForceLoad();
                else
                    sound->ForceUnload();
            }
            else
            {
                for( j = 0; j < fGroups[ i ].fNumSounds; j++ )
                {
                    plSound *sound = ai->GetSound( fGroups[ i ].fIndices[ j ] );
                    if (!sound)
                        return;
                    if( loaded )
                        sound->ForceLoad();
                    else
                        sound->ForceUnload();
                }
            }
        }
    }
    else
    {
        for( i = 0; i < ai->GetNumSounds(); i++ )
        {
            plSound *sound = ai->GetSound( i );
            if (!sound)
                return;
            if( loaded )
                sound->ForceLoad();
            else
                sound->ForceUnload();
        }
    }
}

// Overload this to handle volume changes
bool plRandomSoundMod::MsgReceive(plMessage* msg)
{
    plAnimCmdMsg* anim = plAnimCmdMsg::ConvertNoRef(msg);
    if( anim )
    {
        // Actually sets the volume
        if( anim->Cmd(plAnimCmdMsg::kSetSpeed) )
        {
            ISetVolume(anim->fSpeed);
        }
    }

    // Don't understand, pass on to base class.
    return plRandomCommandMod::MsgReceive(msg);
}

void plRandomSoundMod::ISetVolume(float volume)
{
    plSound *pSound = nullptr;
    
    pSound = IGetSoundPtr();
    if(pSound)
        pSound->SetVolume(volume);
}

float plRandomSoundMod::GetVolume()
{
    float volume = 1.0;
    plSound *pSound;
    
    pSound = IGetSoundPtr();
    if(pSound)
        volume = pSound->GetMaxVolume();
    return volume;
}

void plRandomSoundMod::ISetPosition(hsPoint3 pos)
{
    plSound *pSound = IGetSoundPtr();
    if(pSound)
    {
        pSound->SetPosition(pos);
    }
}

plSound *plRandomSoundMod::IGetSoundPtr()
{
    plSound *pSound = nullptr;
    if (fGroups != nullptr)
        return nullptr;
    if (fCurrent == -1)
        return nullptr;  // sound list hasn't been initialized yet, don't try and access it

    int currentSndIdx = fActiveList[fCurrent];
    plAudioInterface* ai = IGetTargetAudioInterface(0);
    if( !ai )
        return nullptr;
    
    pSound = ai->GetSound( currentSndIdx );
    return pSound;
}
