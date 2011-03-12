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
#ifndef plAudioSystem_h
#define plAudioSystem_h

#include "hsStlUtils.h"
#include "hsWindowHndl.h"
#include "hsTemplates.h"
#include "hsGeometry3.h"
#include "../pnKeyedObject/hsKeyedObject.h"

#define DEFAULT_AUDIO_DEVICE_NAME "Generic Software"

typedef wchar_t WCHAR;

class plSound;
class plSoftSoundNode;
class plgAudioSys;
class plStatusLog;
class plEAXListenerMod;

typedef struct ALCdevice_struct ALCdevice;
typedef struct ALCcontext_struct ALCcontext;


class DeviceDescriptor
{
public:
	DeviceDescriptor(const char *name, hsBool supportsEAX):
	fDeviceName(name),
	fSupportsEAX(supportsEAX)
	{
	}
	const char *GetDeviceName() { return fDeviceName.c_str();}
	hsBool SupportsEAX() { return fSupportsEAX; }

private:
	std::string fDeviceName;
	hsBool fSupportsEAX;
};

class plAudioSystem : public hsKeyedObject
{
public:
	plAudioSystem();
	~plAudioSystem();

	CLASSNAME_REGISTER( plAudioSystem );
	GETINTERFACE_ANY( plAudioSystem, hsKeyedObject );

	enum
	{
		kThreadSndRef = 0,
		kRefEAXRegion
	};

	hsBool	Init(hsWindowHndl hWnd);
	void	Shutdown();

	void	SetActive( hsBool b );
	
	void SetListenerPos(const hsPoint3 pos);
	void SetListenerVelocity(const hsVector3 vel);
	void SetListenerOrientation(const hsVector3 view, const hsVector3 up);
	void SetMaxNumberOfActiveSounds();		// sets the max number of active sounds based on the priority cutoff
	void SetDistanceModel(int i);
	
	virtual hsBool MsgReceive(plMessage* msg);
	double GetTime();
	
	void		NextDebugSound( void );
	hsPoint3	GetCurrListenerPos( void ) const { return fCurrListenerPos; }

	int			GetNumAudioDevices();
	const char *GetAudioDeviceName(int index);
	hsBool		SupportsEAX(const char *deviceName);

	void		SetFadeLength(float lengthSec);
	
protected:

	friend class plgAudioSys;

	ALCdevice *		fDevice;
	ALCcontext *	fContext;
	ALCdevice *		fCaptureDevice;
	
	plSoftSoundNode		*fSoftRegionSounds;
	plSoftSoundNode		*fActiveSofts;
	plStatusLog			*fDebugActiveSoundDisplay;

	static Int32		fMaxNumSounds, fNumSoundsSlop;		// max number of sounds the engine is allowed to audibly play. Different than fMaxNumSources. That is the max number of sounds the audio card can play
	plSoftSoundNode		*fCurrDebugSound;
	hsTArray<plKey>		fPendingRegisters;

	hsPoint3	fCurrListenerPos;//, fCommittedListenerPos;
	hsBool		fActive, fUsingEAX, fRestartOnDestruct, fWaitingForShutdown;
	__int64		fStartTime;

	hsTArray<hsKeyedObject *>		fMyRefs;
	hsTArray<plEAXListenerMod *>	fEAXRegions;

	hsPoint3			fLastPos;
	hsBool				fAvatarPosSet;		// used for listener stuff

	hsBool				fDisplayNumBuffers;
	
	std::vector<DeviceDescriptor> fDeviceList;		// list of openal device names

	double			fStartFade;
	float			fFadeLength;
	unsigned int	fMaxNumSources;		// max openal sources
	double			fLastUpdateTimeMs;

	void	RegisterSoftSound( const plKey soundKey );
	void	UnregisterSoftSound( const plKey soundKey );
	void	IUpdateSoftSounds( const hsPoint3 &newPosition );
	UInt32 	IScaleVolume(float volume);
	void	IEnumerateDevices();

public:
	hsBool							fListenerInit;
};

class plgAudioSys
{
public:
	enum ASChannel
	{
		kSoundFX,
		kAmbience,
		kBgndMusic,
		kGUI,
		kNPCVoice,
		kVoice,
		kNumChannels
	};

	enum DebugFlags
	{
		kDisableRightSelect = 0x00000001,
		kDisableLeftSelect	= 0x00000002
	};

	enum AudioMode
	{
		kDisabled,
		kSoftware,
		kHardware,
		kHardwarePlusEAX,
	};
	static void Init(hsWindowHndl hWnd);
	static hsBool Hardware() { return fUseHardware; }
	static void SetUseHardware(hsBool b);
	static void SetActive(hsBool b);
	static void	SetMuted( hsBool b );
	static void	EnableEAX( hsBool b );
	static hsBool Active() { return fInit; }
	static void Shutdown();
	static void Activate(hsBool b);
	static hsBool	IsMuted( void ) { return fMuted; }
	static hsWindowHndl hWnd() { return fWnd; }
	static plAudioSystem* Sys() { return fSys; }
	static void	Restart( void );
	static hsBool	UsingEAX( void ) { return fSys->fUsingEAX; }

	static void	NextDebugSound( void );

	static void		SetChannelVolume( ASChannel chan, hsScalar vol );
	static hsScalar	GetChannelVolume( ASChannel chan );

	static void		Set2D3DBias( hsScalar bias );
	static hsScalar	Get2D3Dbias();

	static void		SetGlobalFadeVolume( hsScalar vol );
	static hsScalar	GetGlobalFadeVolume( void ) { return fGlobalFadeVolume; }

	static void		SetDebugFlag( UInt32 flag, hsBool set = true ) { if( set ) fDebugFlags |= flag; else fDebugFlags &= ~flag; }
	static hsBool	IsDebugFlagSet( UInt32 flag ) { return fDebugFlags & flag; }
	static void		ClearDebugFlags( void ) { fDebugFlags = 0; }

	static hsScalar	GetStreamingBufferSize( void ) { return fStreamingBufferSize; }
	static void		SetStreamingBufferSize( hsScalar size ) { fStreamingBufferSize = size; }

	static UInt8	GetPriorityCutoff( void ) { return fPriorityCutoff; }
	static void		SetPriorityCutoff( UInt8 cut ) { fPriorityCutoff = cut;  if(fSys) fSys->SetMaxNumberOfActiveSounds(); }

	static hsBool	AreExtendedLogsEnabled( void ) { return fEnableExtendedLogs; }
	static void		EnableExtendedLogs( hsBool e ) { fEnableExtendedLogs = e; }

	static hsScalar	GetStreamFromRAMCutoff( void ) { return fStreamFromRAMCutoff; }
	static void		SetStreamFromRAMCutoff( hsScalar c ) { fStreamFromRAMCutoff = c; }

	static void SetListenerPos(const hsPoint3 pos);
	static void SetListenerVelocity(const hsVector3 vel);
	static void SetListenerOrientation(const hsVector3 view, const hsVector3 up);

	static void ShowNumBuffers(hsBool b) { if(fSys) fSys->fDisplayNumBuffers = b; }

	static void SetAudioMode(AudioMode mode);
	static int GetAudioMode();
	static hsBool LogStreamingUpdates() { return fLogStreamingUpdates; }
	static void SetLogStreamingUpdates(hsBool logUpdates) { fLogStreamingUpdates = logUpdates; }
	static void SetDeviceName(const char *device, hsBool restart = false);
	static const char *GetDeviceName() { return fDeviceName.c_str(); }
	static int GetNumAudioDevices();
	static const char *GetAudioDeviceName(int index);
	static ALCdevice *GetCaptureDevice();
	static hsBool SupportsEAX(const char *deviceName);
	static void	RegisterSoftSound( const plKey soundKey );
	static void	UnregisterSoftSound( const plKey soundKey );

	static hsBool IsRestarting() {return fRestarting;}

private:
	friend class plAudioSystem;

	static plAudioSystem*		fSys;
	static hsBool				fInit;
	static hsBool				fActive;
	static hsBool				fMuted;
	static hsWindowHndl			fWnd;
	static hsBool				fUseHardware;
	static hsBool				fDelayedActivate;
	static hsScalar				fChannelVolumes[ kNumChannels ];
	static hsScalar				fGlobalFadeVolume;
	static UInt32				fDebugFlags;
	static hsBool				fEnableEAX;
	static hsScalar				fStreamingBufferSize;
	static UInt8				fPriorityCutoff;
	static hsBool				fEnableExtendedLogs;
	static hsScalar				fStreamFromRAMCutoff;
	static hsScalar				f2D3DBias;
	static hsBool				fLogStreamingUpdates;
	static std::string			fDeviceName;
	static hsBool				fRestarting;
	static hsBool				fMutedStateChange;

};

#endif //plAudioSystem_h
