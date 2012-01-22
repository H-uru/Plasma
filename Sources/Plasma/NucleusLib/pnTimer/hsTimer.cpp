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
#include "hsTimer.h"
#include "HeadSpin.h"

#include "plTweak.h"

//
// plTimerShare - the actual worker. All process spaces should share a single
//      plTimerShare to keep time synchronized across spaces.
//
plTimerShare::plTimerShare()
:   fFirstTime(true),
    fSysSeconds(0),
    fRealSeconds(0),
    fDelSysSeconds(0),
    fFrameTimeInc(0.03f),
    fSysTimeScale(1.f),
    fTimeClampSecs(0.1f),
    fSmoothingClampSecs(-1.0f),
    fRunningFrameTime(false),
    fClamping(false),
    fResetSmooth(true),
    fCurrSlot(0)
{
}

plTimerShare::~plTimerShare()
{
}

double plTimerShare::GetSeconds() const
{
    hsWide  ticks;
    return hsTimer::GetRawTicks(&ticks)->AsDouble() / hsTimer::GetRawBase().AsDouble();
}

double plTimerShare::GetMilliSeconds() const
{
    return GetSeconds() * 1.e3;
}

hsWide plTimerShare::DSecondsToRawTicks(double secs)
{
    hsWide retVal;
    double ticks = secs * hsTimer::GetRawBase().AsDouble();
    double hi = ticks / double(65536) / double(65536);
    ticks -= hi;
    retVal.fHi = int32_t(hi);
    retVal.fLo = int32_t(ticks);
    return retVal;
}

double plTimerShare::RawTicksToDSeconds(const hsWide& ticks)
{
    return ticks.AsDouble() / hsTimer::GetRawBase().AsDouble();
}


inline hsWide* plTimerShare::FactorInTimeZero(hsWide* ticks) const
{
    if( fFirstTime )
    {   
        fFirstTime = false;
        fRawTimeZero = *ticks;
        ticks->Set(0, 0);
    }
    else
    {
        ticks->Sub(&fRawTimeZero);
    }
    return ticks;
}

double plTimerShare::IncSysSeconds()
{
    if( fRunningFrameTime )
    {
        fDelSysSeconds = fFrameTimeInc * fSysTimeScale;
        fSysSeconds += fDelSysSeconds;

        fResetSmooth = true;
    }
    else if( fSmoothingClampSecs >= 0 )
    {
        double t = GetSeconds();
        float delSys = float(t - fRealSeconds);
        fClamping = ( (fTimeClampSecs > 0) && (delSys > fTimeClampSecs) );
        if (fClamping)
        {
            delSys = fTimeClampSecs;
        }
        delSys *= fSysTimeScale;
        if( fDelSysSeconds > 0 && fDelSysSeconds < fSmoothingClampSecs )
        {
            const float kFrac = 0.1f;
            const float kOneMinusFrac = 1.f-kFrac;
            delSys *= kFrac;
            delSys += fDelSysSeconds * kOneMinusFrac;
        }
        if (delSys > 4.0f && delSys < 5.0f)
        {
            //got that mysterious bug, (Win2k? certain CPU's?) try again...
#if HS_BUILD_FOR_WIN32
            int count = 10;
            while( delSys >= fDelSysSeconds * 2 && count > 0 )
            {
                fRealSeconds = t;
                t = GetSeconds();
                delSys = float(t - fRealSeconds);
                count--;
            }
#endif
        }
        fDelSysSeconds = delSys;
        fSysSeconds += fDelSysSeconds;
        fRealSeconds = t;

        fResetSmooth = true;
    }
    else
    {
        double t = GetSeconds();
        plCONST(int) kSmoothBuffUsed(kSmoothBuffLen);

        if( fResetSmooth )
        {
            int i;
            for( i = 0; i < kSmoothBuffUsed; i++ )
                fSmoothBuff[i] = t;
            fResetSmooth = false;
        }

        if( ++fCurrSlot >= kSmoothBuffUsed )
            fCurrSlot = 0;
        fSmoothBuff[fCurrSlot] = t;

        double avg = 0;
        int j;
        for( j = 0; j < kSmoothBuffUsed; j++ )
        {
            avg += fSmoothBuff[j];
        }
        avg /= double(kSmoothBuffUsed);

        plCONST(float) kMaxSmoothable(0.15f);
        fDelSysSeconds = float(avg - fRealSeconds) * fSysTimeScale;
        if( fDelSysSeconds > kMaxSmoothable * fSysTimeScale )
        {
            avg = t;
            fDelSysSeconds = float(avg - fRealSeconds) * fSysTimeScale;
            fResetSmooth = true;
        }
        fSysSeconds += fDelSysSeconds;
        fRealSeconds = avg;
    }
    return fSysSeconds;
}

void plTimerShare::SetRealTime(hsBool realTime)
{ 
    fRunningFrameTime = !realTime; 
    if( realTime )
    {
        fRealSeconds = GetSeconds();
    }
}

#if HS_BUILD_FOR_WIN32

hsWide* plTimerShare::GetRawTicks(hsWide* ticks) const
{
    LARGE_INTEGER   large;

    if (::QueryPerformanceCounter(&large))
    {           
        ticks->Set(large.HighPart, large.LowPart);
    }
    else
    {
        ticks->Set(0, ::GetTickCount());
    }

    return FactorInTimeZero(ticks);
}

hsWide hsTimer::IInitRawBase()
{
    hsWide base;
    LARGE_INTEGER   large;
    if (::QueryPerformanceFrequency(&large))
        base.Set(large.HighPart, large.LowPart);
    else
        base.Set(0, 1000);

    return base;
}

#elif HS_BUILD_FOR_UNIX

#include <sys/time.h>

#define kMicroSecondsUnit   1000000
static uint32_t   gBaseTime = 0;

hsWide* plTimerShare::GetRawTicks(hsWide* ticks) const
{
    timeval tv;

    (void)::gettimeofday(&tv, nil);
    if (gBaseTime == 0)
        gBaseTime = tv.tv_sec;

    ticks->Mul(tv.tv_sec - gBaseTime, kMicroSecondsUnit)->Add(tv.tv_usec);
    return ticks;
}

hsWide hsTimer::IInitRawBase()
{
    hsWide base;
    base.Set(0, kMicroSecondsUnit);
    return base;
}

#endif

// 
// hsTimer - thin static interface to plTimerShare. Also keeps a couple of
//      constants.
//
static plTimerShare         staticTimer;
plTimerShare*               hsTimer::fTimer = &staticTimer; // until overridden.
const double                hsTimer::fPrecTicksPerSec = hsTimer::GetPrecTicksPerSec();
const hsWide                hsTimer::fRawBase = hsTimer::IInitRawBase();

void hsTimer::SetTheTimer(plTimerShare* timer)
{
    fTimer = timer;
}

///////////////////////////
// Precision timer routines
// These remain as statics 
// since they are stateless 
// anyway.
///////////////////////////

double hsTimer::GetPrecTicksPerSec()
{
#if HS_BUILD_FOR_WIN32
    LARGE_INTEGER freq;
    if( !QueryPerformanceFrequency(&freq) )
    {
        return 1000.f;
    }
    return ((double) freq.LowPart);
#endif
    
    return 1;
}

uint32_t hsTimer::GetPrecTickCount()
{
#if HS_BUILD_FOR_WIN32
    LARGE_INTEGER ti;
    if( !QueryPerformanceCounter(&ti) )
        return GetTickCount();

    return ti.LowPart;
#else
    return 1;
#endif
}
uint32_t hsTimer::PrecSecsToTicks(float secs)
{
    return (uint32_t)(((double)secs) * fPrecTicksPerSec);
}
double hsTimer::PrecTicksToSecs(uint32_t ticks)
{
    return ((double)ticks) / fPrecTicksPerSec;
}
double hsTimer::PrecTicksToHz(uint32_t ticks)
{
    return fPrecTicksPerSec / ((double)ticks);
}

uint64_t hsTimer::GetFullTickCount()
{
#if HS_BUILD_FOR_WIN32
    LARGE_INTEGER ticks;
    QueryPerformanceCounter(&ticks);
    return ticks.QuadPart;
#else
    return 0;
#endif
}

float hsTimer::FullTicksToMs(uint64_t ticks)
{
#ifdef HS_BUILD_FOR_WIN32
    static uint64_t ticksPerTenthMs = 0;

    if (ticksPerTenthMs == 0)
    {
        LARGE_INTEGER perfFreq;
        QueryPerformanceFrequency(&perfFreq);
        ticksPerTenthMs = perfFreq.QuadPart / 10000;
    }

    return float(ticks / ticksPerTenthMs) / 10.f;
#else
    return 0.f;
#endif
}
