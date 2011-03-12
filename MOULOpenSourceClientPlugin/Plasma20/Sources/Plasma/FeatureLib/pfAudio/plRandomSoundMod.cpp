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

#include "hsTypes.h"
#include "plRandomSoundMod.h"
#include "../pnSceneObject/plSceneObject.h"
#include "../pnSceneObject/plAudioInterface.h"
#include "../pnMessage/plSoundMsg.h"
#include "../plMessage/plAnimCmdMsg.h"
#include "../plAudio/plAudioSystem.h"
#include "../plAudio/plSound.h"
#include "../plAudio/plWin32GroupedSound.h"		// EEK BAD
#include "plgDispatch.h"
#include "hsTimer.h"
#include "../plStatusLog/plStatusLog.h"

plRandomSoundModGroup::plRandomSoundModGroup() : fNumSounds(0), fIndices(nil), fGroupedIdx(-1), fCurrent(-1)
{
}

plRandomSoundModGroup::~plRandomSoundModGroup()
{
	delete [] fIndices;
}

void plRandomSoundModGroup::Read(hsStream *s)
{
	fNumSounds = s->ReadSwap16();
	fGroupedIdx = s->ReadSwap16();
	fIndices = TRACKED_NEW UInt16[fNumSounds];

	int i;
	for (i = 0; i < fNumSounds; i++)
		fIndices[i] = s->ReadSwap16();
}

void plRandomSoundModGroup::Write(hsStream *s)
{
	s->WriteSwap16(fNumSounds);
	s->WriteSwap16(fGroupedIdx);
	
	int i;
	for (i = 0; i < fNumSounds; i++)
		s->WriteSwap16(fIndices[i]);
}

///////////////////////////////////////////////////////////////////////////////////////

plRandomSoundMod::plRandomSoundMod() : fCurrentGroup(0), fNumGroups(0), fGroups(nil), fOldPriority(-1), fFirstTimePlay(true)
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
	
	plAudioInterface *ai = nil;
	
	if( !plgAudioSys::Active() ) return;

	ai = IGetTargetAudioInterface(0);
	if(!ai) return;

	if( fGroups != nil && fGroups[ fCurrentGroup ].fGroupedIdx != -1 )
	{
		plSoundMsg *msg = TRACKED_NEW plSoundMsg();
		msg->SetCmd(plSoundMsg::kStop);
		msg->fIndex = fGroups[ fCurrentGroup ].fIndices[ fCurrent ];
		plgDispatch::MsgSend(msg);
	}
	else
	{
		if(fCurrent == -1) return;
		UInt16 currentSndIdx = ( fGroups != nil ) ? fGroups[fCurrentGroup].fIndices[fCurrent] : fActiveList[fCurrent];
		plSoundMsg* snd = TRACKED_NEW plSoundMsg(GetKey(), GetTarget()->GetKey(), nil);
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
	UInt16 currentSndIdx;
	int nSounds = (fGroups == nil ? ai->GetNumSounds() : fGroups[fCurrentGroup].fNumSounds);
	fEndTimes.ExpandAndZero(nSounds);
	plSound *pSound = nil;

	// The global sound priority has changed, update the active random sounds list
	if(fOldPriority != plgAudioSys::GetPriorityCutoff() && fGroups == nil)
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
		if(fGroups == nil && fActiveList.empty() && nSounds)
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
			hsScalar delay = IGetDelay(0);
			double t = hsTimer::GetSysSeconds() + delay;

			plAnimCmdMsg* anim = TRACKED_NEW plAnimCmdMsg(GetKey(), GetKey(), &t);
			anim->SetCmd(plAnimCmdMsg::kContinue);
			plgDispatch::MsgSend(anim);
			return;
		}
	}
	
	if( !ISelectNext(fGroups == nil ? fActiveList.size() : nSounds) )
	{
		plRandomCommandMod::IStop();
		return;
	}

	// We don't want random sounds to synch, since we don't synch the randomness. So force this next
	// sound to not synch
	hsScalar currLen;
	if( fGroups != nil && fGroups[ fCurrentGroup ].fGroupedIdx != -1 )
	{
		currentSndIdx = fGroups[ fCurrentGroup ].fIndices[ fCurrent ];
		plWin32GroupedSound	*sound = plWin32GroupedSound::ConvertNoRef( ai->GetSound( fGroups[ fCurrentGroup ].fGroupedIdx ) );
		
		if (!sound)
		{	
			hsAssert( sound != nil, "Invalid sound type in plRandomSoundMod" );
			return;
		}
		sound->SetLocalOnly(true);

		// Send msg to the grouped sound to switch sounds
		plSoundMsg *snd = TRACKED_NEW plSoundMsg();
		snd->SetCmd( plSoundMsg::kSelectFromGroup );
		snd->fIndex = currentSndIdx;
		snd->Send( sound->GetKey() );

		// Now tell the audio interface to play the sound (probably should change this....)
		snd = TRACKED_NEW plSoundMsg(GetKey(), GetTarget()->GetKey(), nil);
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
		currentSndIdx = ( fGroups != nil ) ? fGroups[fCurrentGroup].fIndices[fCurrent] : fActiveList[fCurrent];
		if (ai->GetSound(currentSndIdx))
		{
			ai->GetSound( currentSndIdx )->SetLocalOnly(true);

			ai->GetSound(currentSndIdx)->Stop();
			ai->GetSound(currentSndIdx)->Play();
		}

		if (ai->GetSound(currentSndIdx))
			currLen = (hsScalar)(ai->GetSound(currentSndIdx)->GetLength());
		else
			currLen = 0;
	}

	if (plgAudioSys::AreExtendedLogsEnabled())
	{
		if (fGroups)
			plStatusLog::AddLineS("audio.log", "%s: Playing sound #%d from group %d", GetTarget(0)->GetKeyName(), fCurrent, fCurrentGroup);
		else
			plStatusLog::AddLineS("audio.log", "%s: Playing sound #%d", GetTarget(0)->GetKeyName(), fCurrent);
	}

	fEndTimes[fCurrent] = hsTimer::GetSysSeconds() + currLen;

	if( !(fMode & kOneCmd) )
	{
		hsScalar delay = IGetDelay(currLen);

		double t = hsTimer::GetSysSeconds() + delay;

		plAnimCmdMsg* anim = TRACKED_NEW plAnimCmdMsg(GetKey(), GetKey(), &t);
		anim->SetCmd(plAnimCmdMsg::kContinue);
		plgDispatch::MsgSend(anim);
	}
	else
	{
		plRandomCommandMod::IStop();
	}
}

void plRandomSoundMod::SetCurrentGroup(UInt16 group)
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

	fNumGroups = s->ReadSwap16();
	if (fNumGroups > 0)
	{
		fGroups = TRACKED_NEW plRandomSoundModGroup[fNumGroups];
		int i;
		for (i = 0; i < fNumGroups; i++)
			fGroups[i].Read(s);
	}
}

void plRandomSoundMod::Write(hsStream *s, hsResMgr *mgr)
{
	plRandomCommandMod::Write(s, mgr);

	s->WriteSwap16(fNumGroups);
	if (fNumGroups > 0)
	{
		int i;
		for (i = 0; i < fNumGroups; i++)
			fGroups[i].Write(s);
	}
}

void	plRandomSoundMod::ForceSoundLoadState( hsBool loaded )
{
	UInt16	i, j;

	plAudioInterface* ai = IGetTargetAudioInterface(0);
	if( ai == nil )
		return;

	if( fGroups != nil )
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
hsBool plRandomSoundMod::MsgReceive(plMessage* msg)
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

void plRandomSoundMod::ISetVolume(hsScalar volume)
{
	plSound *pSound = nil;
	
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
	plSound *pSound = nil;
	if(fGroups != nil) return nil;
	if(fCurrent == -1) return nil;	// sound list hasn't been initialized yet, don't try and access it 

	int currentSndIdx = fActiveList[fCurrent];
	plAudioInterface* ai = IGetTargetAudioInterface(0);
	if( !ai )
		return nil;
	
	pSound = ai->GetSound( currentSndIdx );
	return pSound;
}