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
//	plSound.h - Base sound class header										//
//																			//
//// History /////////////////////////////////////////////////////////////////
//																			//
//	10.12.01 mcn	- Added preliminary soft region (volume) support.		//
//	7.12.02 mcn		- Added EAX support										//
//	7.15.02 mcn		- Added support for animated volumes					//
//																			//
//////////////////////////////////////////////////////////////////////////////

#ifndef plSound_h
#define plSound_h

#include "hsTemplates.h"
#include "hsGeometry3.h"
#include "plEAXEffects.h"
#include "../pnNetCommon/plSynchedObject.h"
#include "../plAvatar/plAGChannel.h"
#include "../plAvatar/plAGApplicator.h"
#include "../plAudioCore/plSoundBuffer.h"

class hsResMgr;
class hsStream;
class plSoundProxy;
class plDrawableSpans;
class hsGMaterial;
class plSoundMsg;
class plSoftVolume;
class plGraphPlate;
struct hsMatrix44;
class plSoundBuffer;
class plSceneObject;
class plSoundVolumeApplicator;

// Set this to 1 to do our own distance attenuation (model doesn't work yet tho)
#define MCN_HACK_OUR_ATTEN	0
#define MAX_INCIDENTALS 4

class plSound : public plSynchedObject
{
	friend class plSoundSDLModifier;
	friend class plSoundVolumeApplicator;

public:
	plSound();
	virtual ~plSound();

	CLASSNAME_REGISTER( plSound );
	GETINTERFACE_ANY( plSound, plSynchedObject );

	enum Property
	{
		kPropIs3DSound		= 0x00000001,
		kPropDisableLOD		= 0x00000002,
		kPropLooping		= 0x00000004,
		kPropAutoStart		= 0x00000008,
		kPropLocalOnly		= 0x00000010,	// Disables network synching and triggering
		kPropLoadOnlyOnCall	= 0x00000020,	// Only load and unload when we're told to
		kPropFullyDisabled  = 0x00000040,	// This sound should never play while this is set
											// Only plWin32LinkSound uses it. Placed here as a TODO though...
		kPropDontFade		= 0x00000080,
		kPropIncidental		= 0x00000100	// Incidental sound, will be played thru the incidental manager
	};

	enum Type
	{
		kStartType,
		kSoundFX = kStartType,				// For now, 3D sounds are always marked as this
		kAmbience,
		kBackgroundMusic,
		kGUISound,
		kNPCVoices,
		kNumTypes
	};

	enum Refs
	{
		kRefSoftVolume = 0,
		kRefDataBuffer,		// plugins only
		kRefParentSceneObject,
		kRefSoftOcclusionRegion
	};

	enum 
	{
		kSoftRegion = 0
	};

	enum StreamType
	{ 
		kNoStream, 
		kStreamFromRAM, 
		kStreamFromDisk, 
		kStreamCompressed 
	};

	class plFadeParams
	{
		friend class plSound;

		public:
			enum Type
			{
				kLinear,
				kLogarithmic,
				kExponential
			};

			hsScalar	fLengthInSecs;		// Time to take to fade
			hsScalar	fVolStart;			// Set one of these two for fade in/out,
			hsScalar	fVolEnd;			// the other becomes the current volume
			UInt8		fType;
			hsBool		fStopWhenDone;		// Actually stop the sound once the fade is complete
			hsBool		fFadeSoftVol;		// Fade the soft volume instead of fCurrVolume

			plFadeParams() { fLengthInSecs = 0.f; fCurrTime = -1.f; fStopWhenDone = false; fFadeSoftVol = false; fVolStart = fVolEnd = 0.f; fType = kLinear; }

			plFadeParams( Type type, hsScalar len, hsScalar start, hsScalar end )
			{
				fLengthInSecs = len; fVolStart = start; fVolEnd = end; fType = type;
				fStopWhenDone = false;
				fFadeSoftVol = false;
			}

			void	Read( hsStream *s );
			void	Write( hsStream *s );

			hsScalar	InterpValue( void );

		protected:
			hsScalar	fCurrTime;			// -1 if we aren't active, else it's how far we're into the animation
	};

	virtual hsBool		LoadSound( hsBool is3D ) = 0;
	hsScalar			GetVirtualStartTime( void ) const { return (hsScalar)fVirtualStartTime; }

	virtual void 		Play();
	void				SynchedPlay( unsigned bytes );
	void				SynchedPlay( hsScalar virtualStartTime );
	virtual void 		Stop();
	virtual void 		FastForwardPlay();
	virtual void 		FastForwardToggle();
	virtual void 		SetMin(const int m); // sets minimum falloff distance
	virtual void 		SetMax(const int m); // sets maximum falloff distance
	virtual int			GetMin() const;
	virtual int			GetMax() const;
	virtual void		SetVolume(const float volume);
	virtual float		GetVolume(void) const { return fCurrVolume; }
	hsScalar			GetMaxVolume() { return fMaxVolume; }
	virtual hsBool		IsPlaying() { return fPlaying; }
	void				SetTime(double t);
	virtual double		GetTime( void ) { return 0.f; }
	virtual void		Activate(hsBool forcePlay = false);
	virtual void		DeActivate();
	virtual void		SetLength(double l) { fLength = l; }
	virtual void		SetMuted( hsBool muted );
	virtual hsBool		IsMuted( void ) { return fMuted; }
	void				Disable() { fDistAttenuation = 0; }
	virtual plSoundMsg* GetStatus(plSoundMsg* pMsg){return NULL;}
	virtual void		SetConeOrientation(hsScalar x, hsScalar y, hsScalar z);
	virtual void		SetOuterVolume( const int v ); // volume for the outer cone (if applicable)
	virtual void		SetConeAngles( int inner, int outer );
	virtual void		SetPosition(const hsPoint3 pos);
	virtual void		SetVelocity(const hsVector3 vel);
	virtual hsPoint3	GetPosition() const;
	virtual hsVector3	GetVelocity() const;

	virtual void		Update();
	
	plSoundBuffer *		GetDataBuffer( void ) const { return (plSoundBuffer *)fDataBufferKey->ObjectIsLoaded(); }
	hsScalar			QueryCurrVolume( void ) const;	// Returns the current volume, attenuated

	const char *		GetFileName( void ) const;
	virtual double		GetLength();

	void				SetProperty( Property prop, hsBool on ) { if( on ) fProperties |= prop; else fProperties &= ~prop; }
	hsBool				IsPropertySet( Property prop ) const { return ( fProperties & prop ) ? true : false; }

	virtual void		RefreshVolume( void );

	virtual void		SetStartPos(unsigned bytes) = 0;
	virtual unsigned	GetByteOffset(){return 0;}
	virtual float		GetActualTimeSec() = 0;

	virtual	void		AddCallbacks(plSoundMsg* pMsg) = 0;
	virtual void		RemoveCallbacks(plSoundMsg* pMsg) = 0;

	virtual UInt8		GetChannelSelect( void ) const { return 0; }	// Only defined on Win32Sound right now, should be here tho

	virtual void		Read(hsStream* s, hsResMgr* mgr);
	virtual void		Write(hsStream* s, hsResMgr* mgr);
	
	virtual void		SetFadeInEffect( plFadeParams::Type type, hsScalar length );
	virtual void		SetFadeOutEffect( plFadeParams::Type type, hsScalar length );
	virtual hsScalar	CalcSoftVolume( hsBool enable, hsScalar distToListenerSquared );
	virtual void		UpdateSoftVolume( hsBool enable, hsBool firstTime = false );

	virtual hsBool		MsgReceive( plMessage* pMsg );
	virtual hsBool		DirtySynchState( const char *sdlName = nil, UInt32 sendFlags = 0 );	// call when state has changed

	// Tests whether this sound is within range of the given position, not counting soft regions
	hsBool				IsWithinRange( const hsPoint3 &listenerPos, hsScalar *distSquared );

	// Type setting and getting, from the Types enum
	void				SetType( UInt8 type ) { fType = type; }
	UInt8				GetType( void ) const { return fType; }

	// Priority stuff
	void				SetPriority( UInt8 pri ) { fPriority = pri; }
	UInt8				GetPriority( void ) const { return fPriority; }

	// Visualization
	virtual plDrawableSpans*	CreateProxy(const hsMatrix44& l2w, hsGMaterial* mat, hsTArray<UInt32>& idx, plDrawableSpans* addTo);

	// Forced loading/unloading (for when the audio system's LOD just doesn't cut it)
	virtual void		ForceLoad(  );
	virtual void		ForceUnload( void );

	// Note: ONLY THE AUDIOSYS SHOULD CALL THIS. If you're not the audioSys, get lost.
	static void			SetCurrDebugPlate( const plKey soundKey );

	void				RegisterOnAudioSys( void );
	void				UnregisterOnAudioSys( void );

	// Also only for the audio system
	hsScalar			GetVolumeRank( void );
	void				ForceUnregisterFromAudioSys( void );

	static void			SetLoadOnDemand( hsBool activate ) { fLoadOnDemandFlag = activate; }
	static void			SetLoadFromDiskOnDemand( hsBool activate ) { fLoadFromDiskOnDemand = activate; }

	const plEAXSourceSettings	&GetEAXSettings( void ) const { return fEAXSettings; }
	plEAXSourceSettings			&GetEAXSettings( void ) { return fEAXSettings; }
	virtual StreamType			GetStreamType() const { return kNoStream; }
	virtual void	FreeSoundData();


protected:
	hsBool		fPlaying;
	hsBool		fActive;
	double		fTime;
	int			fMaxFalloff;
	int			fMinFalloff;
	hsScalar	fCurrVolume;
	hsScalar	fDesiredVol;		// Equal to fCurrVolume except when we're fading or muted
	hsScalar	fFadedVolume;
	hsScalar	fMaxVolume;

	int			fOuterVol;
	int			fInnerCone;
	int			fOuterCone;
	double		fLength;
	
	int			fProperties;
	UInt8		fType;
	UInt8		fPriority;

	hsBool		fMuted, fFading, fRegisteredForTime, fPlayOnReactivate, fFreeData;
	hsBool		fNotHighEnoughPriority;		// Set whenever the audioSys calls UpdateSoftVolume() with enable=false,
											// thus indicating that we slipped off the top 16 most wanted list. 

	// Do these need to be synched values? They weren't before...
	hsVector3	fConeOrientation;
	hsPoint3	f3DPosition;
	hsVector3	f3DVelocity;
	hsBool		fPlayWhenLoaded;

	double		fSynchedStartTimeSec;
	
	// Just around for reference and sending messages upward (synched state)
	plSceneObject		*fOwningSceneObject;

	// EAX Settings storage here
	plEAXSourceSettings	fEAXSettings;
	hsBool fQueued;

	plFadeParams	fFadeInParams, fFadeOutParams;
	plFadeParams	fCoolSoftVolumeTrickParams;
	plFadeParams	*fCurrFadeParams;

	plSoftVolume	*fSoftRegion;
	hsScalar		fSoftVolume;
	hsScalar		fDistAttenuation, fDistToListenerSquared;
	double			fVirtualStartTime;
	hsBool			fRegistered;
	static unsigned	fIncidentalsPlaying;

	plSoftVolume	*fSoftOcclusionRegion;

	plSoundBuffer	*fDataBuffer;			// Not always around
	hsBool			fDataBufferLoaded;
	plKey			fDataBufferKey;		// Always around

	static plGraphPlate	*fDebugPlate;
	static plSound		*fCurrDebugPlateSound;

	static hsBool		fLoadOnDemandFlag, fLoadFromDiskOnDemand;
	hsBool				fLoading;

	void			IUpdateDebugPlate( void );
	void			IPrintDbgMessage( const char *msg, hsBool isErr = false );

	virtual void	ISetActualVolume(const float v) = 0;
	virtual void	IActuallyStop( void );
	virtual hsBool	IActuallyPlaying( void ) = 0;
	virtual void	IActuallyPlay( void ) = 0;
	virtual void	IFreeBuffers( void ) = 0;

	//NOTE: if isIncidental is true the entire sound will be loaded. 
	virtual plSoundBuffer::ELoadReturnVal	IPreLoadBuffer( hsBool playWhenLoaded, hsBool isIncidental = false );	
	virtual void		ISetActualTime( double t ) = 0;
	
	virtual hsBool		IActuallyLoaded( void ) = 0;
	virtual void		IRefreshEAXSettings( hsBool force = false ) = 0;

	virtual hsScalar	IGetChannelVolume( void ) const;

	void	ISynchToStartTime( void );
	void	ISynchedPlay( double virtualStartTime );
	void	IStartFade( plFadeParams *params, hsScalar offsetIntoFade = 0.f );
	void	IStopFade( hsBool shuttingDown = false, hsBool SetVolEnd = true);
	
	hsBool	IWillBeAbleToPlay( void );

	void		ISetSoftRegion( plSoftVolume *region );
	hsScalar	IAttenuateActualVolume( hsScalar volume ) const;
	void		ISetSoftOcclusionRegion( plSoftVolume *region );

	// Override to make sure the buffer is available before the base class is called
	virtual void	IRefreshParams( void );

	virtual bool	ILoadDataBuffer( void );
	virtual void	IUnloadDataBuffer( void );

	//virtual void	ISetMinDistance( const int m ) = 0;
	//virtual void	ISetMaxDistance( const int m ) = 0;
	//virtual void	ISetOuterVolume( const int v ) = 0;
	//virtual void	ISetConeAngles( int inner, int outer ) = 0;
	//virtual void	ISetActualConeOrient( hsVector3 &vector ) = 0;
	//virtual void	ISetVelocity( const hsVector3 vel ) = 0;
	//virtual void	ISetPosition( const hsPoint3 pos ) = 0;

	virtual void	IRead( hsStream *s, hsResMgr *mgr );
	virtual void	IWrite( hsStream *s, hsResMgr *mgr );
};


//// plSoundVolumeApplicator /////////////////////////////////////////////////
//	Tiny helper for handling animated volumes

class plSoundVolumeApplicator : public plAGApplicator
{
public:
	plSoundVolumeApplicator() { }
	plSoundVolumeApplicator( UInt32 index ) { fIndex = index; }

	CLASSNAME_REGISTER( plSoundVolumeApplicator );
	GETINTERFACE_ANY( plSoundVolumeApplicator, plAGApplicator );

	virtual plAGApplicator *CloneWithChannel( plAGChannel *channel );
	virtual void			Write( hsStream *stream, hsResMgr *mgr );
	virtual void			Read( hsStream *s, hsResMgr *mgr );

protected:
	UInt32		fIndex;
	virtual void IApply( const plAGModifier *mod, double time );
};

#endif //plWin32Sound_h
