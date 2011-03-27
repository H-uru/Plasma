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
#include "al.h"
#include "alc.h"
#include "efx.h"
#include <MMREG.H>
#ifdef EAX_SDK_AVAILABLE
#include <eax.h>
#endif

#include "hsTimer.h"
#include "hsGeometry3.h"
#include "plgDispatch.h"
#include "plProfile.h"
#include "../plStatusLog/plStatusLog.h"

#include "plSound.h"
#include "plAudioCaps.h"
#include "plAudioSystem.h"
#include "plDSoundBuffer.h"
#include "plEAXEffects.h"
#include "plEAXListenerMod.h"
#include "plVoiceChat.h"

#include "../pnMessage/plAudioSysMsg.h"
#include "../plMessage/plRenderMsg.h"
#include "../pnMessage/plRefMsg.h"
#include "../plMessage/plAgeLoadedMsg.h"
#include "../pnMessage/plTimeMsg.h"

#include "../pnKeyedObject/plFixedKey.h"
#include "../pnKeyedObject/plKey.h"


#define SAFE_RELEASE(p) if(p){ p->Release(); p = nil; }
#define FADE_TIME	3
#define MAX_NUM_SOURCES 128
#define UPDATE_TIME_MS 100

plProfile_CreateTimer("EAX Update", "Sound", SoundEAXUpdate);
plProfile_CreateTimer("Soft Update", "Sound", SoundSoftUpdate);
plProfile_CreateCounter("Max Sounds", "Sound", SoundMaxNum);
plProfile_CreateTimer("AudioUpdate", "RenderSetup", AudioUpdate);

typedef std::vector<DeviceDescriptor>::iterator DeviceIter;

//// Internal plSoftSoundNode Class Definition ///////////////////////////////
class plSoftSoundNode
{
	public:
		const plKey	fSoundKey;
		hsScalar	fRank;

		plSoftSoundNode	*fNext;
		plSoftSoundNode	**fPrev;

		plSoftSoundNode	*fSortNext;
		plSoftSoundNode	**fSortPrev;

		plSoftSoundNode( const plKey s ) : fSoundKey( s ) { fNext = nil; fPrev = nil; }
		~plSoftSoundNode() { Unlink(); }

		void	Link( plSoftSoundNode **prev )
		{
			fNext = *prev;
			fPrev = prev;
			if( fNext != nil )
				fNext->fPrev = &fNext;
			*prev = this;
		}

		void	Unlink( void )
		{
			if( fPrev != nil )
			{
				*fPrev = fNext;
				if( fNext )
					fNext->fPrev = fPrev;
				fPrev = nil;
				fNext = nil;
			}
		}

		void	SortedLink( plSoftSoundNode **prev, hsScalar rank )
		{
			fRank = rank;

			fSortNext = *prev;
			fSortPrev = prev;
			if( fSortNext != nil )
				fSortNext->fSortPrev = &fSortNext;
			*prev = this;
		}

		// Larger values are first in the list
		void	AddToSortedLink( plSoftSoundNode *toAdd, hsScalar rank )
		{
			if( fRank > rank )
			{
				if( fSortNext != nil )
					fSortNext->AddToSortedLink( toAdd, rank );
				else
				{
					toAdd->SortedLink( &fSortNext, rank );
				}
			}
			else
			{
				// Cute trick here...
				toAdd->SortedLink( fSortPrev, rank );
			}
		}

		void	BootSourceOff( void )
		{
			plSound	*sound = plSound::ConvertNoRef( fSoundKey->ObjectIsLoaded() );
			if( sound != nil )
			{
				sound->ForceUnregisterFromAudioSys();
			}
		}
};

// plAudioSystem //////////////////////////////////////////////////////////////////////////

Int32	plAudioSystem::fMaxNumSounds = 16;
Int32	plAudioSystem::fNumSoundsSlop = 8;

plAudioSystem::plAudioSystem() :
fStartTime(0),
fListenerInit(false),
fSoftRegionSounds(nil),
fActiveSofts(nil),
fCurrDebugSound(nil),
fDebugActiveSoundDisplay(nil),
fUsingEAX(false),
fRestartOnDestruct(false),
fWaitingForShutdown(false),
fActive(false),
fDisplayNumBuffers(false),
fStartFade(0),
fFadeLength(FADE_TIME),
fCaptureDevice(nil),
fLastUpdateTimeMs(0)
{
	fCurrListenerPos.Set( -1.e30, -1.e30, -1.e30 );
	//fCommittedListenerPos.Set( -1.e30, -1.e30, -1.e30 );
	fLastPos.Set(100, 100, 100);
}

plAudioSystem::~plAudioSystem()
{
}

void plAudioSystem::IEnumerateDevices()
{
	fDeviceList.clear();
	plStatusLog::AddLineS("audio.log", "--Audio Devices --" );
	char *devices = (char *)alcGetString(nil, ALC_DEVICE_SPECIFIER);
	int major, minor;

	while(*devices != nil)
	{
		ALCdevice *device = alcOpenDevice(devices);
		if (device) 
		{
			ALCcontext *context = alcCreateContext(device, NULL);
			if (context)
			{
				alcMakeContextCurrent(context);
				// if new actual device name isn't already in the list, then add it...
				bool bNewName = true;
				for (DeviceIter i = fDeviceList.begin(); i != fDeviceList.end(); i++) 
				{
					if (strcmp((*i).GetDeviceName(), devices) == 0) 
					{
						bNewName = false;
					}
				}
				if ((bNewName)) 
				{
					alcGetIntegerv(device, ALC_MAJOR_VERSION, sizeof(int), &major);
					alcGetIntegerv(device, ALC_MINOR_VERSION, sizeof(int), &minor);
					plStatusLog::AddLineS("audio.log", "%s OpenAL ver: %d.%d", devices, major, minor );

					// filter out any devices that aren't openal 1.1 compliant
					if(major > 1 || (major == 1 && minor >= 1))
					{
						hsBool supportsEAX = false;
#ifdef EAX_SDK_AVAILABLE
						if(alIsExtensionPresent((ALchar *)"EAX4.0") || alIsExtensionPresent((ALchar *) "EAX4.0Emulated"))		
						{
							supportsEAX = true;
						}
#endif
						DeviceDescriptor desc(devices, supportsEAX);
						fDeviceList.push_back(desc);
					}
				}
				alcMakeContextCurrent(nil);
				alcDestroyContext(context);
			}
			alcCloseDevice(device);
		}
		devices += strlen(devices) + 1;
	}

	DeviceDescriptor temp("", 0);
	// attempt to order devices
	for(unsigned i = 0; i < fDeviceList.size(); ++i)
	{
		if(strstr(fDeviceList[i].GetDeviceName(), "Software"))
		{
			temp = fDeviceList[i];
			fDeviceList[i] = fDeviceList[0];
			fDeviceList[0] = temp;
		}
		if(strstr(fDeviceList[i].GetDeviceName(), "Hardware"))
		{	
			temp = fDeviceList[i];
			fDeviceList[i] = fDeviceList[1];
			fDeviceList[1] = temp;
		}
	}
}

//// Init ////////////////////////////////////////////////////////////////////
hsBool	plAudioSystem::Init( hsWindowHndl hWnd )
{
	plgAudioSys::fRestarting = false;
	static hsBool firstTimeInit = true;	
	plStatusLog::AddLineS( "audio.log", plStatusLog::kBlue, "ASYS: -- Init --" );
	
	fMaxNumSources = 0;
	plStatusLog::AddLineS( "audio.log", plStatusLog::kGreen, "ASYS: Detecting caps..." );
	plSoundBuffer::Init();

	// Set the maximum number of sounds based on priority cutoff slider
	SetMaxNumberOfActiveSounds();
	const char *deviceName = plgAudioSys::fDeviceName.c_str();
	hsBool useDefaultDevice = true;
	
	if(firstTimeInit)
	{
		IEnumerateDevices();
		firstTimeInit = false;
	}

	if(!fDeviceList.size())
	{
		plStatusLog::AddLineS( "audio.log", plStatusLog::kRed, "ASYS: ERROR Unable to query any devices, is openal installed?" );
		return false;
	}

	// shouldn't ever happen, but just in case
	if(!deviceName)
		plgAudioSys::SetDeviceName(DEFAULT_AUDIO_DEVICE_NAME);

	for(DeviceIter i = fDeviceList.begin(); i != fDeviceList.end(); i++)
	{
		if(!strcmp((*i).GetDeviceName(), deviceName))
		{
			useDefaultDevice = false;
			break;
		}
	}
	
	if(useDefaultDevice)
	{
		// if no device has been specified we will use the "Generic Software" device by default. If "Generic Software" is unavailable(which can happen) we select the first available device
		// We want to use software by default since some audio cards have major problems with hardware + eax.
		const char *defaultDev = fDeviceList.front().GetDeviceName();
		for(DeviceIter i = fDeviceList.begin(); i != fDeviceList.end(); i++)
		{
			if(!strcmp(DEFAULT_AUDIO_DEVICE_NAME, (*i).GetDeviceName()))
			{
				defaultDev = DEFAULT_AUDIO_DEVICE_NAME;
				break;
			}
		}

		plgAudioSys::SetDeviceName(defaultDev, false);
		fDevice = alcOpenDevice(defaultDev);
		plStatusLog::AddLineS( "audio.log", plStatusLog::kRed, "ASYS: %s device selected", defaultDev );
		deviceName = defaultDev;
	}
	else
	{
		plgAudioSys::SetDeviceName(deviceName, false);
		fDevice = alcOpenDevice(deviceName);
		plStatusLog::AddLineS( "audio.log", plStatusLog::kRed, "ASYS: %s device selected", deviceName );
	}
	if(!fDevice)
	{
		plStatusLog::AddLineS( "audio.log", plStatusLog::kRed, "ASYS: ERROR initializing OpenAL" );
		return false;
	}
	
	fContext = alcCreateContext(fDevice, 0);
	alcMakeContextCurrent(fContext);

	ALenum error;
	if(alGetError() != AL_NO_ERROR)
	{
		plStatusLog::AddLineS( "audio.log", plStatusLog::kRed, "ASYS: ERROR alcMakeContextCurrent failed" );
		return false;
	}

	plStatusLog::AddLineS("audio.log", "OpenAL vendor: %s",     alGetString(AL_VENDOR));
	plStatusLog::AddLineS("audio.log", "OpenAL version: %s",    alGetString(AL_VERSION));
	plStatusLog::AddLineS("audio.log", "OpenAL renderer: %s",   alGetString(AL_RENDERER));
	plStatusLog::AddLineS("audio.log", "OpenAL extensions: %s", alGetString(AL_EXTENSIONS));
	plAudioCaps caps = plAudioCapsDetector::Detect();

	if(strcmp(deviceName, DEFAULT_AUDIO_DEVICE_NAME))		
	{
		// we are using a hardware device, set priority based on number of hardware voices
		unsigned int numVoices = caps.GetMaxNumVoices();
		
		if(numVoices < 16)
			plgAudioSys::SetPriorityCutoff(3);
		
		SetMaxNumberOfActiveSounds();
	}

	// setup capture device
	ALCsizei bufferSize = FREQUENCY * 2 * BUFFER_LEN_SECONDS;	// times 2 for 16-bit format
	//const char *dev = alcGetString(fDevice, ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER);
	fCaptureDevice = alcCaptureOpenDevice(nil, FREQUENCY, AL_FORMAT_MONO16, bufferSize);
 
	fMaxNumSources = caps.GetMaxNumVoices();

	// attempt to init the EAX listener. 
	if( plgAudioSys::fEnableEAX )
	{
		fUsingEAX = plEAXListener::GetInstance().Init();
		if( fUsingEAX )
		{
			plStatusLog::AddLineS( "audio.log", plStatusLog::kGreen, "ASYS: EAX support detected and enabled." );
		}
		else
		{
			plStatusLog::AddLineS( "audio.log", plStatusLog::kRed, "ASYS: EAX support NOT detected. EAX effects disabled." );
		}
	}
	else
		fUsingEAX = false;
	
	plProfile_Set(SoundMaxNum, fMaxNumSounds);

	alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);	
	
	error = alGetError();

	fWaitingForShutdown = false;
	plgDispatch::Dispatch()->RegisterForExactType( plAgeLoadedMsg::Index(), GetKey() );
	return true;
}

//// Shutdown ////////////////////////////////////////////////////////////////

void plAudioSystem::Shutdown()
{
	plStatusLog::AddLineS( "audio.log", plStatusLog::kBlue, "ASYS: -- Shutdown --" );

	plSoundBuffer::Shutdown();

	// Delete our active sounds list
	delete fDebugActiveSoundDisplay;
	fDebugActiveSoundDisplay = nil;

	// Unregister soft sounds
	while( fSoftRegionSounds != nil )
	{
		fSoftRegionSounds->BootSourceOff();
		delete fSoftRegionSounds;
	}
	while( fActiveSofts != nil )
	{
		fActiveSofts->BootSourceOff();
		delete fActiveSofts;
	}

	while( fEAXRegions.GetCount() > 0 )
	{
		if( fEAXRegions[ 0 ] != nil && fEAXRegions[ 0 ]->GetKey() != nil )
		{
			GetKey()->Release( fEAXRegions[ 0 ]->GetKey() );
		}
		fEAXRegions.Remove( 0 );
	}
	plEAXListener::GetInstance().ClearProcessCache();

	plSound::SetCurrDebugPlate( nil );
	fCurrDebugSound = nil;

	// Reset this, just in case
	fPendingRegisters.Reset();

	//fListenerInit = false;
	
	if( fUsingEAX )
	{
		plEAXListener::GetInstance().Shutdown();
	}

	alcCaptureStop(fCaptureDevice);
	alcCaptureCloseDevice(fCaptureDevice);
	fCaptureDevice = nil;

	alcMakeContextCurrent(nil);
	alcDestroyContext(fContext);
	alcCloseDevice(fDevice);
	fContext = nil;
	fDevice = nil;

	fStartTime = 0;
	fUsingEAX = false;
	fCurrListenerPos.Set( -1.e30, -1.e30, -1.e30 );
	//fCommittedListenerPos.Set( -1.e30, -1.e30, -1.e30 );
	
	if( fRestartOnDestruct )
	{
		fRestartOnDestruct = false;
		plgAudioSys::Activate( true );
	}

	plgDispatch::Dispatch()->UnRegisterForExactType(plAgeLoadedMsg::Index(), GetKey() );
	fWaitingForShutdown = false;
}

int plAudioSystem::GetNumAudioDevices()
{
	return fDeviceList.size();
}

const char *plAudioSystem::GetAudioDeviceName(int index)
{
	if(index < 0 || index >= fDeviceList.size())
	{
		hsAssert(false, "Invalid index passed to GetAudioDeviceName");
		return nil;
	}
	return fDeviceList[index].GetDeviceName();
}

hsBool plAudioSystem::SupportsEAX(const char *deviceName)
{
	for(DeviceIter i = fDeviceList.begin(); i != fDeviceList.end(); i++)
	{
		if(!strcmp((*i).GetDeviceName(), deviceName))
		{
			return (*i).SupportsEAX();
		}
	}
	return false;
}

void plAudioSystem::SetDistanceModel(int i)
{
	switch(i)
	{
		case 0: alDistanceModel(AL_NONE); break;
		case 1: alDistanceModel(AL_INVERSE_DISTANCE ); break;
		case 2: alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED); break;
		case 3: alDistanceModel(AL_LINEAR_DISTANCE ); break;
		case 4: alDistanceModel(AL_LINEAR_DISTANCE_CLAMPED ); break;
		case 5: alDistanceModel(AL_EXPONENT_DISTANCE ); break;
		case 6: alDistanceModel(AL_EXPONENT_DISTANCE_CLAMPED); break;
	}
}

// Set the number of active sounds the audio system is allowed to play, based on the priority cutoff
void plAudioSystem::SetMaxNumberOfActiveSounds()
{
	UInt16 priorityCutoff = plgAudioSys::GetPriorityCutoff();
	int maxNumSounds = 24;
	
	// Keep this to a reasonable amount based on the users hardware, since we want the sounds to be played in hardware
	if(maxNumSounds > fMaxNumSources && fMaxNumSources != 0 )
		maxNumSounds = fMaxNumSources / 2;

	// Allow a certain number of sounds based on a user specified setting.
	// Above that limit, we want 1/2 more sounds (8 for the max of 16) for a slop buffer, 
	// if they fit, so that sounds that shouldn't be playing can still play in case they suddenly pop
	// back into priority range (so we don't incur performance hits when sounds hover on
	// the edge of being high enough priority to play)
	fMaxNumSounds = maxNumSounds;
	fNumSoundsSlop = fMaxNumSounds / 2;

	plStatusLog::AddLineS( "audio.log", "Max Number of Sounds Set to: %d", fMaxNumSounds);
}

void plAudioSystem::SetListenerPos(const hsPoint3 pos)
{
	fCurrListenerPos = pos;
	alListener3f(AL_POSITION, pos.fX, pos.fZ, -pos.fY);		// negate z coord, since openal uses opposite handedness
}

void plAudioSystem::SetListenerVelocity(const hsVector3 vel)
{
	alListener3f(AL_VELOCITY, 0, 0, 0);	// no doppler shift
}

void plAudioSystem::SetListenerOrientation(const hsVector3 view, const hsVector3 up)
{
	ALfloat orientation[] = { view.fX, view.fZ, -view.fY, up.fX, up.fZ, -up.fY };
	alListenerfv(AL_ORIENTATION, orientation);
}

void	plAudioSystem::SetActive( hsBool b ) 
{
	fActive = b;
	if( fActive )
	{
		// Clear to send activate message (if listener not inited yet, delay until then)
		plgDispatch::MsgSend( TRACKED_NEW plAudioSysMsg( plAudioSysMsg::kActivate ) );
	}
}

//// IRegisterSoftSound //////////////////////////////////////////////////////
//	Note: each sound might kick another off the top-n-ranked-sounds list, so
//	we call IUpdateSoftSounds() each time one is added. Ugly as sin, but at
//	least all the calc code is in one place. Possible optimization in the
//	future: when calling IUpdate(), any sounds that are already active don't
//	need to be recalced, just resorted.
void	plAudioSystem::RegisterSoftSound( const plKey soundKey )
{
	plSoftSoundNode	*node = TRACKED_NEW plSoftSoundNode( soundKey );
	node->Link( &fSoftRegionSounds );

	fCurrDebugSound = nil;
	plSound::SetCurrDebugPlate( nil );
}

//// IUnregisterSoftSound ////////////////////////////////////////////////////

void	plAudioSystem::UnregisterSoftSound( const plKey soundKey )
{
	plSoftSoundNode	*node;
	for( node = fActiveSofts; node != nil; node = node->fNext )
	{
		if( node->fSoundKey == soundKey )
		{
			delete node;
			return;
		}
	}

	for( node = fSoftRegionSounds; node != nil; node = node->fNext )
	{
		if( node->fSoundKey == soundKey )
		{
			delete node;
			return;
		}
	}

	// We might have unregistered it ourselves on destruction, so don't bother

	fCurrDebugSound = nil;
	plSound::SetCurrDebugPlate( nil );
}

//// IUpdateSoftSounds ///////////////////////////////////////////////////////
//	OK, so the listener moved. Since our sound soft volumes are all based on 
//	the listener's position, we have to update all sounds with soft volumes 
//	now. This involves some steps:
//		- Determining what volumes the listener has moved out of
//		- Determining what volumes the listener has changed position in
//		- Determining what volumes the listener has entered
//		- Updating the levels of all sounds associated with all of the above 
//			volumes
//	The first two tests are easy, since we'll have kept track of what volumes 
//	the listener was in last time. The last part is the tricky one and the 
//	one that will kill us in performance if we're not careful. However, we 
//	can first check the bounding box of the sounds in question, since outside
//	of them the soft volume won't have any effect. The last part is simply a 
//	matter of querying the sound volumes based on the listener's position and
//	setting the soft volume attenuation on the sound to that level.
//
//	Note: for each sound that is still in range, we call CalcSoftVolume() and
//	use the resulting value to rank each sound. Then, we take the top n sounds
//	and disable the rest. We *could* rank by distance to listener, which would
//	be far faster; however, we could have a sound (or background music) that
//	is technically closest to the listener but completely faded by a soft volume,
//	thus needlessly cutting off another sound that might be more important.
//	This way is slower, but better quality.
//	Also note: to differentiate between two sounds in the same soft volume at
//	full strength, we divide the soft volume ranks by the distSquared, thus
//	giving us a reasonable approximation at a good rank...<sigh>

void	plAudioSystem::IUpdateSoftSounds( const hsPoint3 &newPosition )
{
	plSoftSoundNode	*node, *myNode;
	hsScalar		distSquared, rank;
	plSoftSoundNode	*sortedList = nil;
	Int32			i;
	
	plProfile_BeginTiming(SoundSoftUpdate);
	
	// Check the sounds the listener is already inside of. If we moved out, stop sound, else
	// just change attenuation
	for( node = fActiveSofts; node != nil; )
	{
		plSound	*sound = plSound::ConvertNoRef( node->fSoundKey->ObjectIsLoaded() );

		bool notActive = false;
		if(sound)
			sound->Update();
		
		// Quick checks for not-active
		if( sound == nil )
			notActive = true;
		else if( !sound->IsWithinRange( newPosition, &distSquared )  )
			notActive = true;
		else if(sound->GetPriority() > plgAudioSys::GetPriorityCutoff())
			notActive = true;

		if(plgAudioSys::fMutedStateChange)
		{
			sound->SetMuted(plgAudioSys::fMuted);
		}
		
		if( !notActive )
		{
			/// Our initial guess is that it's enabled...
			sound->CalcSoftVolume( true, distSquared );
			rank = sound->GetVolumeRank();
			if( rank <= 0.f )
				notActive = true;
			else
			{
				/// Queue up in our sorted list...
				if( sortedList == nil )
					node->SortedLink( &sortedList, (10.0f - sound->GetPriority()) * rank );
				else
					sortedList->AddToSortedLink( node, (10.0f - sound->GetPriority()) * rank );
				/// Still in radius, so consider it still "active". 
				node = node->fNext;
			}
		}

		if( notActive )
		{
			/// Moved out of range of the sound--stop the sound entirely and move it to our
			/// yeah-they're-registered-but-not-active list
			myNode = node;
			node = node->fNext;
			myNode->Unlink();
			myNode->Link( &fSoftRegionSounds );

			/// We know this sound won't be enabled, so skip the Calc() call
			if( sound != nil )
				sound->UpdateSoftVolume( false );
		}
	}
	
	// Now check remaining sounds to see if the listener moved into them
	for( node = fSoftRegionSounds; node != nil; )
	{
		if( !fListenerInit )
		{
			node = node->fNext;
			continue;
		}
	
		plSound	*sound = plSound::ConvertNoRef( node->fSoundKey->ObjectIsLoaded() ); 
		if( !sound || sound->GetPriority() > plgAudioSys::GetPriorityCutoff() )
		{	
			node = node->fNext;
			continue;
		}

		sound->Update();
		if(plgAudioSys::fMutedStateChange)
		{
			sound->SetMuted(plgAudioSys::fMuted);
		}
		
		if( sound->IsWithinRange( newPosition, &distSquared ) )
		{
			/// Our initial guess is that it's enabled...
			sound->CalcSoftVolume( true, distSquared );
			rank = sound->GetVolumeRank();

			if( rank > 0.f )
			{			
				/// We just moved into its range, so move it to our active list and start the sucker
				myNode = node;
				node = node->fNext;
				myNode->Unlink();
				myNode->Link( &fActiveSofts );

				/// Queue up in our sorted list...
				if( sortedList == nil )
					myNode->SortedLink( &sortedList, (10.0f - sound->GetPriority()) * rank );
				else
					sortedList->AddToSortedLink( myNode, (10.0f - sound->GetPriority()) * rank );
			}
			else
			{
				/// Do NOT notify sound, since we were outside of its range and still are
				// (but if we're playing, we shouldn't be, so better update)
				if( sound->IsPlaying() )
					sound->UpdateSoftVolume( false );

				node = node->fNext;
			}
		}
		else
		{
			/// Do NOT notify sound, since we were outside of its range and still are
			node = node->fNext;		
			sound->Disable();		// ensure that dist attenuation is set to zero so we don't accidentally play
		}
	}
	
	plgAudioSys::fMutedStateChange = false;
	/// Go through sorted list, enabling only the first n sounds and disabling the rest
	// DEBUG: Create a screen-only statusLog to display which sounds are what
	if( fDebugActiveSoundDisplay == nil )
		fDebugActiveSoundDisplay = plStatusLogMgr::GetInstance().CreateStatusLog( 32, "Active Sounds", plStatusLog::kDontWriteFile | plStatusLog::kDeleteForMe | plStatusLog::kFilledBackground );
	fDebugActiveSoundDisplay->Clear();

	if(fDisplayNumBuffers)
		fDebugActiveSoundDisplay->AddLineF(0xffffffff, "Num Buffers: %d", plDSoundBuffer::GetNumBuffers() );
	fDebugActiveSoundDisplay->AddLine("Not streamed", plStatusLog::kGreen);
	fDebugActiveSoundDisplay->AddLine("Disk streamed", plStatusLog::kYellow);
	fDebugActiveSoundDisplay->AddLine("RAM streamed", plStatusLog::kWhite);
	fDebugActiveSoundDisplay->AddLine("Ogg streamed", plStatusLog::kRed);
	fDebugActiveSoundDisplay->AddLine("Incidentals", 0xff00ffff);
	fDebugActiveSoundDisplay->AddLine("--------------------");
	
	for( i = 0; sortedList != nil && i < fMaxNumSounds; sortedList = sortedList->fSortNext )
	{
		plSound	*sound = plSound::ConvertNoRef( sortedList->fSoundKey->GetObjectPtr() );
		if(!sound) continue;
	
		/// Notify sound that it really is still enabled
		sound->UpdateSoftVolume( true );
		
		UInt32 color = plStatusLog::kGreen;
		switch (sound->GetStreamType())
		{
			case plSound::kStreamFromDisk:		color = plStatusLog::kYellow;	break;
			case plSound::kStreamFromRAM:		color = plStatusLog::kWhite;	break;
			case plSound::kStreamCompressed:	color = plStatusLog::kRed;		break;
		}
		if(sound->GetType() == plgAudioSys::kVoice) color = 0xffff8800;
		if(sound->IsPropertySet(plSound::kPropIncidental)) color = 0xff00ffff;
		
		if( fUsingEAX && sound->GetEAXSettings().IsEnabled() )
		{
			fDebugActiveSoundDisplay->AddLineF(
				color, 
				"%d %1.2f %1.2f (%d occ) %s",
				sound->GetPriority(), 
				sortedList->fRank, 
				sound->GetVolume() ? sound->GetVolumeRank() / sound->GetVolume() : 0, 
				sound->GetEAXSettings().GetCurrSofts().GetOcclusion(), 
				sound->GetKeyName()
			);
		}
		else 
		{
			fDebugActiveSoundDisplay->AddLineF(
				color, 
				"%d %1.2f %1.2f %s", 
				sound->GetPriority(), 
				sortedList->fRank, 
				sound->GetVolume() ? sound->GetVolumeRank() / sound->GetVolume() : 0, 
				sound->GetKeyName()
			);
		}
		i++;
	}

	for( ; sortedList != nil; sortedList = sortedList->fSortNext, i++ )
	{
		plSound	*sound = plSound::ConvertNoRef( sortedList->fSoundKey->GetObjectPtr() );
		if(!sound) continue;

		/// These unlucky sounds don't get to play (yet). Also, be extra mean
		/// and pretend we're updating for "the first time", which will force them to
		/// stop immediately
		// Update: since being extra mean can incur a nasty performance hit when sounds hover back and
		// forth around the fMaxNumSounds mark, we have a "slop" allowance: i.e. sounds that we're going
		// to say shouldn't be playing but we'll let them play for a bit anyway just in case they raise
		// in priority. So only be mean to the sounds outside this slop range
		sound->UpdateSoftVolume( false, ( i < fMaxNumSounds + fNumSoundsSlop ) ? false : true );
		fDebugActiveSoundDisplay->AddLineF(
			0xff808080,
			"%d %1.2f %s", 
			sound->GetPriority(), 
			sound->GetVolume() ? sound->GetVolumeRank() / sound->GetVolume() : 0, 
			sound->GetKeyName()
		);
	}
	
	plProfile_EndTiming(SoundSoftUpdate);
}

void	plAudioSystem::NextDebugSound( void )
{
	plSoftSoundNode		*node;

	if( fCurrDebugSound == nil )
		fCurrDebugSound = ( fSoftRegionSounds == nil ) ? fActiveSofts : fSoftRegionSounds;
	else
	{
		node = fCurrDebugSound;
		fCurrDebugSound = fCurrDebugSound->fNext;
		if( fCurrDebugSound == nil )
		{
			// Trace back to find which list we were in
			for( fCurrDebugSound = fSoftRegionSounds; fCurrDebugSound != nil; fCurrDebugSound = fCurrDebugSound->fNext )
			{
				if( fCurrDebugSound == node )	// Was in first list, move to 2nd
				{
					fCurrDebugSound = fActiveSofts;		
					break;
				}
			}
			// else Must've been in 2nd list, so keep nil
		}
	}

	if( fCurrDebugSound != nil )
		plSound::SetCurrDebugPlate( fCurrDebugSound->fSoundKey );
	else
		plSound::SetCurrDebugPlate( nil );
}

void plAudioSystem::SetFadeLength(float lengthSec)
{
	fFadeLength = lengthSec;
}
	
hsBool plAudioSystem::MsgReceive(plMessage* msg)
{
	if(plTimeMsg *time = plTimeMsg::ConvertNoRef( msg ) )
	{
		if(!plgAudioSys::IsMuted())
		{
			double currTime = hsTimer::GetSeconds();
			if(fStartFade == 0)
			{
				plStatusLog::AddLineS("audio.log", "Starting Fade %f", currTime);
			}
			if((currTime - fStartFade) > fFadeLength) 
			{
				fStartFade = 0;
				plgDispatch::Dispatch()->UnRegisterForExactType( plTimeMsg::Index(), GetKey() );
				plStatusLog::AddLineS("audio.log", "Stopping Fade %f", currTime);
				plgAudioSys::SetGlobalFadeVolume( 1.0 ); 
			}
			else
			{
				plgAudioSys::SetGlobalFadeVolume( (hsScalar)((currTime-fStartFade) / fFadeLength) );
			}
		}
		return true;
	}
	
	if (plAudioSysMsg* pASMsg = plAudioSysMsg::ConvertNoRef( msg ))
	{
		if (pASMsg->GetAudFlag() == plAudioSysMsg::kPing && fListenerInit)
		{
			plAudioSysMsg* pMsg = TRACKED_NEW plAudioSysMsg( plAudioSysMsg::kActivate );
			pMsg->AddReceiver( pASMsg->GetSender() );
			pMsg->SetBCastFlag(plMessage::kBCastByExactType, false);
			plgDispatch::MsgSend( pMsg );
			return true;
		}
		else if (pASMsg->GetAudFlag() == plAudioSysMsg::kSetVol)
		{
			return true;
		}
		else if( pASMsg->GetAudFlag() == plAudioSysMsg::kDestroy )
		{
			if( fWaitingForShutdown )
				Shutdown();
			return true;
		}
		else if( pASMsg->GetAudFlag() == plAudioSysMsg::kUnmuteAll )
		{
			if( !pASMsg->HasBCastFlag( plMessage::kBCastByExactType ) )
				plgAudioSys::SetMuted( false );
			return true;
		}
	}
	
	if(plRenderMsg* pRMsg = plRenderMsg::ConvertNoRef( msg ))
	{
		//if( fListener )
		{
			plProfile_BeginLap(AudioUpdate, this->GetKey()->GetUoid().GetObjectName());
			if(hsTimer::GetMilliSeconds() - fLastUpdateTimeMs > UPDATE_TIME_MS)
			{
				IUpdateSoftSounds( fCurrListenerPos );

				if( fUsingEAX )
				{
					plProfile_BeginTiming(SoundEAXUpdate);
					plEAXListener::GetInstance().ProcessMods( fEAXRegions );
					plProfile_EndTiming(SoundEAXUpdate);
				}
				//fCommittedListenerPos = fCurrListenerPos;
			}
			plProfile_EndLap(AudioUpdate, this->GetKey()->GetUoid().GetObjectName());
		}
		
		return true;
	}

	if(plGenRefMsg *refMsg = plGenRefMsg::ConvertNoRef( msg ))
	{
		if( refMsg->GetContext() & ( plRefMsg::kOnCreate | plRefMsg::kOnRequest | plRefMsg::kOnReplace ) )
		{
			fEAXRegions.Append( plEAXListenerMod::ConvertNoRef( refMsg->GetRef() ) );
			plEAXListener::GetInstance().ClearProcessCache();
		}
		else if( refMsg->GetContext() & ( plRefMsg::kOnRemove | plRefMsg::kOnDestroy ) )
		{
			int idx = fEAXRegions.Find( plEAXListenerMod::ConvertNoRef( refMsg->GetRef() ) );
			if( idx != fEAXRegions.kMissingIndex )
				fEAXRegions.Remove( idx );
			plEAXListener::GetInstance().ClearProcessCache();
		}
		return true;
	}
	
	if(plAgeLoadedMsg *pALMsg = plAgeLoadedMsg::ConvertNoRef(msg))
	{
		if(!pALMsg->fLoaded)
		{
			fLastPos = fCurrListenerPos;
			fListenerInit = false;
		}
		else
		{
			fListenerInit = true;
		}
	}

	return hsKeyedObject::MsgReceive(msg);
}

// plgAudioSystem //////////////////////////////////////////////////////////////////////

plAudioSystem*	plgAudioSys::fSys = nil;
hsBool			plgAudioSys::fInit = false;
hsBool			plgAudioSys::fActive = false;
hsBool			plgAudioSys::fUseHardware = false;
hsBool			plgAudioSys::fMuted = true;
hsBool			plgAudioSys::fDelayedActivate = false;
hsBool			plgAudioSys::fEnableEAX = false;
hsWindowHndl	plgAudioSys::fWnd = nil;
hsScalar		plgAudioSys::fChannelVolumes[ kNumChannels ] = { 1.f, 1.f, 1.f, 1.f, 1.f, 1.f };
hsScalar		plgAudioSys::f2D3DBias = 0.75f;
UInt32			plgAudioSys::fDebugFlags = 0;
hsScalar		plgAudioSys::fStreamingBufferSize = 2.f;
hsScalar		plgAudioSys::fStreamFromRAMCutoff = 10.f;
UInt8			plgAudioSys::fPriorityCutoff = 9;			// We cut off sounds above this priority
hsBool			plgAudioSys::fEnableExtendedLogs = false;
hsScalar		plgAudioSys::fGlobalFadeVolume = 1.f;
hsBool			plgAudioSys::fLogStreamingUpdates = false;
std::string 	plgAudioSys::fDeviceName;
hsBool			plgAudioSys::fRestarting = false;
hsBool			plgAudioSys::fMutedStateChange = false;

void plgAudioSys::Init(hsWindowHndl hWnd)
{
	fSys = TRACKED_NEW plAudioSystem;
	fSys->RegisterAs( kAudioSystem_KEY );
	plgDispatch::Dispatch()->RegisterForExactType( plAudioSysMsg::Index(), fSys->GetKey() );
	plgDispatch::Dispatch()->RegisterForExactType( plRenderMsg::Index(), fSys->GetKey() );

	if(hsPhysicalMemory() <= 380)	
	{
		plStatusLog::AddLineS("audio.log", "StreamFromRam Disabled");
		fStreamFromRAMCutoff = 4;
	}
	fWnd = hWnd;

	if(fMuted)
		SetGlobalFadeVolume(0.0f);

	if( fDelayedActivate )
		Activate( true );
}

void plgAudioSys::SetActive(hsBool b)
{
	fActive = b;
}

void plgAudioSys::SetMuted( hsBool b )
{
	fMuted = b;
	fMutedStateChange = true;

	if(fMuted)
		SetGlobalFadeVolume(0.0f);
	else
		SetGlobalFadeVolume(1.0);
}

void plgAudioSys::SetUseHardware(hsBool b)
{
	fUseHardware = b;
	if( fActive )
		Restart();
}

void plgAudioSys::EnableEAX( hsBool b )
{
	fEnableEAX = b;
	if( fActive )
		Restart();
}

void plgAudioSys::SetAudioMode(AudioMode mode)
{
	if(mode == kDisabled)
	{
		Activate(false);
		return;
	}
	else if(mode == kSoftware)
	{
		fActive = true;
		fUseHardware = false;
		fEnableEAX = false;
	}
	else if(mode == kHardware)
	{
		fActive = true;
		fUseHardware = true;
		fEnableEAX = false;
	}
	else if(mode == kHardwarePlusEAX)
	{
		fActive = true;
		fUseHardware = true;
		fEnableEAX = true;
	}
	Restart();
}

int plgAudioSys::GetAudioMode()
{
	if (fActive)
	{
        if (fUseHardware)
		{
            if (fEnableEAX)
			{
				return kHardwarePlusEAX;
			}
			else
			{
				return kHardware;
			}
		}
		else
		{
			return kSoftware;
		}
	}
	else
	{
		return kDisabled;
	}
}

void plgAudioSys::Restart( void )
{
	if( fSys )
	{
		fSys->fRestartOnDestruct = true;
		Activate( false );
		fRestarting = true;
	}
}

void plgAudioSys::Shutdown()
{
	Activate( false );
	if( fSys )
	{
		fSys->UnRegisterAs( kAudioSystem_KEY );
	}
}

void plgAudioSys::Activate(hsBool b)
{ 
	if( fSys == nil )
	{
		fDelayedActivate = true;
		return;
	}

	if (b == fInit)
		return;
	if (!fActive)
		return;
	if( b )
	{
		plStatusLog::AddLineS( "audio.log", plStatusLog::kBlue, "ASYS: -- Attempting audio system init --" );
		if( !fSys->Init( fWnd ) )
		{
			// Cannot init audio system. Don't activate
			return; 
		}
		fInit = true;
		fSys->SetActive( true );

		if( !IsMuted() )
		{
			SetMuted( true );
			plAudioSysMsg *msg = TRACKED_NEW plAudioSysMsg( plAudioSysMsg::kUnmuteAll );
			msg->SetTimeStamp( hsTimer::GetSysSeconds() );
			msg->AddReceiver( fSys->GetKey() );
			msg->SetBCastFlag( plMessage::kBCastByExactType, false );
			msg->Send();
		}
		return;
	}

	fSys->SetActive( false );
	
	plStatusLog::AddLineS( "audio.log", plStatusLog::kBlue, "ASYS: -- Sending deactivate/destroy messages --" );
	plgDispatch::MsgSend( TRACKED_NEW plAudioSysMsg( plAudioSysMsg::kDeActivate ) );

	// Send ourselves a shutdown message, so that the deactivates get processed first
	fSys->fWaitingForShutdown = true;
	plAudioSysMsg *msg = TRACKED_NEW plAudioSysMsg( plAudioSysMsg::kDestroy );
	msg->SetBCastFlag( plMessage::kBCastByExactType, false );
	msg->Send( fSys->GetKey() );
//	fSys->Shutdown();

	fInit = false;
}

void	plgAudioSys::SetChannelVolume( ASChannel chan, hsScalar vol )
{
	fChannelVolumes[ chan ] = vol;
}

void	plgAudioSys::SetGlobalFadeVolume( hsScalar vol )
{
	if(!fMuted)
		fGlobalFadeVolume = vol;
	else
		fGlobalFadeVolume = 0;
}

hsScalar	plgAudioSys::GetChannelVolume( ASChannel chan )
{
	return fChannelVolumes[ chan ];
}

void	plgAudioSys::NextDebugSound( void )
{
	fSys->NextDebugSound();
}

void plgAudioSys::Set2D3DBias( hsScalar bias )
{
	f2D3DBias = bias;
}

hsScalar plgAudioSys::Get2D3Dbias()
{
	return f2D3DBias;
}

void plgAudioSys::SetDeviceName(const char *device, hsBool restart /* = false */)
{
	fDeviceName = device;
	if(restart)
		Restart();
}

int plgAudioSys::GetNumAudioDevices()
{
	if(fSys)
		return fSys->GetNumAudioDevices();
	return 0;
}

const char *plgAudioSys::GetAudioDeviceName(int index)
{
	if(fSys)
	{
		return fSys->GetAudioDeviceName(index);
	}
	return nil;
}

ALCdevice *plgAudioSys::GetCaptureDevice()
{
	if(fSys)
	{
		return fSys->fCaptureDevice;
	}
	return nil;
}

hsBool plgAudioSys::SupportsEAX(const char *deviceName)
{
	if(fSys)
	{
		return fSys->SupportsEAX(deviceName);
	}
	return nil;
}

void plgAudioSys::RegisterSoftSound( const plKey soundKey )
{
	if(fSys)
	{
		fSys->RegisterSoftSound(soundKey);
	}
}

void plgAudioSys::UnregisterSoftSound( const plKey soundKey )
{
	if(fSys)
	{
		fSys->UnregisterSoftSound(soundKey);
	}
}

void plgAudioSys::SetListenerPos(const hsPoint3 pos)
{
	if(fSys)
	{
		fSys->SetListenerPos(pos);
	}
}

void plgAudioSys::SetListenerVelocity(const hsVector3 vel)
{
	if(fSys)
	{
		fSys->SetListenerVelocity(vel);
	}
}

void plgAudioSys::SetListenerOrientation(const hsVector3 view, const hsVector3 up)
{
	if(fSys)
	{
		fSys->SetListenerOrientation(view, up);
	}
}