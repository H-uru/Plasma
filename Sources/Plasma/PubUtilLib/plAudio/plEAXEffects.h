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

#ifndef _plEAXEffects_h
#define _plEAXEffects_h

#include <set>
#include <efx.h>
#include <efx-presets.h>

#include "HeadSpin.h"


//// Listener Settings Class Definition ///////////////////////////////////////

class plDSoundBuffer;
class plEAXListenerMod;

class plEAXListener 
{   
public:
    ~plEAXListener() { Shutdown();  }
    static plEAXListener  &GetInstance();

    bool    Init();
    void    Shutdown();

    void    ProcessMods(const std::set<plEAXListenerMod*>& modArray);
    void    ClearProcessCache();

protected:
    plEAXListener() : fInited(), fEffectID() { ClearProcessCache(); }
    void    IFail(bool fatal);
    void    IFail(const char *msg, bool fatal);
    void    IRelease();

    void IMuteProperties(EFXEAXREVERBPROPERTIES *props, float percent);

    bool fInited;
    
    // Cache info
    size_t             fLastModCount;
    bool               fLastWasEmpty;
    float              fLastSingleStrength;
    plEAXListenerMod  *fLastBigRegion;
    ALuint             fEffectID;

};

//// Soft Buffer Settings Class Definition ////////////////////////////////////
//  Used to hold buffer settings that will be attenuated by a soft volume,
//  to make the main settings class a bit cleaner

class hsStream;
class plEAXSourceSoftSettings
{
public:
        int16_t  fOcclusion;
        float    fOcclusionLFRatio, fOcclusionRoomRatio, fOcclusionDirectRatio;

        void     Read( hsStream *s );
        void     Write( hsStream *s );

        void     SetOcclusion( int16_t occ, float lfRatio, float roomRatio, float directRatio );
        int16_t  GetOcclusion() const { return fOcclusion; }
        float    GetOcclusionLFRatio() const { return fOcclusionLFRatio; }
        float    GetOcclusionRoomRatio() const { return fOcclusionRoomRatio; }
        float    GetOcclusionDirectRatio() const { return fOcclusionDirectRatio; }

        void     Reset();
};

//// Buffer Settings Class Definition /////////////////////////////////////////

class plEAXSource;

class plEAXSourceSettings
{
    public:
        plEAXSourceSettings() : fDirtyParams(kAll) { Enable(false); }
        virtual ~plEAXSourceSettings() { }

        void    Read( hsStream *s );
        void    Write( hsStream *s );

        void    Enable( bool e );
        bool    IsEnabled() const { return fEnabled; }

        void    SetRoomParams( int16_t room, int16_t roomHF, bool roomAuto, bool roomHFAuto );
        int16_t GetRoom() const   { return fRoom; }
        int16_t GetRoomHF()  const  { return fRoomHF; }
        bool    GetRoomAuto() const   { return fRoomAuto; }
        bool    GetRoomHFAuto() const  { return fRoomHFAuto; }

        void    SetOutsideVolHF( int16_t vol );
        int16_t GetOutsideVolHF() const { return fOutsideVolHF; }

        void    SetFactors( float airAbsorption, float roomRolloff, float doppler, float rolloff );
        float   GetAirAbsorptionFactor() const { return fAirAbsorptionFactor; }
        float   GetRoomRolloffFactor() const { return fRoomRolloffFactor; }
        float   GetDopplerFactor() const { return fDopplerFactor; }
        float   GetRolloffFactor() const { return fRolloffFactor; }

        plEAXSourceSoftSettings &GetSoftStarts() { return fSoftStarts; }
        plEAXSourceSoftSettings &GetSoftEnds() { return fSoftEnds; }
        
        plEAXSourceSoftSettings &GetCurrSofts()  { return fCurrSoftValues; }

        void  SetOcclusionSoftValue( float value );
        float GetOcclusionSoftValue() const { return fOcclusionSoftValue; }

        void  ClearDirtyParams() const { fDirtyParams = 0; }

    protected:
        friend class plEAXSource;
        friend class plEAXSourceSoftSettings;

        bool    fEnabled;
        int16_t fRoom, fRoomHF;
        bool    fRoomAuto, fRoomHFAuto;
        int16_t fOutsideVolHF;
        float   fAirAbsorptionFactor, fRoomRolloffFactor, fDopplerFactor, fRolloffFactor;
        float   fOcclusionSoftValue;
        mutable uint32_t fDirtyParams;
        plEAXSourceSoftSettings fSoftStarts, fSoftEnds, fCurrSoftValues;

        enum ParamSets
        {
            kOcclusion      = 0x01,
            kRoom           = 0x02,
            kOutsideVolHF   = 0x04,
            kFactors        = 0x08,
            kAll            = 0xff
        };

        void    IRecalcSofts( uint8_t whichOnes );
};

//// Source Class Definition //////////////////////////////////////////////////

class plEAXSource
{
public:
    friend class plEAXSourceSettings;
    friend class plEAXSourceSoftSettings;

    plEAXSource() : fInit(false) {}
    virtual ~plEAXSource() { Release(); };

    void Init( plDSoundBuffer *parent );
    void Release();
    bool IsValid() const;
    void SetFrom(plEAXSourceSettings *settings, ALuint source, bool force = false);

private:
    bool fInit;
};

#endif //_plEAXEffects_h
