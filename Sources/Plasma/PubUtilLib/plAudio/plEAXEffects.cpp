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
//                  environmental audio.                                    //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////


#include "HeadSpin.h"
#include "hsWindows.h"
#include "hsThread.h"

#include "plEAXEffects.h"
#include "plAudioCore/plAudioCore.h"
#include "plDSoundBuffer.h"
#include "plEAXListenerMod.h"
#include "hsStream.h"
#include "plAudioSystem.h"

#include <chrono>
#include <efx-presets.h>

#include "plStatusLog/plStatusLog.h"

#define DebugLog   if (myLog) myLog->AddLineF


inline float lerp(float start, float finish, float ratio) {
    float result = 0.f;

    if (start == finish)
        result = start;
    else
        result = (start * (1.0f - ratio)) + (finish * ratio);

    return result;
}

bool ReverbPropsInterpolate(LPEFXEAXREVERBPROPERTIES lpStart, LPEFXEAXREVERBPROPERTIES lpFinish,
                            float flRatio, LPEFXEAXREVERBPROPERTIES lpResult)
{
    if (flRatio >= 1.0f) {
        lpResult = lpFinish;
        return true;
    } else if (flRatio <= 0.0f) {
        lpResult = lpStart;
        return true;
    }

    // Interpolate property values
    lpResult->flDensity = lerp(lpStart->flDensity, lpFinish->flDensity, flRatio);
    lpResult->flDiffusion = lerp(lpStart->flDiffusion, lpFinish->flDiffusion, flRatio);
    lpResult->flGain = lerp(lpStart->flGain, lpFinish->flGain, flRatio);
    lpResult->flGainHF = lerp(lpStart->flGainHF, lpFinish->flGainHF, flRatio);
    lpResult->flGainLF = lerp(lpStart->flGainLF, lpFinish->flGainLF, flRatio);
    lpResult->flDecayTime = (float)exp(lerp(log(lpStart->flDecayTime), log(lpFinish->flDecayTime), flRatio));
    lpResult->flDecayHFRatio = (float)exp(lerp(log(lpStart->flDecayHFRatio), log(lpFinish->flDecayHFRatio), flRatio));
    lpResult->flDecayLFRatio = (float)exp(lerp(log(lpStart->flDecayLFRatio), log(lpFinish->flDecayLFRatio), flRatio));
    lpResult->flReflectionsGain = lerp(lpStart->flReflectionsGain, lpFinish->flReflectionsGain, flRatio);
    lpResult->flReflectionsDelay = (float)exp(lerp(log(lpStart->flReflectionsDelay), log(lpFinish->flReflectionsDelay), flRatio));
    lpResult->flLateReverbGain = lerp(lpStart->flLateReverbGain, lpFinish->flLateReverbGain, flRatio);
    lpResult->flLateReverbDelay = (float)exp(lerp(log(lpStart->flLateReverbDelay), log(lpFinish->flLateReverbDelay), flRatio));
    lpResult->flEchoTime = (float)exp(lerp(log(lpStart->flEchoTime), log(lpFinish->flEchoTime), flRatio));
    lpResult->flEchoDepth = lerp(lpStart->flEchoDepth, lpFinish->flEchoDepth, flRatio);
    lpResult->flModulationTime = (float)exp(lerp(log(lpStart->flModulationTime), log(lpFinish->flModulationTime), flRatio));
    lpResult->flModulationDepth = lerp(lpStart->flModulationDepth, lpFinish->flModulationDepth, flRatio);
    lpResult->flAirAbsorptionGainHF = lerp(lpStart->flAirAbsorptionGainHF, lpFinish->flAirAbsorptionGainHF, flRatio);
    lpResult->flHFReference = (float)exp(lerp(log(lpStart->flHFReference), log(lpFinish->flHFReference), flRatio));
    lpResult->flLFReference = (float)exp(lerp(log(lpStart->flLFReference), log(lpFinish->flLFReference), flRatio));
    lpResult->flRoomRolloffFactor = lerp(lpStart->flRoomRolloffFactor, lpFinish->flRoomRolloffFactor, flRatio);

    flRatio < 0.5 ? lpResult->iDecayHFLimit = lpStart->iDecayHFLimit : lpFinish->iDecayHFLimit;

    // Clamp Delays
    if (lpResult->flReflectionsDelay > AL_EAXREVERB_MAX_REFLECTIONS_DELAY)
        lpResult->flReflectionsDelay = AL_EAXREVERB_MAX_REFLECTIONS_DELAY;

    if (lpResult->flLateReverbDelay > AL_EAXREVERB_MAX_LATE_REVERB_DELAY)
        lpResult->flLateReverbDelay = AL_EAXREVERB_MAX_LATE_REVERB_DELAY;

    return true;
}


ALboolean SetEFXEAXReverbProperties(EFXEAXREVERBPROPERTIES *pEFXEAXReverb, ALuint uiEffect)
{
    ALboolean bReturn = AL_FALSE;

    if (pEFXEAXReverb) {
        // Clear AL Error code
        alGetError();
        
        // Generate a new effect
        if (alIsEffect(uiEffect))
            alDeleteEffects(1, &uiEffect);
        alGenEffects(1, &uiEffect);
        alEffecti(uiEffect, AL_EFFECT_TYPE, AL_EFFECT_EAXREVERB);

        alEffectf(uiEffect, AL_EAXREVERB_DENSITY, pEFXEAXReverb->flDensity);
        alEffectf(uiEffect, AL_EAXREVERB_DIFFUSION, pEFXEAXReverb->flDiffusion);
        alEffectf(uiEffect, AL_EAXREVERB_GAIN, pEFXEAXReverb->flGain);
        alEffectf(uiEffect, AL_EAXREVERB_GAINHF, pEFXEAXReverb->flGainHF);
        alEffectf(uiEffect, AL_EAXREVERB_GAINLF, pEFXEAXReverb->flGainLF);
        alEffectf(uiEffect, AL_EAXREVERB_DECAY_TIME, pEFXEAXReverb->flDecayTime);
        alEffectf(uiEffect, AL_EAXREVERB_DECAY_HFRATIO, pEFXEAXReverb->flDecayHFRatio);
        alEffectf(uiEffect, AL_EAXREVERB_DECAY_LFRATIO, pEFXEAXReverb->flDecayLFRatio);
        alEffectf(uiEffect, AL_EAXREVERB_REFLECTIONS_GAIN, pEFXEAXReverb->flReflectionsGain);
        alEffectf(uiEffect, AL_EAXREVERB_REFLECTIONS_DELAY, pEFXEAXReverb->flReflectionsDelay);
        alEffectfv(uiEffect, AL_EAXREVERB_REFLECTIONS_PAN, pEFXEAXReverb->flReflectionsPan);
        alEffectf(uiEffect, AL_EAXREVERB_LATE_REVERB_GAIN, pEFXEAXReverb->flLateReverbGain);
        alEffectf(uiEffect, AL_EAXREVERB_LATE_REVERB_DELAY, pEFXEAXReverb->flLateReverbDelay);
        alEffectfv(uiEffect, AL_EAXREVERB_LATE_REVERB_PAN, pEFXEAXReverb->flLateReverbPan);
        alEffectf(uiEffect, AL_EAXREVERB_ECHO_TIME, pEFXEAXReverb->flEchoTime);
        alEffectf(uiEffect, AL_EAXREVERB_ECHO_DEPTH, pEFXEAXReverb->flEchoDepth);
        alEffectf(uiEffect, AL_EAXREVERB_MODULATION_TIME, pEFXEAXReverb->flModulationTime);
        alEffectf(uiEffect, AL_EAXREVERB_MODULATION_DEPTH, pEFXEAXReverb->flModulationDepth);
        alEffectf(uiEffect, AL_EAXREVERB_AIR_ABSORPTION_GAINHF, pEFXEAXReverb->flAirAbsorptionGainHF);
        alEffectf(uiEffect, AL_EAXREVERB_HFREFERENCE, pEFXEAXReverb->flHFReference);
        alEffectf(uiEffect, AL_EAXREVERB_LFREFERENCE, pEFXEAXReverb->flLFReference);
        alEffectf(uiEffect, AL_EAXREVERB_ROOM_ROLLOFF_FACTOR, pEFXEAXReverb->flRoomRolloffFactor);
        alEffecti(uiEffect, AL_EAXREVERB_DECAY_HFLIMIT, pEFXEAXReverb->iDecayHFLimit);

        // Copy the updated properties to the reverb effect slot
        alAuxiliaryEffectSloti(1, AL_EFFECTSLOT_EFFECT, uiEffect);

        if (alGetError() == AL_NO_ERROR)
            bReturn = AL_TRUE;
    }

    return bReturn;
}


//// GetInstance /////////////////////////////////////////////////////////////

plEAXListener &plEAXListener::GetInstance()
{
    static plEAXListener    instance;
    return instance;
}

//// Init ////////////////////////////////////////////////////////////////////

bool plEAXListener::Init()
{
    if (fInited)
        return true;

    alListenerf(AL_METERS_PER_UNIT, 0.3048f); // Distance units set to feet
    if (alGetError() != AL_NO_ERROR)
        IFail(false);

    alGenEffects(1, &fEffectID);
    alEffecti(fEffectID, AL_EFFECT_TYPE, AL_EFFECT_EAXREVERB);
    if (alGetError() != AL_NO_ERROR) {
        IFail("Could not create EFX Reverb Effect.", false);
        return false;
    } else {
        plStatusLog::AddLineSF("audio.log", "Reverb Listener Effect created: {}", fEffectID);
    }

    ALuint uiEffectSlot;
    alGenAuxiliaryEffectSlots(1, &uiEffectSlot);
    alAuxiliaryEffectSloti(1, AL_EFFECTSLOT_EFFECT, fEffectID);
    if (alGetError() != AL_NO_ERROR) {
        IFail("Could not attach EFX Reverb Effect.", false);
    }

    fInited = true;

    ClearProcessCache();

    return true;
}

//// Shutdown ////////////////////////////////////////////////////////////////

void plEAXListener::Shutdown()
{
    if (!fInited)
        return;

    IRelease();
}

//// IRelease ////////////////////////////////////////////////////////////////

void plEAXListener::IRelease()
{
    if (alIsEffect(fEffectID))
        alDeleteEffects(1, &fEffectID);
    ALuint slot = 1;
    alDeleteAuxiliaryEffectSlots(1, &slot);
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

void plEAXListener::IMuteProperties(EFXEAXREVERBPROPERTIES *props, float percent)
{
    // We only mute the gain, gainHF and gainLF, since those control the overall effect
    // application. All three are a direct linear blend, so this should be rather easy

    props->flGain *= percent;
    props->flGainLF *= percent;
    props->flGainHF *= percent;
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

    plEAXListenerMod* thisBigRegion = nullptr;
    EFXEAXREVERBPROPERTIES finalProps;
    static auto oldTime = std::chrono::steady_clock::now();     // Get starting time
    bool bMorphing = false;

    static plStatusLog  *myLog = nullptr;

    if (myLog == nullptr && plgAudioSys::AreExtendedLogsEnabled())
        myLog = plStatusLogMgr::GetInstance().CreateStatusLog( 30, "EFX Reverbs", plStatusLog::kFilledBackground | plStatusLog::kDeleteForMe | plStatusLog::kDontWriteFile );
    else if (myLog != nullptr && !plgAudioSys::AreExtendedLogsEnabled())
    {
        delete myLog;
        myLog = nullptr;
    }

    if (myLog != nullptr)
        myLog->Clear();

    if (modArray.size() != fLastModCount)
    {
        DebugLog("Clearing cache...");
        ClearProcessCache();    // Code path changed, clear the entire cache
        fLastModCount = modArray.size();
    }

    if( modArray.size() > 0 )
    {
        DebugLog("{} regions to calc:", modArray.size());

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
                    finalProps = *(mod->GetListenerProps());
                    totalStrength = strength;
                    firstOne = false;
                }
                else
                {
                    float scale = strength / (totalStrength + strength);
                    ReverbPropsInterpolate(&finalProps, mod->GetListenerProps(), scale, &finalProps);
                    totalStrength += strength;
                    bMorphing = true;
                }

                if (totalStrength >= 1.f)
                    break;
            }
        }

        if (firstOne)
        {
            // No regions of strength > 0, so just make it quiet
            DebugLog("Reverb should be quiet.");
            if (fLastWasEmpty)
                return;

            finalProps = EFX_REVERB_PRESET_GENERIC;
            finalProps.flGain = AL_EAXREVERB_MIN_GAIN;
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
                DebugLog( "Total strength < 1; muting result." );
                // All of them together is less than full strength, so mute our result
                IMuteProperties( &finalProps, totalStrength );
            }
        }
    }
    else
    {
        DebugLog( "No regions at all; disabling reverb." );
        // No regions whatsoever, so disable listener props entirely
        if( fLastWasEmpty )
            return;

        finalProps = EFX_REVERB_PRESET_GENERIC;
        finalProps.flGain = AL_EAXREVERB_MIN_GAIN;
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

    if (SetEFXEAXReverbProperties(&finalProps, fEffectID) != AL_TRUE) {
        IFail(false);
    }
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
    SetFrom(&defaultParams, parent->GetSource());
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

void plEAXSource::SetFrom(plEAXSourceSettings *settings, ALuint source, bool force)
{
    uint32_t dirtyParams;
    if(source == 0 || !fInit)
        return;

    if(force)
        dirtyParams = plEAXSourceSettings::kAll;
    else
        dirtyParams = settings->fDirtyParams;

    // Do the params
    if (dirtyParams & plEAXSourceSettings::kRoom) {
        alSourcef(source, AL_GAIN, powf(10.0f, (settings->fRoom) / 2000.0f));
    }

    if(dirtyParams & plEAXSourceSettings::kOutsideVolHF)
    {
        alSourcef(source, AL_CONE_OUTER_GAINHF, settings->fOutsideVolHF);  //  TODO: Convert from short value to float factor?
    }

    if (dirtyParams & plEAXSourceSettings::kFactors)
    {
        alSourcef(source, AL_DOPPLER_FACTOR, settings->fDopplerFactor);
        alSourcef(source, AL_ROOM_ROLLOFF_FACTOR, settings->fRoomRolloffFactor);
        alSourcef(source, AL_AIR_ABSORPTION_FACTOR, settings->fAirAbsorptionFactor);
    }

    if(dirtyParams & plEAXSourceSettings::kOcclusion)
    {
        // EFX doesn't support the high-level EAX Occlusion properties, so we'll have to calculate it ourselves.

        /*SetSourceEAXProperty(source, DSPROPSETID_EAX_BufferProperties, DSPROPERTY_EAXBUFFER_OCCLUSION, &settings->GetCurrSofts().fOcclusion, sizeof(settings->GetCurrSofts().fOcclusion));
        SetSourceEAXProperty(source, DSPROPSETID_EAX_BufferProperties, DSPROPERTY_EAXBUFFER_OCCLUSIONLFRATIO, &settings->GetCurrSofts().fOcclusionLFRatio, sizeof(settings->GetCurrSofts().fOcclusionLFRatio));
        SetSourceEAXProperty(source, DSPROPSETID_EAX_BufferProperties, DSPROPERTY_EAXBUFFER_OCCLUSIONROOMRATIO, &settings->GetCurrSofts().fOcclusionRoomRatio, sizeof(settings->GetCurrSofts().fOcclusionRoomRatio));
        SetSourceEAXProperty(source, DSPROPSETID_EAX_BufferProperties, DSPROPERTY_EAXBUFFER_OCCLUSIONDIRECTRATIO, &settings->GetCurrSofts().fOcclusionDirectRatio, sizeof(settings->GetCurrSofts().fOcclusionDirectRatio));
        */
    }

    settings->ClearDirtyParams();

    if (settings->GetRoomAuto())
        alSourcei(source, AL_AUXILIARY_SEND_FILTER_GAIN_AUTO, AL_TRUE);
    else
        alSourcei(source, AL_AUXILIARY_SEND_FILTER_GAIN_AUTO, AL_FALSE);

    if (settings->GetRoomHFAuto())
        alSourcei(source, AL_AUXILIARY_SEND_FILTER_GAINHF_AUTO, AL_TRUE);
    else
        alSourcei(source, AL_AUXILIARY_SEND_FILTER_GAINHF_AUTO, AL_FALSE);
}
