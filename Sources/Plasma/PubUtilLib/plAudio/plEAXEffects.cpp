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
//  plEAXEffects - Various classes and wrappers to support EAX/EFX          //
//                  acceleration.                                           //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include "HeadSpin.h"
#include "hsWindows.h"
#include "hsThread.h"

#include "plEAXEffects.h"
#include "plEAXStructures.h"
#include "plAudioCore/plAudioCore.h"
#include "plDSoundBuffer.h"
#include "plEAXListenerMod.h"
#include "hsStream.h"
#include "plAudioSystem.h"

#include <chrono>

#include "plStatusLog/plStatusLog.h"

#define DebugLog   if (myLog) myLog->AddLineF

//// GetInstance /////////////////////////////////////////////////////////////

plEAXListener &plEAXListener::GetInstance()
{
    static plEAXListener    instance;
    return instance;
}

//// Init ////////////////////////////////////////////////////////////////////

bool plEAXListener::Init(ALCdevice *alDevice)
{
    if(fInited)
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
    } else {
        ALCint iVerMajor, iVerMinor;
        alcGetIntegerv(alDevice, ALC_EFX_MAJOR_VERSION, 1, &iVerMajor);
        alcGetIntegerv(alDevice, ALC_EFX_MAJOR_VERSION, 1, &iVerMinor);

        plStatusLog::AddLineSF("audio.log", "EFX v{}.{} available.", iVerMajor, iVerMinor);
    }

    // EAX is supported 
    fInited = true;

    try
    {
        // Make an EAX call here to prevent problems on WDM driver
        unsigned int lRoom = -10000;

        //SetGlobalEAXProperty(DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_ROOM, &lRoom, sizeof( unsigned int ));
    }
    catch (std::exception &e)
    {
        plStatusLog::AddLineSF("audio.log", "Unable to set EAX Property Set ({}), disabling EAX...", e.what());
        plgAudioSys::EnableEAX(false);
        return false;
    }
    catch (...)
    {
        plStatusLog::AddLineS("audio.log", "Unable to set EAX Property Set, disabling EAX...");
        plgAudioSys::EnableEAX(false);
        return false;
    }

    ClearProcessCache();

    return true;
}

//// Shutdown ////////////////////////////////////////////////////////////////

void plEAXListener::Shutdown()
{
    if(!fInited)
        return;

    IRelease();
}
/*
bool plEAXListener::SetGlobalEAXProperty(GUID guid, unsigned long ulProperty, void *pData, unsigned long ulDataSize )
{
    if(fInited)
    {
        return s_EAXSet(&guid, ulProperty, 0, pData, ulDataSize) == AL_NO_ERROR;
    }
    return false;
}

bool plEAXListener::GetGlobalEAXProperty(GUID guid, unsigned long ulProperty, void *pData, unsigned long ulDataSize)
{
    if(fInited)
    {
        return s_EAXGet(&guid, ulProperty, 0, pData, ulDataSize) == AL_NO_ERROR;
    }
    return false;
}

bool plEAXSource::SetSourceEAXProperty(unsigned source, GUID guid, unsigned long ulProperty, void *pData, unsigned long ulDataSize)
{
    return s_EAXSet(&guid, ulProperty, source, pData, ulDataSize) == AL_NO_ERROR;
    return false;
}

bool plEAXSource::GetSourceEAXProperty(unsigned source, GUID guid, unsigned long ulProperty, void *pData, unsigned long ulDataSize)
{
    return s_EAXGet(&guid, ulProperty, source, pData, ulDataSize) == AL_NO_ERROR;
    return false;
}*/


//// IRelease ////////////////////////////////////////////////////////////////

void plEAXListener::IRelease()
{
    fInited = false;
}

//// IFail ///////////////////////////////////////////////////////////////////

void plEAXListener::IFail(bool fatal)
{
    plStatusLog::AddLineS("audio.log", plStatusLog::kRed,
                          "ERROR in plEAXListener: Could not set global eax params");

    if(fatal)
        IRelease();
}

void plEAXListener::IFail(const char *msg, bool fatal)
{
    plStatusLog::AddLineSF("audio.log", plStatusLog::kRed,
                           "ERROR in plEAXListener: {}", msg);

    if(fatal)
        IRelease();
}

//// IMuteProperties /////////////////////////////////////////////////////////
//  Mutes the given properties, so if you have some props that you want
//  half strength, this function will do it for ya.

void    plEAXListener::IMuteProperties(EFXEAXREVERBPROPERTIES *props, float percent)
{
    // We only mute the room, roomHF and roomLF, since those control the overall effect
    // application. All three are a direct linear blend as defined by eax-util.cpp, so
    // this should be rather easy

    float invPercent = 1.f - percent;

    props->flGain = (int)(2000.f * log(invPercent)) + props->flGain;
    props->flGainLF = (int)(((float)AL_EAXREVERB_MIN_GAINLF * invPercent) + ((float)props->flGainLF * percent));
    props->flGainHF = (int)(((float)AL_EAXREVERB_MIN_GAINHF * invPercent) + ((float)props->flGainHF * percent));
}

//// ClearProcessCache ///////////////////////////////////////////////////////
//  Clears the cache settings used to speed up ProcessMods(). Call whenever
//  the list of mods changed.

void plEAXListener::ClearProcessCache()
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

void plEAXListener::ProcessMods(const std::set<plEAXListenerMod*>& modArray)
{
    float   totalStrength;
    bool    firstOne;

    plEAXListenerMod        *thisBigRegion = nullptr;
    EFXEAXREVERBPROPERTIES  finalProps;
    static auto oldTime = std::chrono::steady_clock::now();     // Get starting time
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

    if(modArray.size() != fLastModCount)
    {
        DebugLog("Clearing cache...");
        ClearProcessCache();    // Code path changed, clear the entire cache
        fLastModCount = modArray.size();
    }

    if( modArray.size() > 0 )
    {
        DebugLog("{} regions to calc", modArray.size());

        // Reset and find a new one if applicable
        thisBigRegion = nullptr;

        // Accumulate settings from all the active listener regions (shouldn't be too many, we hope)
        totalStrength = 0.f;
        firstOne = true;
        for (auto mod : modArray)
        {
            float strength = mod->GetStrength();
            DebugLog("{4.2f} - {}", strength, mod->GetKey()->GetUoid().GetObjectName());
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
                    memcpy(&finalProps, mod->GetListenerProps(), sizeof(finalProps));
                    totalStrength = strength;
                    firstOne = false;
                }
                else
                {
                    float scale = strength / ( totalStrength + strength );
                    //EAX3ListenerInterpolate( &finalProps, mod->GetListenerProps(), scale, &finalProps, false );
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

            //memcpy( &finalProps, &EAX30_ORIGINAL_PRESETS[ ORIGINAL_GENERIC ], sizeof( EAXLISTENERPROPERTIES ) );
            finalProps = EFX_REVERB_PRESET_GENERIC;
            finalProps.flGain = AL_EAXREVERB_MIN_GAIN;
            finalProps.flGainLF = AL_EAXREVERB_MIN_GAINLF;
            finalProps.flGainHF = AL_EAXREVERB_MIN_GAINHF;
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

        finalProps = EFX_REVERB_PRESET_GENERIC;
        finalProps.flGain = AL_EAXREVERB_MIN_GAIN;
        finalProps.flGainLF = AL_EAXREVERB_MIN_GAINLF;
        finalProps.flGainHF = AL_EAXREVERB_MIN_GAINHF;
        fLastWasEmpty = true;
    }

    // if were morphing between regions, do 10th of a second check, otherwise just let it
    // change due to opt out(caching) feature.
    if(bMorphing)
    {
        auto newTime = std::chrono::steady_clock::now();

        // Update, at most, ten times per second
        if((newTime - oldTime) < std::chrono::duration<int>(100))
            return;

        oldTime = newTime;      // update time
    }
    finalProps.flAirAbsorptionGainHF *= 0.3048f; // Convert to feet


    //if(!SetGlobalEAXProperty(DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_ALLPARAMETERS, &finalProps, sizeof( finalProps )))
    //{
        IFail(  false );
    //}
}


//////////////////////////////////////////////////////////////////////////////
//// Source Settings /////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//// Read/Write/Set //////////////////////////////////////////////////////////

void plEAXSourceSettings::Read( hsStream *s )
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
        fRoom = 0;
        fRoomHF = 0;
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

        int16_t  occ = (int16_t)( ( (float)fSoftStarts.GetOcclusion() * invPercent ) + ( (float)fSoftEnds.GetOcclusion() * percent ) );
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

void plEAXSource::SetFrom(plEAXSourceSettings *settings, unsigned source, bool force)
{
    uint32_t dirtyParams;
    if(source == 0 || !fInit)
        return;

    if(force)
        dirtyParams = plEAXSourceSettings::kAll;
    else
        dirtyParams = settings->fDirtyParams;

    // Do the params
    if (dirtyParams & plEAXSourceSettings::kRoom)
    {
        alSourcef(source, AL_EAXREVERB_GAIN, settings->fRoom);
        alSourcef(source, AL_EAXREVERB_GAINHF, settings->fRoomHF);
    }

    if(dirtyParams & plEAXSourceSettings::kOutsideVolHF)
    {
        alSourcef(source, AL_CONE_OUTER_GAINHF, settings->fOutsideVolHF);
    }

    if (dirtyParams & plEAXSourceSettings::kFactors)
    {
        alSourcef(source, AL_DOPPLER_FACTOR, settings->fDopplerFactor);
        alSourcef(source, AL_EAXREVERB_ROOM_ROLLOFF_FACTOR, settings->fRoomRolloffFactor);
        alSourcef(source, AL_EAXREVERB_AIR_ABSORPTION_GAINHF, settings->fAirAbsorptionFactor);
    }

    if(dirtyParams & plEAXSourceSettings::kOcclusion)
    {
        // EFX doesn't support the high-level EAX Occlusion properties, so we'll have to calculate it ourselves.
        //alSourcef(source, , settings->);
        //SetSourceEAXProperty(source, DSPROPSETID_EAX_BufferProperties, DSPROPERTY_EAXBUFFER_OCCLUSION, &settings->GetCurrSofts().fOcclusion, sizeof(settings->GetCurrSofts().fOcclusion));
        //SetSourceEAXProperty(source, DSPROPSETID_EAX_BufferProperties, DSPROPERTY_EAXBUFFER_OCCLUSIONLFRATIO, &settings->GetCurrSofts().fOcclusionLFRatio, sizeof(settings->GetCurrSofts().fOcclusionLFRatio));
        //SetSourceEAXProperty(source, DSPROPSETID_EAX_BufferProperties, DSPROPERTY_EAXBUFFER_OCCLUSIONROOMRATIO, &settings->GetCurrSofts().fOcclusionRoomRatio, sizeof(settings->GetCurrSofts().fOcclusionRoomRatio));
        //SetSourceEAXProperty(source, DSPROPSETID_EAX_BufferProperties, DSPROPERTY_EAXBUFFER_OCCLUSIONDIRECTRATIO, &settings->GetCurrSofts().fOcclusionDirectRatio, sizeof(settings->GetCurrSofts().fOcclusionDirectRatio));
    }

    settings->ClearDirtyParams();

    // Do all the flags in one pass
    DWORD   flags;

    /*
    if (GetSourceEAXProperty(source, DSPROPSETID_EAX_BufferProperties, DSPROPERTY_EAXBUFFER_FLAGS, &flags, sizeof(DWORD)))
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
        hsAssert( false, "Unable to set EFX buffer flags" );
    }
    */
}
