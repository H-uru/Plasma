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

typedef std::chrono::duration<double> dSeconds;
typedef std::chrono::duration<double, std::milli> dMilliseconds;

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
    return dSeconds(GetRawTicks()).count();
}

double plTimerShare::GetMilliSeconds() const
{
    return dMilliseconds(GetRawTicks()).count();
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

void plTimerShare::SetRealTime(bool realTime)
{
    fRunningFrameTime = !realTime;
    if (realTime)
    {
        fRealSeconds = GetSeconds();
    }
}


plTimerShare::Duration plTimerShare::GetRawTicks() const
{
    plTimerShare::TimePoint tp = Clock::now();

    if (fFirstTime)
    {
        fFirstTime = false;
        fRawTimeZero = tp;
        return plTimerShare::Duration(0);
    }
    else
    {
        return plTimerShare::Duration(tp - fRawTimeZero);
    }
}


//
// hsTimer - thin static interface to plTimerShare. Also keeps a couple of
//      constants.
//
static plTimerShare staticTimer;
plTimerShare*       hsTimer::fTimer = &staticTimer; // until overridden.

void hsTimer::SetTheTimer(plTimerShare* timer)
{
    fTimer = timer;
}

double hsTimer::GetPrecTicksPerSec()
{
    return  double(plTimerShare::Clock::period::num) /
            double(plTimerShare::Clock::period::den);
}

uint32_t hsTimer::GetPrecTickCount()
{
    return uint32_t(plTimerShare::Clock::now().time_since_epoch().count());
}

double hsTimer::PrecTicksToSecs(uint32_t ticks)
{
    return dSeconds(plTimerShare::Duration(ticks)).count();
}

uint64_t hsTimer::GetFullTickCount()
{
    return uint64_t(plTimerShare::Clock::now().time_since_epoch().count());
}

float hsTimer::FullTicksToMs(uint64_t ticks)
{
    return float(dMilliseconds(plTimerShare::Duration(ticks)).count());
}
