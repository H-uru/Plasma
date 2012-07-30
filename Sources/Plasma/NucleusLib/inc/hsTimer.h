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
#ifndef hsTimer_Defined
#define hsTimer_Defined

#include "hsWide.h"

class plTimerShare
{
protected:
    mutable bool        fFirstTime;
    mutable hsWide      fRawTimeZero;
    mutable bool        fResetSmooth;

    enum {
        kSmoothBuffLen = 10
    };
    double              fSmoothBuff[kSmoothBuffLen];
    int                 fCurrSlot;

    float            fSysTimeScale;
    double           fRealSeconds;
    double           fSysSeconds;
    float            fDelSysSeconds;
    float            fFrameTimeInc;
    bool             fRunningFrameTime;
    float            fTimeClampSecs;
    float            fSmoothingClampSecs;
    bool             fClamping;

    hsWide*             FactorInTimeZero(hsWide* ticks) const;

    double              GetSeconds() const;
    double              GetMilliSeconds() const;

    hsWide*             GetRawTicks(hsWide* ticks) const;

    double              RawTicksToDSeconds(const hsWide& ticks);
    hsWide              DSecondsToRawTicks(double secs);

    float               GetDelSysSeconds() const { return fDelSysSeconds; }
    double              GetSysSeconds() const { return fSysSeconds; }
    double              IncSysSeconds();

    void                SetRealTime(bool realTime);
    bool                IsRealTime() const { return !fRunningFrameTime; }

    void                SetFrameTimeInc(float inc) { fFrameTimeInc = inc; }

    void                SetTimeScale(float s) { fSysTimeScale = s; }
    float               GetTimeScale() const { return fSysTimeScale; }

    void                SetTimeClamp(float secs) { fTimeClampSecs = secs; }
    void                SetSmoothingCap(float secs) { fSmoothingClampSecs = secs; }
    float               GetTimeClamp() const { return fTimeClampSecs; }
    bool                IsClamping() const { return fClamping; }

    friend class hsTimer;
public:
    plTimerShare();
    ~plTimerShare();
};

class hsTimer 
{
protected:
    static const double             fPrecTicksPerSec;
    static const hsWide             fRawBase;

    static  hsWide                  IInitRawBase();
    
    static plTimerShare*            fTimer;
public:

    static bool          VerifyRawBase() { return fRawBase == IInitRawBase(); }
    static const hsWide& GetRawBase() { return fRawBase; }

    static  hsWide*      GetRawTicks(hsWide* ticks) { return fTimer->GetRawTicks(ticks); }

    static  double      GetSeconds() { return fTimer->GetSeconds(); }
    static  double      GetMilliSeconds() { return fTimer->GetMilliSeconds(); }

    static double       RawTicksToDSeconds(const hsWide& ticks) { return fTimer->RawTicksToDSeconds(ticks); }
    static hsWide       DSecondsToRawTicks(double secs) { return fTimer->DSecondsToRawTicks(secs); }

    static float        GetDelSysSeconds() { return fTimer->GetDelSysSeconds(); }
    static double       GetSysSeconds() { return fTimer->GetSysSeconds(); }

    static double       IncSysSeconds() { return fTimer->IncSysSeconds(); }

    static void         SetRealTime(bool realTime) { fTimer->SetRealTime(realTime); }
    static bool         IsRealTime() { return fTimer->IsRealTime(); }

    static void         SetFrameTimeInc(float inc) { fTimer->SetFrameTimeInc(inc); }

    static void         SetTimeScale(float s) { fTimer->SetTimeScale(s); }
    static float        GetTimeScale() { return fTimer->GetTimeScale(); }

    static void         SetTimeClamp(float secs) { fTimer->SetTimeClamp(secs); }
    static void         SetTimeSmoothingClamp(float secs) { fTimer->SetSmoothingCap(secs); }
    static float        GetTimeClamp() { return fTimer->GetTimeClamp(); }
    static bool         IsClamping() { return fTimer->IsClamping(); }

    ///////////////////////////
    // Precision timer routines - these are stateless and implemented as statics.
    ///////////////////////////
    static uint32_t     GetPrecTickCount();
    static double       GetPrecTicksPerSec();
    static uint32_t     PrecSecsToTicks(float secs);
    static double       PrecTicksToSecs(uint32_t ticks);
    static double       PrecTicksToHz(uint32_t ticks);

    // If you need to time something longer than 20 seconds, use this instead of
    // the precision timer.  It works the same, it just gives you full resolution.
    static uint64_t   GetFullTickCount();
    static float      FullTicksToMs(uint64_t ticks);

    //
    // Pass GetTheTimer() into other process space, and then call SetTheTimer() on it.
    static void             SetTheTimer(plTimerShare* timer);
    static plTimerShare*    GetTheTimer() { return fTimer; }
};



#endif


