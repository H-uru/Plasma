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
#include "hsTimer.h"
#include "hsUtils.h"

#if HS_BUILD_FOR_MAC
#include <Timer.h>
#endif

#include "plTweak.h"

//
// plTimerShare - the actual worker. All process spaces should share a single
//		plTimerShare to keep time synchronized across spaces.
//
plTimerShare::plTimerShare()
:	fFirstTime(true),
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
	hsWide	ticks;
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
	retVal.fHi = Int32(hi);
	retVal.fLo = Int32(ticks);
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
		hsScalar delSys = hsScalar(t - fRealSeconds);
		fClamping = ( (fTimeClampSecs > 0) && (delSys > fTimeClampSecs) );
		if (fClamping)
		{
			delSys = fTimeClampSecs;
		}
		delSys *= fSysTimeScale;
		if( fDelSysSeconds > 0 && fDelSysSeconds < fSmoothingClampSecs )
		{
			const hsScalar kFrac = 0.1f;
			const hsScalar kOneMinusFrac = 1.f-kFrac;
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
				delSys = hsScalar(t - fRealSeconds);
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

		plCONST(hsScalar) kMaxSmoothable(0.15f);
		fDelSysSeconds = hsScalar(avg - fRealSeconds) * fSysTimeScale;
		if( fDelSysSeconds > kMaxSmoothable * fSysTimeScale )
		{
			avg = t;
			fDelSysSeconds = hsScalar(avg - fRealSeconds) * fSysTimeScale;
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

#include <windows.h>

hsWide* plTimerShare::GetRawTicks(hsWide* ticks) const
{
	LARGE_INTEGER	large;

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
	LARGE_INTEGER	large;
	if (::QueryPerformanceFrequency(&large))
		base.Set(large.HighPart, large.LowPart);
	else
		base.Set(0, 1000);

	return base;
}

#elif HS_BUILD_FOR_MAC

#include <Events.h>
#include <DriverServices.h>

//#define HS_USE_TICKCOUNT
hsWide* plTimerShare::GetRawTicks(hsWide* ticks)
{	
#ifndef HS_USE_TICKCOUNT
	UnsignedWide ns = AbsoluteToNanoseconds(UpTime());
	ticks->Set(ns.hi, ns.lo);
#else
	ticks->Set(0, TickCount());
#endif
	return FactorInTimeZero(ticks);
}

hsWide plTimerShare::IInitRawBase()
{
	hsWide base;
#ifndef HS_USE_TICKCOUNT
	base.Set(0, 1000000000L);
#else
	base.Set(0, 60);
#endif
	return base;
}

#elif HS_BUILD_FOR_UNIX

#include <sys/time.h>

#define kMicroSecondsUnit	1000000
static UInt32	gBaseTime = 0;

hsWide* plTimerShare::GetRawTicks(hsWide* ticks) const
{
	timeval	tv;

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


#elif HS_BUILD_FOR_PS2

extern unsigned long psTimerGetCount();
//#define kTickMul (150000000)		// kTickMul/kTickDiv :: 4577.636719
#define kTickMul (100000000)	// kTickMul/kTickDiv :: 3051.757813	// for debugger
#define kTickDiv (256*128)


hsWide* plTimerShare::GetRawTicks(hsWide* ticks)
{
	unsigned long t= psTimerGetCount();
	ticks->Set( (Int32)(t>>32), (Int32)(t&((1ul<<32)-1)));
	return ticks;
}

hsWide plTimerShare::IInitRawBase()
{
	hsWide base;
	base.Set(0, kTickMul/kTickDiv );
	return base;
}

#endif

// 
// hsTimer - thin static interface to plTimerShare. Also keeps a couple of
//		constants.
//
static plTimerShare			staticTimer;
plTimerShare*				hsTimer::fTimer = &staticTimer; // until overridden.
const double				hsTimer::fPrecTicksPerSec = hsTimer::GetPrecTicksPerSec();
const hsWide				hsTimer::fRawBase = hsTimer::IInitRawBase();

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
#if HS_BUILD_FOR_MAC
	return 1000.f;
#endif
#if HS_BUILD_FOR_PS2
	return 1000.f;
#endif
	
	return 1;
}

UInt32 hsTimer::GetPrecTickCount()
{
#if HS_BUILD_FOR_WIN32
	LARGE_INTEGER ti;
	if( !QueryPerformanceCounter(&ti) )
		return GetTickCount();

	return ti.LowPart;
#endif
#if HS_BUILD_FOR_MACPPC
	return hsTimer::GetMSeconds();
#endif

#if HS_BUILD_FOR_PS2
	return hsTimer::GetMSeconds();
#endif
}
UInt32 hsTimer::PrecSecsToTicks(hsScalar secs)
{
	return (UInt32)(((double)secs) * fPrecTicksPerSec);
}
double hsTimer::PrecTicksToSecs(UInt32 ticks)
{
	return ((double)ticks) / fPrecTicksPerSec;
}
double hsTimer::PrecTicksToHz(UInt32 ticks)
{
	return fPrecTicksPerSec / ((double)ticks);
}

UInt64 hsTimer::GetFullTickCount()
{
#if HS_BUILD_FOR_WIN32
	LARGE_INTEGER ticks;
	QueryPerformanceCounter(&ticks);
	return ticks.QuadPart;
#else
	return 0;
#endif
}

float hsTimer::FullTicksToMs(UInt64 ticks)
{
#ifdef HS_BUILD_FOR_WIN32
	static UInt64 ticksPerTenthMs = 0;

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
