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
//  plSound.h - Base sound class header                                     //
//                                                                          //
//// History /////////////////////////////////////////////////////////////////
//                                                                          //
//  10.12.01 mcn    - Added preliminary soft region (volume) support.       //
//  7.12.02 mcn     - Added EAX support                                     //
//  7.15.02 mcn     - Added support for animated volumes                    //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifndef plSound_h
#define plSound_h

#include "hsTemplates.h"
#include "hsGeometry3.h"
#include "plEAXEffects.h"
#include "pnNetCommon/plSynchedObject.h"
#include "plAnimation/plAGChannel.h"
#include "plAnimation/plAGApplicator.h"
#include "plAudioCore/plSoundBuffer.h"

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
#define MCN_HACK_OUR_ATTEN  0
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
        kPropIs3DSound      = 0x00000001,
        kPropDisableLOD     = 0x00000002,
        kPropLooping        = 0x00000004,
        kPropAutoStart      = 0x00000008,
        kPropLocalOnly      = 0x00000010,   // Disables network synching and triggering
        kPropLoadOnlyOnCall = 0x00000020,   // Only load and unload when we're told to
        kPropFullyDisabled  = 0x00000040,   // This sound should never play while this is set
                                            // Only plWin32LinkSound uses it. Placed here as a TODO though...
        kPropDontFade       = 0x00000080,
        kPropIncidental     = 0x00000100    // Incidental sound, will be played thru the incidental manager
    };

    enum Type
    {
        kStartType,
        kSoundFX = kStartType,              // For now, 3D sounds are always marked as this
        kAmbience,
        kBackgroundMusic,
        kGUISound,
        kNPCVoices,
        kNumTypes
    };

    enum Refs
    {
        kRefSoftVolume = 0,
        kRefDataBuffer,     // plugins only
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

            float    fLengthInSecs;      // Time to take to fade
            float    fVolStart;          // Set one of these two for fade in/out,
            float    fVolEnd;            // the other becomes the current volume
            uint8_t  fType;
            bool     fStopWhenDone;      // Actually stop the sound once the fade is complete
            bool     fFadeSoftVol;       // Fade the soft volume instead of fCurrVolume

            plFadeParams() { fLengthInSecs = 0.f; fCurrTime = -1.f; fStopWhenDone = false; fFadeSoftVol = false; fVolStart = fVolEnd = 0.f; fType = kLinear; }

            plFadeParams( Type type, float len, float start, float end )
            {
                fLengthInSecs = len; fVolStart = start; fVolEnd = end; fType = type;
                fStopWhenDone = false;
                fFadeSoftVol = false;
            }

            void    Read( hsStream *s );
            void    Write( hsStream *s );

            float    InterpValue();

        protected:
            float    fCurrTime;          // -1 if we aren't active, else it's how far we're into the animation
    };

    virtual bool        LoadSound( bool is3D ) = 0;
    float            GetVirtualStartTime() const { return (float)fVirtualStartTime; }

    virtual void        Play();
    void                SynchedPlay( unsigned bytes );
    void                SynchedPlay( float virtualStartTime );
    virtual void        Stop();
    virtual void        FastForwardPlay();
    virtual void        FastForwardToggle();
    virtual void        SetMin(const int m); // sets minimum falloff distance
    virtual void        SetMax(const int m); // sets maximum falloff distance
    virtual int         GetMin() const;
    virtual int         GetMax() const;
    virtual void        SetVolume(const float volume);
    virtual float       GetVolume() const { return fCurrVolume; }
    float            GetMaxVolume() { return fMaxVolume; }
    virtual bool        IsPlaying() { return fPlaying; }
    void                SetTime(double t);
    virtual double      GetTime() { return 0.f; }
    virtual void        Activate(bool forcePlay = false);
    virtual void        DeActivate();
    virtual void        SetLength(double l) { fLength = l; }
    virtual void        SetMuted( bool muted );
    virtual bool        IsMuted() { return fMuted; }
    void                Disable() { fDistAttenuation = 0; }
    virtual plSoundMsg* GetStatus(plSoundMsg* pMsg){return NULL;}
    virtual void        SetConeOrientation(float x, float y, float z);
    virtual void        SetOuterVolume( const int v ); // volume for the outer cone (if applicable)
    virtual void        SetConeAngles( int inner, int outer );
    virtual void        SetPosition(const hsPoint3 pos);
    virtual void        SetVelocity(const hsVector3 vel);
    virtual hsPoint3    GetPosition() const;
    virtual hsVector3   GetVelocity() const;

    virtual void        Update();
    
    plSoundBuffer *     GetDataBuffer() const { return (plSoundBuffer *)fDataBufferKey->ObjectIsLoaded(); }
    float               QueryCurrVolume() const;  // Returns the current volume, attenuated

    plFileName          GetFileName() const;
    virtual double      GetLength();

    void                SetProperty( Property prop, bool on ) { if( on ) fProperties |= prop; else fProperties &= ~prop; }
    bool                IsPropertySet( Property prop ) const { return ( fProperties & prop ) ? true : false; }

    virtual void        RefreshVolume();

    virtual void        SetStartPos(unsigned bytes) = 0;
    virtual unsigned    GetByteOffset(){return 0;}
    virtual float       GetActualTimeSec() = 0;

    virtual void        AddCallbacks(plSoundMsg* pMsg) = 0;
    virtual void        RemoveCallbacks(plSoundMsg* pMsg) = 0;

    virtual uint8_t       GetChannelSelect() const { return 0; }    // Only defined on Win32Sound right now, should be here tho

    virtual void        Read(hsStream* s, hsResMgr* mgr);
    virtual void        Write(hsStream* s, hsResMgr* mgr);
    
    virtual void        SetFadeInEffect( plFadeParams::Type type, float length );
    virtual void        SetFadeOutEffect( plFadeParams::Type type, float length );
    virtual float    CalcSoftVolume( bool enable, float distToListenerSquared );
    virtual void        UpdateSoftVolume( bool enable, bool firstTime = false );

    virtual bool        MsgReceive( plMessage* pMsg );
    virtual bool        DirtySynchState( const ST::string &sdlName = {}, uint32_t sendFlags = 0 ); // call when state has changed

    // Tests whether this sound is within range of the given position, not counting soft regions
    bool                IsWithinRange( const hsPoint3 &listenerPos, float *distSquared );

    // Type setting and getting, from the Types enum
    void                SetType( uint8_t type ) { fType = type; }
    uint8_t               GetType() const { return fType; }

    // Priority stuff
    void                SetPriority( uint8_t pri ) { fPriority = pri; }
    uint8_t               GetPriority() const { return fPriority; }

    // Visualization
    virtual plDrawableSpans*    CreateProxy(const hsMatrix44& l2w, hsGMaterial* mat, hsTArray<uint32_t>& idx, plDrawableSpans* addTo);

    // Forced loading/unloading (for when the audio system's LOD just doesn't cut it)
    virtual void        ForceLoad(  );
    virtual void        ForceUnload();

    // Note: ONLY THE AUDIOSYS SHOULD CALL THIS. If you're not the audioSys, get lost.
    static void         SetCurrDebugPlate( const plKey& soundKey );

    void                RegisterOnAudioSys();
    void                UnregisterOnAudioSys();

    // Also only for the audio system
    float            GetVolumeRank();
    void                ForceUnregisterFromAudioSys();

    static void         SetLoadOnDemand( bool activate ) { fLoadOnDemandFlag = activate; }
    static void         SetLoadFromDiskOnDemand( bool activate ) { fLoadFromDiskOnDemand = activate; }

    const plEAXSourceSettings   &GetEAXSettings() const { return fEAXSettings; }
    plEAXSourceSettings         &GetEAXSettings() { return fEAXSettings; }
    virtual StreamType          GetStreamType() const { return kNoStream; }
    virtual void    FreeSoundData();


protected:
    bool        fPlaying;
    bool        fActive;
    double      fTime;
    int         fMaxFalloff;
    int         fMinFalloff;
    float    fCurrVolume;
    float    fDesiredVol;        // Equal to fCurrVolume except when we're fading or muted
    float    fFadedVolume;
    float    fMaxVolume;

    int         fOuterVol;
    int         fInnerCone;
    int         fOuterCone;
    double      fLength;
    
    int         fProperties;
    uint8_t       fType;
    uint8_t       fPriority;

    bool        fMuted, fFading, fRegisteredForTime, fPlayOnReactivate, fFreeData;
    bool        fNotHighEnoughPriority;     // Set whenever the audioSys calls UpdateSoftVolume() with enable=false,
                                            // thus indicating that we slipped off the top 16 most wanted list. 

    // Do these need to be synched values? They weren't before...
    hsVector3   fConeOrientation;
    hsPoint3    f3DPosition;
    hsVector3   f3DVelocity;
    bool        fPlayWhenLoaded;

    double      fSynchedStartTimeSec;
    
    // Just around for reference and sending messages upward (synched state)
    plSceneObject       *fOwningSceneObject;

    // EAX Settings storage here
    plEAXSourceSettings fEAXSettings;
    bool fQueued;

    plFadeParams    fFadeInParams, fFadeOutParams;
    plFadeParams    fCoolSoftVolumeTrickParams;
    plFadeParams    *fCurrFadeParams;

    plSoftVolume    *fSoftRegion;
    float        fSoftVolume;
    float        fDistAttenuation, fDistToListenerSquared;
    double          fVirtualStartTime;
    bool            fRegistered;
    static unsigned fIncidentalsPlaying;

    plSoftVolume    *fSoftOcclusionRegion;

    plSoundBuffer   *fDataBuffer;           // Not always around
    bool            fDataBufferLoaded;
    plKey           fDataBufferKey;     // Always around

    static plGraphPlate *fDebugPlate;
    static plSound      *fCurrDebugPlateSound;

    static bool         fLoadOnDemandFlag, fLoadFromDiskOnDemand;
    bool                fLoading;

    void            IUpdateDebugPlate();
    void            IPrintDbgMessage( const char *msg, bool isErr = false );

    virtual void    ISetActualVolume(float v) = 0;
    virtual void    IActuallyStop();
    virtual bool    IActuallyPlaying() = 0;
    virtual void    IActuallyPlay() = 0;
    virtual void    IFreeBuffers() = 0;

    //NOTE: if isIncidental is true the entire sound will be loaded. 
    virtual plSoundBuffer::ELoadReturnVal   IPreLoadBuffer( bool playWhenLoaded, bool isIncidental = false );   
    virtual void        ISetActualTime( double t ) = 0;
    
    virtual bool        IActuallyLoaded() = 0;
    virtual void        IRefreshEAXSettings( bool force = false ) = 0;

    virtual float    IGetChannelVolume() const;

    void    ISynchToStartTime();
    void    ISynchedPlay( double virtualStartTime );
    void    IStartFade( plFadeParams *params, float offsetIntoFade = 0.f );
    void    IStopFade( bool shuttingDown = false, bool SetVolEnd = true);
    
    bool    IWillBeAbleToPlay();

    void        ISetSoftRegion( plSoftVolume *region );
    float    IAttenuateActualVolume( float volume ) const;
    void        ISetSoftOcclusionRegion( plSoftVolume *region );

    // Override to make sure the buffer is available before the base class is called
    virtual void    IRefreshParams();

    virtual bool    ILoadDataBuffer();
    virtual void    IUnloadDataBuffer();

    //virtual void  ISetMinDistance( const int m ) = 0;
    //virtual void  ISetMaxDistance( const int m ) = 0;
    //virtual void  ISetOuterVolume( const int v ) = 0;
    //virtual void  ISetConeAngles( int inner, int outer ) = 0;
    //virtual void  ISetActualConeOrient( hsVector3 &vector ) = 0;
    //virtual void  ISetVelocity( const hsVector3 vel ) = 0;
    //virtual void  ISetPosition( const hsPoint3 pos ) = 0;

    virtual void    IRead( hsStream *s, hsResMgr *mgr );
    virtual void    IWrite( hsStream *s, hsResMgr *mgr );
};


//// plSoundVolumeApplicator /////////////////////////////////////////////////
//  Tiny helper for handling animated volumes

class plSoundVolumeApplicator : public plAGApplicator
{
public:
    plSoundVolumeApplicator() { }
    plSoundVolumeApplicator( uint32_t index ) { fIndex = index; }

    CLASSNAME_REGISTER( plSoundVolumeApplicator );
    GETINTERFACE_ANY( plSoundVolumeApplicator, plAGApplicator );

    virtual plAGApplicator *CloneWithChannel( plAGChannel *channel );
    virtual void            Write( hsStream *stream, hsResMgr *mgr );
    virtual void            Read( hsStream *s, hsResMgr *mgr );

protected:
    uint32_t      fIndex;
    virtual void IApply( const plAGModifier *mod, double time );
};

#endif //plWin32Sound_h
