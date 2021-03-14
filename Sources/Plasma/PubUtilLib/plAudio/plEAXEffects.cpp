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
//  plEAXEffects - Various classes and wrappers to support EAX              //
//                  acceleration.                                           //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include "HeadSpin.h"
#include "hsWindows.h"
#include "hsThread.h"

#ifndef EAX_SDK_AVAILABLE
#   include "plEAXStructures.h"
#endif

#include "plEAXEffects.h"
#include "plAudioCore/plAudioCore.h"
#include "plDSoundBuffer.h"
#include "plEAXListenerMod.h"
#include "hsStream.h"
#include "plAudioSystem.h"
#include <al.h>

#ifdef EAX_SDK_AVAILABLE
#include <eax.h>
#include <eax-util.h>
#include <eaxlegacy.h>
#endif
#include "plStatusLog/plStatusLog.h"

#define DebugLog   if (myLog) myLog->AddLine

#ifdef EAX_SDK_AVAILABLE
static EAXGet           s_EAXGet;
static EAXSet           s_EAXSet;
#endif


//// GetInstance /////////////////////////////////////////////////////////////

plEAXListener   &plEAXListener::GetInstance()
{
    static plEAXListener    instance;
    return instance;
}

//// Constructor/Destructor //////////////////////////////////////////////////

plEAXListener::plEAXListener()
{
    fInited = false;
    ClearProcessCache();
}

plEAXListener::~plEAXListener()
{
    Shutdown();
}

//// Init ////////////////////////////////////////////////////////////////////

bool    plEAXListener::Init()
{
#ifdef EAX_SDK_AVAILABLE
    if( fInited )
        return true;

    if(!alIsExtensionPresent((ALchar *)"EAX4.0"))       // is eax 4 supported
    {
        if(!alIsExtensionPresent((ALchar *) "EAX4.0Emulated"))      // is an earlier version of eax supported
        {
            plStatusLog::AddLineS("audio.log", "EAX not supported");
            return false;
        }
        else
        {
            plStatusLog::AddLineS("audio.log", "EAX 4 Emulated supported");
        }
    }
    else
    {
        plStatusLog::AddLineS("audio.log", "EAX 4 available");
    }
    
    // EAX is supported 
    s_EAXGet = (EAXGet)alGetProcAddress((ALchar *)"EAXGet");
    s_EAXSet = (EAXSet)alGetProcAddress((ALchar *)"EAXSet");
    if(!s_EAXGet || ! s_EAXSet)
    {
        IFail( "EAX initialization failed", true );
        return false;
    }
    fInited = true;

#if 1
    try
    {
        // Make an EAX call here to prevent problems on WDM driver
        unsigned int lRoom = -10000;

        SetGlobalEAXProperty(DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_ROOM, &lRoom, sizeof( unsigned int ));
    }
    catch (std::exception &e)
    {
        plStatusLog::AddLineS("audio.log", "Unable to set EAX Property Set ({}), disabling EAX...", e.what());
        plgAudioSys::EnableEAX(false);
        return false;
    }
    catch (...)
    {
        plStatusLog::AddLineS("audio.log", "Unable to set EAX Property Set, disabling EAX...");
        plgAudioSys::EnableEAX(false);
        return false;
    }
#endif

    ClearProcessCache();

    return true;
#else /* !EAX_SDK_AVAILABLE */
    plStatusLog::AddLineS("audio.log", "EAX disabled in this build");
    return false;
#endif
}

//// Shutdown ////////////////////////////////////////////////////////////////

void    plEAXListener::Shutdown()
{
    if( !fInited )
        return;

#ifdef EAX_SDK_AVAILABLE
    s_EAXSet = nullptr;
    s_EAXGet = nullptr;
#endif
    IRelease();
}


bool plEAXListener::SetGlobalEAXProperty(GUID guid, unsigned long ulProperty, void *pData, unsigned long ulDataSize )
{
    if(fInited)
    {
#ifdef EAX_SDK_AVAILABLE
        return s_EAXSet(&guid, ulProperty, 0, pData, ulDataSize) == AL_NO_ERROR;
#endif
    }
    return false;
}

bool plEAXListener::GetGlobalEAXProperty(GUID guid, unsigned long ulProperty, void *pData, unsigned long ulDataSize)
{
    if(fInited)
    {
#ifdef EAX_SDK_AVAILABLE
        return s_EAXGet(&guid, ulProperty, 0, pData, ulDataSize) == AL_NO_ERROR;
#endif
    }
    return false;
}

bool plEAXSource::SetSourceEAXProperty(unsigned source, GUID guid, unsigned long ulProperty, void *pData, unsigned long ulDataSize)
{
#ifdef EAX_SDK_AVAILABLE
    return s_EAXSet(&guid, ulProperty, source, pData, ulDataSize) == AL_NO_ERROR;
#else
    return false;
#endif
}

bool plEAXSource::GetSourceEAXProperty(unsigned source, GUID guid, unsigned long ulProperty, void *pData, unsigned long ulDataSize)
{
#ifdef EAX_SDK_AVAILABLE
    return s_EAXGet(&guid, ulProperty, source, pData, ulDataSize) == AL_NO_ERROR;
#else
    return false;
#endif
}


//// IRelease ////////////////////////////////////////////////////////////////

void    plEAXListener::IRelease()
{
    fInited = false;
}

//// IFail ///////////////////////////////////////////////////////////////////

void    plEAXListener::IFail(  bool major )
{
    plStatusLog::AddLineS( "audio.log", plStatusLog::kRed,
                            "ERROR in plEAXListener: Could not set global eax params");

    if( major )
        IRelease();
}

void    plEAXListener::IFail( const char *msg, bool major )
{
    plStatusLog::AddLineSF( "audio.log", plStatusLog::kRed,
                            "ERROR in plEAXListener: {}", msg );

    if( major )
        IRelease();
}

//// IMuteProperties /////////////////////////////////////////////////////////
//  Mutes the given properties, so if you have some props that you want
//  half strength, this function will do it for ya.

void    plEAXListener::IMuteProperties( EAXREVERBPROPERTIES *props, float percent )
{
    // We only mute the room, roomHF and roomLF, since those control the overall effect
    // application. All three are a direct linear blend as defined by eax-util.cpp, so
    // this should be rather easy

    float invPercent = 1.f - percent;

    // The old way, as dictated by EAX sample code...
#ifdef EAX_SDK_AVAILABLE
    props->lRoom   = (int)( ( (float)EAXLISTENER_MINROOM   * invPercent ) + ( (float)props->lRoom   * percent ) );
#endif
    // The new way, as suggested by EAX guys...
//  props->lRoom = (int)( 2000.f * log( invPercent ) ) + props->lRoom;

//  props->lRoomLF = (int)( ( (float)EAXLISTENER_MINROOMLF * invPercent ) + ( (float)props->lRoomLF * percent ) );
//  props->lRoomHF = (int)( ( (float)EAXLISTENER_MINROOMHF * invPercent ) + ( (float)props->lRoomHF * percent ) );
}

//// ClearProcessCache ///////////////////////////////////////////////////////
//  Clears the cache settings used to speed up ProcessMods(). Call whenever
//  the list of mods changed.

void    plEAXListener::ClearProcessCache()
{
    fLastBigRegion = nullptr;
    fLastModCount = -1;
    fLastWasEmpty = false;
    fLastSingleStrength = -1.f;
}

//// ProcessMods /////////////////////////////////////////////////////////////
//  9.13.02 mcn - Updated the caching method. Now fLastBigRegion will point
//  to a region iff it's the only region from the last pass that had a
//  strength > 0. The reason we can't do our trick before is because even if
//  we have a region w/ strength 1, another region might power up from 1 and
//  thus suddenly alter the total reverb settings. Thus, the only time we 
//  can wisely skip is if our current big region == fLastBigRegion *and*
//  the total strength is the same.

void    plEAXListener::ProcessMods(const std::set<plEAXListenerMod*>& modArray )
{
#ifdef EAX_SDK_AVAILABLE
    int     i;
    float   totalStrength;
    bool    firstOne;

    plEAXListenerMod        *thisBigRegion = nullptr;
    EAXLISTENERPROPERTIES   finalProps;
    static int oldTime = timeGetTime();     // Get starting time
    int newTime;
    bool bMorphing = false;

    static plStatusLog  *myLog = nullptr;

    if (myLog == nullptr && plgAudioSys::AreExtendedLogsEnabled())
        myLog = plStatusLogMgr::GetInstance().CreateStatusLog( 30, "EAX Reverbs", plStatusLog::kFilledBackground | plStatusLog::kDeleteForMe | plStatusLog::kDontWriteFile );
    else if (myLog != nullptr && !plgAudioSys::AreExtendedLogsEnabled())
    {
        delete myLog;
        myLog = nullptr;
    }

    if (myLog != nullptr)
        myLog->Clear();

    if( modArray.size() != fLastModCount )
    {
        DebugLog( "Clearing cache..." );
        ClearProcessCache();    // Code path changed, clear the entire cache
        fLastModCount = modArray.size();
    }
    else
    {
        DebugLog( "" );
    }

    if( modArray.size() > 0 )
    {
        DebugLog( "{} regions to calc", modArray.size() );

        // Reset and find a new one if applicable
        thisBigRegion = nullptr;

        // Accumulate settings from all the active listener regions (shouldn't be too many, we hope)
        totalStrength = 0.f;
        firstOne = true;
        for (auto mod : modArray)
        {
            float strength = mod->GetStrength();
            DebugLog( "{4.2f} - {}", strength, mod->GetKey()->GetUoid().GetObjectName() );
            if( strength > 0.f )
            {
                // fLastBigRegion will point to a region iff it's the only region w/ strength > 0
                if( totalStrength == 0.f )
                    thisBigRegion = mod;
                else
                    thisBigRegion = nullptr;

                if( firstOne )
                {
                    // First one, just init to it
                    memcpy( &finalProps, mod->GetListenerProps(), sizeof( finalProps ) );
                    totalStrength = strength;
                    firstOne = false;
                }
                else
                {
                    float scale = strength / ( totalStrength + strength );
                    EAX3ListenerInterpolate( &finalProps, mod->GetListenerProps(), scale, &finalProps, false );
                    totalStrength += strength;
                    bMorphing = true;
                }
                
                if( totalStrength >= 1.f )
                    break;
            }
        }

        if( firstOne )
        {
            // No regions of strength > 0, so just make it quiet
            DebugLog( "Reverb should be quiet" );
            if( fLastWasEmpty )
                return;

            memcpy( &finalProps, &EAX30_ORIGINAL_PRESETS[ ORIGINAL_GENERIC ], sizeof( EAXLISTENERPROPERTIES ) );
            finalProps.lRoom = EAXLISTENER_MINROOM;
//          finalProps.lRoomLF = EAXLISTENER_MINROOMLF;
//          finalProps.lRoomHF = EAXLISTENER_MINROOMHF;
            fLastWasEmpty = true;
            fLastBigRegion = nullptr;
            fLastSingleStrength = -1.f;
        }
        else 
        {
            fLastWasEmpty = false;

            if( thisBigRegion == fLastBigRegion && totalStrength == fLastSingleStrength )
                // Cached values should be the same, so we can bail at this point
                return;

            fLastBigRegion = thisBigRegion;
            fLastSingleStrength = (thisBigRegion != nullptr) ? totalStrength : -1.f;

            if( totalStrength < 1.f )
            {
                DebugLog( "Total strength < 1; muting result" );
                // All of them together is less than full strength, so mute our result
                IMuteProperties( &finalProps, totalStrength );
            }
        }
    }
    else
    {
        DebugLog( "No regions at all; disabling reverb" );
        // No regions whatsoever, so disable listener props entirely
        if( fLastWasEmpty )
            return;

        memcpy( &finalProps, &EAX30_ORIGINAL_PRESETS[ ORIGINAL_GENERIC ], sizeof( EAXLISTENERPROPERTIES ) );
        finalProps.lRoom = EAXLISTENER_MINROOM;
//      finalProps.lRoomLF = EAXLISTENER_MINROOMLF;
//      finalProps.lRoomHF = EAXLISTENER_MINROOMHF;
        fLastWasEmpty = true;
    }

    // if were morphing between regions, do 10th of a second check, otherwise just let it 
    // change due to opt out(caching) feature.
    if(bMorphing)
    {
        newTime = timeGetTime();

        // Update, at most, ten times per second
        if((newTime - oldTime) < 100) return;
            
        oldTime = newTime;      // update time
    }
//finalProps.flAirAbsorptionHF *= 0.3048f; // Convert to feet
    //DebugLog( "** Updating property set **" );


    if(!SetGlobalEAXProperty(DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_ALLPARAMETERS, &finalProps, sizeof( finalProps )))
    {
        IFail(  false );
    }
#endif /* EAX_SDK_AVAILABLE */
}


//////////////////////////////////////////////////////////////////////////////
//// Source Settings /////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//// Constructor/Destructor //////////////////////////////////////////////////

plEAXSourceSettings::plEAXSourceSettings()
{
    fDirtyParams = kAll;
    Enable( false );
}

plEAXSourceSettings::~plEAXSourceSettings()
{
}

//// Read/Write/Set //////////////////////////////////////////////////////////

void    plEAXSourceSettings::Read( hsStream *s )
{
    fEnabled = s->ReadBool();
    if( fEnabled )
    {
        fRoom = s->ReadLE16();
        fRoomHF = s->ReadLE16();
        fRoomAuto = s->ReadBool();
        fRoomHFAuto = s->ReadBool();

        fOutsideVolHF = s->ReadLE16();
        
        fAirAbsorptionFactor = s->ReadLEFloat();
        fRoomRolloffFactor = s->ReadLEFloat();
        fDopplerFactor = s->ReadLEFloat();
        fRolloffFactor = s->ReadLEFloat();

        fSoftStarts.Read( s );
        fSoftEnds.Read( s );

        fOcclusionSoftValue = -1.f;
        SetOcclusionSoftValue( s->ReadLEFloat() );

        fDirtyParams = kAll;
    }
    else
        Enable( false );    // Force init of params
}

void    plEAXSourceSettings::Write( hsStream *s )
{
    s->WriteBool( fEnabled );
    if( fEnabled )
    {
        s->WriteLE16( fRoom );
        s->WriteLE16( fRoomHF );
        s->WriteBool( fRoomAuto );
        s->WriteBool( fRoomHFAuto );

        s->WriteLE16( fOutsideVolHF );
        
        s->WriteLEFloat( fAirAbsorptionFactor );
        s->WriteLEFloat( fRoomRolloffFactor );
        s->WriteLEFloat( fDopplerFactor );
        s->WriteLEFloat( fRolloffFactor );

        fSoftStarts.Write( s );
        fSoftEnds.Write( s );

        s->WriteLEFloat( fOcclusionSoftValue );
    }
}

void    plEAXSourceSettings::SetRoomParams( int16_t room, int16_t roomHF, bool roomAuto, bool roomHFAuto )
{
    fRoom = room;
    fRoomHF = roomHF;
    fRoomAuto = roomAuto;
    fRoomHFAuto = roomHFAuto;
    fDirtyParams |= kRoom;
}

void    plEAXSourceSettings::Enable( bool e )
{
    fEnabled = e;
    if( !e )
    {
#ifdef EAX_SDK_AVAILABLE
        fRoom = EAXBUFFER_MINROOM;
        fRoomHF = EAXBUFFER_MINROOMHF;
#else
        fRoom = 0;
        fRoomHF = 0;
#endif
        fRoomAuto = true;
        fRoomHFAuto = true;

        fOutsideVolHF = 0;

        fAirAbsorptionFactor = 1.f;
        fRoomRolloffFactor = 0.f;
        fDopplerFactor = 0.f;
        fRolloffFactor = 0.f;

        fOcclusionSoftValue = 0.f;
        fSoftStarts.Reset();
        fSoftEnds.Reset();
        fCurrSoftValues.Reset();
        fDirtyParams = kAll;
    }
}

void    plEAXSourceSettings::SetOutsideVolHF( int16_t vol )
{
    fOutsideVolHF = vol;
    fDirtyParams |= kOutsideVolHF;
}

void    plEAXSourceSettings::SetFactors( float airAbsorption, float roomRolloff, float doppler, float rolloff )
{
    fAirAbsorptionFactor = airAbsorption;
    fRoomRolloffFactor = roomRolloff;
    fDopplerFactor = doppler;
    fRolloffFactor = rolloff;
    fDirtyParams |= kFactors;
}

void    plEAXSourceSettings::SetOcclusionSoftValue( float value )
{
    if( fOcclusionSoftValue != value )
    {
        fOcclusionSoftValue = value;
        IRecalcSofts( kOcclusion );
        fDirtyParams |= kOcclusion;
    }
}

void    plEAXSourceSettings::IRecalcSofts( uint8_t whichOnes )
{
    float    percent, invPercent;

    if( whichOnes & kOcclusion )
    {
        percent = fOcclusionSoftValue;
        invPercent = 1.f - percent;

        int16_t       occ = (int16_t)( ( (float)fSoftStarts.GetOcclusion() * invPercent ) + ( (float)fSoftEnds.GetOcclusion() * percent ) );
        float    lfRatio = (float)( ( fSoftStarts.GetOcclusionLFRatio() * invPercent ) + ( fSoftEnds.GetOcclusionLFRatio() * percent ) );
        float    roomRatio = (float)( ( fSoftStarts.GetOcclusionRoomRatio() * invPercent ) + ( fSoftEnds.GetOcclusionRoomRatio() * percent ) );
        float    directRatio = (float)( ( fSoftStarts.GetOcclusionDirectRatio() * invPercent ) + ( fSoftEnds.GetOcclusionDirectRatio() * percent ) );

        fCurrSoftValues.SetOcclusion( occ, lfRatio, roomRatio, directRatio );
    }
}


//////////////////////////////////////////////////////////////////////////////
//// Source Soft Settings ////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

void    plEAXSourceSoftSettings::Reset()
{
    fOcclusion = 0;
    fOcclusionLFRatio = 0.25f;
    fOcclusionRoomRatio = 1.5f;
    fOcclusionDirectRatio = 1.f;
}

void    plEAXSourceSoftSettings::Read( hsStream *s )
{
    s->ReadLE16(&fOcclusion);
    s->ReadLEFloat(&fOcclusionLFRatio);
    s->ReadLEFloat(&fOcclusionRoomRatio);
    s->ReadLEFloat(&fOcclusionDirectRatio);
}

void    plEAXSourceSoftSettings::Write( hsStream *s )
{
    s->WriteLE16(fOcclusion);
    s->WriteLEFloat(fOcclusionLFRatio);
    s->WriteLEFloat(fOcclusionRoomRatio);
    s->WriteLEFloat(fOcclusionDirectRatio);
}

void    plEAXSourceSoftSettings::SetOcclusion( int16_t occ, float lfRatio, float roomRatio, float directRatio )
{
    fOcclusion = occ;
    fOcclusionLFRatio = lfRatio;
    fOcclusionRoomRatio = roomRatio;
    fOcclusionDirectRatio = directRatio;
}

//// Constructor/Destructor //////////////////////////////////////////////////

plEAXSource::plEAXSource()
{   
    fInit = false;
    
}

plEAXSource::~plEAXSource()
{
    Release();
}

//// Init/Release ////////////////////////////////////////////////////////////

void    plEAXSource::Init( plDSoundBuffer *parent )
{
    fInit = true;
    // Init some default params
    plEAXSourceSettings defaultParams;
    SetFrom( &defaultParams, parent->GetSource() );
}

void    plEAXSource::Release()
{
    fInit = false;
}

bool    plEAXSource::IsValid() const
{
    return true;
}

//// SetFrom /////////////////////////////////////////////////////////////////

void    plEAXSource::SetFrom( plEAXSourceSettings *settings, unsigned source, bool force )
{
#if EAX_SDK_AVAILABLE
    uint32_t dirtyParams;
    if(source == 0 || !fInit) 
        return;

    if( force )
        dirtyParams = plEAXSourceSettings::kAll;
    else
        dirtyParams = settings->fDirtyParams;
    
    // Do the params
    if( dirtyParams & plEAXSourceSettings::kRoom )
    {
        SetSourceEAXProperty(source, DSPROPSETID_EAX_BufferProperties, DSPROPERTY_EAXBUFFER_ROOM, &settings->fRoom, sizeof(settings->fRoom));
        SetSourceEAXProperty(source, DSPROPSETID_EAX_BufferProperties, DSPROPERTY_EAXBUFFER_ROOMHF, &settings->fRoomHF, sizeof(settings->fRoomHF));
    }

    if( dirtyParams & plEAXSourceSettings::kOutsideVolHF )
    {
        SetSourceEAXProperty(source, DSPROPSETID_EAX_BufferProperties, DSPROPERTY_EAXBUFFER_OUTSIDEVOLUMEHF, &settings->fOutsideVolHF, sizeof(settings->fOutsideVolHF));
    }
    
    if( dirtyParams & plEAXSourceSettings::kFactors )
    {
        SetSourceEAXProperty(source, DSPROPSETID_EAX_BufferProperties, DSPROPERTY_EAXBUFFER_DOPPLERFACTOR, &settings->fDopplerFactor, sizeof(settings->fDopplerFactor));
        SetSourceEAXProperty(source, DSPROPSETID_EAX_BufferProperties, DSPROPERTY_EAXBUFFER_ROLLOFFFACTOR, &settings->fRolloffFactor, sizeof(settings->fRolloffFactor));
        SetSourceEAXProperty(source, DSPROPSETID_EAX_BufferProperties, DSPROPERTY_EAXBUFFER_ROOMROLLOFFFACTOR, &settings->fRoomRolloffFactor, sizeof(settings->fRoomRolloffFactor));
        SetSourceEAXProperty(source, DSPROPSETID_EAX_BufferProperties, DSPROPERTY_EAXBUFFER_AIRABSORPTIONFACTOR, &settings->fAirAbsorptionFactor, sizeof(settings->fAirAbsorptionFactor));
    }

    if( dirtyParams & plEAXSourceSettings::kOcclusion )
    {
        SetSourceEAXProperty(source, DSPROPSETID_EAX_BufferProperties, DSPROPERTY_EAXBUFFER_OCCLUSION, &settings->GetCurrSofts().fOcclusion, sizeof(settings->GetCurrSofts().fOcclusion));
        SetSourceEAXProperty(source, DSPROPSETID_EAX_BufferProperties, DSPROPERTY_EAXBUFFER_OCCLUSIONLFRATIO, &settings->GetCurrSofts().fOcclusionLFRatio, sizeof(settings->GetCurrSofts().fOcclusionLFRatio));
        SetSourceEAXProperty(source, DSPROPSETID_EAX_BufferProperties, DSPROPERTY_EAXBUFFER_OCCLUSIONROOMRATIO, &settings->GetCurrSofts().fOcclusionRoomRatio, sizeof(settings->GetCurrSofts().fOcclusionRoomRatio));
        SetSourceEAXProperty(source, DSPROPSETID_EAX_BufferProperties, DSPROPERTY_EAXBUFFER_OCCLUSIONDIRECTRATIO, &settings->GetCurrSofts().fOcclusionDirectRatio, sizeof(settings->GetCurrSofts().fOcclusionDirectRatio));
    }
#endif /* EAX_SDK_AVAILABLE */

    settings->ClearDirtyParams();

    // Do all the flags in one pass
#ifdef EAX_SDK_AVAILABLE
    DWORD   flags;
    

    if( GetSourceEAXProperty( source, DSPROPSETID_EAX_BufferProperties, DSPROPERTY_EAXBUFFER_FLAGS, &flags, sizeof( DWORD )) ) 
    {
        if( settings->GetRoomAuto() )
            flags |= EAXBUFFERFLAGS_ROOMAUTO;
        else
            flags &= ~EAXBUFFERFLAGS_ROOMAUTO;

        if( settings->GetRoomHFAuto() )
            flags |= EAXBUFFERFLAGS_ROOMHFAUTO;
        else
            flags &= ~EAXBUFFERFLAGS_ROOMHFAUTO;

        if( SetSourceEAXProperty( source, DSPROPSETID_EAX_BufferProperties, DSPROPERTY_EAXBUFFER_FLAGS, &flags, sizeof( DWORD ) ) ) 
        {
            return; // All worked, return here
        }   
        
        // Flag setting failed somehow
        hsAssert( false, "Unable to set EAX buffer flags" );
    }
#endif /* EAX_SDK_AVAILABLE */
}


