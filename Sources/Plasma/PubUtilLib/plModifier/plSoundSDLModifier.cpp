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
#include "plSoundSDLModifier.h"
#include "../plSDL/plSDL.h"
#include "../pnSceneObject/plSceneObject.h"
#include "../pnSceneObject/plAudioInterface.h"
#include "../plAudio/plSound.h"

// static vars
char plSoundSDLModifier::kStrVolume[]="desiredVolume";
char plSoundSDLModifier::kStrTime[]="time";
char plSoundSDLModifier::kStrPlaying[]="playing";
char plSoundSDLModifier::kStrSounds[]="sounds";

//
// get current state from audio interface
// fill out state data rec
//
void plSoundSDLModifier::IPutCurrentStateIn(plStateDataRecord* dstState)
{
	/*plSceneObject* sobj=GetTarget();
	hsAssert(sobj, "plSoundSDLModifier, nil target");
	
	const plAudioInterface* ai=sobj->GetAudioInterface();
	hsAssert(ai, "nil audio interface");

	plSDStateVariable* soundListVar=dstState->FindSDVar(kStrSounds);
	int numSounds=ai->GetNumSounds();
	soundListVar->Resize(numSounds);
		
	int i;
	for(i=0;i<numSounds; i++)
	{
		plStateDataRecord* soundState=soundListVar->GetStateDataRecord(i);
		plSound* sound=ai->GetSound(i);

		soundState->FindVar(kStrVolume)->Set(sound->fDesiredVol);
		soundState->FindVar(kStrTime)->Set(sound->fVirtualStartTime);
		soundState->FindVar(kStrPlaying)->Set(sound->IsPlaying());
	}*/
}

//
// apply incoming state to current audio interface
//
void plSoundSDLModifier::ISetCurrentStateFrom(const plStateDataRecord* srcState)
{
	plSceneObject* sobj=GetTarget();
	hsAssert(sobj, "plSoundSDLModifier, nil target");

	const plAudioInterface* ai=sobj->GetAudioInterface();
	hsAssert(ai, "nil audio interface");
	int numSounds=ai->GetNumSounds();

	plSDStateVariable* soundListVar=srcState->FindSDVar(kStrSounds);

	if( soundListVar->GetCount() != numSounds )
	{
		hsAssert( false, "number sounds sounds should not be changing");
		return;
	}

	int i;
	for(i=0;i<numSounds;i++)
	{
		plStateDataRecord* soundState=soundListVar->GetStateDataRecord(i);
		plSound* sound=ai->GetSound(i);

		float desiredVol;
		soundState->FindVar(kStrVolume)->Get(&desiredVol);		
		//sound->ISetUnsynchedVolume(desiredVol);		// MCN CHECK

		bool playing;
		if (soundState->FindVar(kStrPlaying)->Get(&playing))
		{
			if (playing)
			{
				//double timeStarted;
				/*if (soundState->FindVar(kStrTime)->Get(&timeStarted))
					sound->SynchedPlay((hsScalar)timeStarted);
				else
				{
					// Can't get the time we're supposed to start at, so we'll just try to play normally,
					// which should be better than nothing...
					hsAssert( false, "No timeStarted state in sound SDL. Bad state from server? Contact MCN *immediately*" );
					sound->Play();
				}*/
			}
			else
			{
				if( sound->IsPropertySet( plSound::kPropAutoStart ) )
				{
#if 0
					// There is a sound in teledahn (swampAmb) which leggaly has this behavior
					hsAssert( false, "Auto-start sound just got a state update telling it to stop. "
								"This is technically legal, but so far there isn't any case where this should "
								"happen. Further, it's very likely to be the cause of the very-intermittent "
								"auto-start-sounds-not-playing bug. Leave this up and contact MCN *immediately*" );
#endif
				}
				//sound->IStop();
			}
		}
	}
}


