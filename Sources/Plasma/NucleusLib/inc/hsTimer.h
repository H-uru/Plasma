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
#ifndef hsTimer_Defined
#define hsTimer_Defined

#include "hsWide.h"
#include "hsScalar.h"

#if !HS_CAN_USE_FLOAT
#error "Unsupported without double's"
#endif // !HS_CAN_USE_FLOAT

class plTimerShare
{
protected:
	mutable hsBool		fFirstTime;
	mutable hsWide		fRawTimeZero;
	mutable hsBool		fResetSmooth;

	enum {
		kSmoothBuffLen = 10
	};
	double				fSmoothBuff[kSmoothBuffLen];
	int					fCurrSlot;

	hsScalar			fSysTimeScale;
	double				fRealSeconds;
	double				fSysSeconds;
	hsScalar			fDelSysSeconds;
	hsScalar			fFrameTimeInc;
	hsBool				fRunningFrameTime;
	hsScalar			fTimeClampSecs;
	hsScalar			fSmoothingClampSecs;
	hsBool				fClamping;

	hsWide*				FactorInTimeZero(hsWide* ticks) const;

	double				GetSeconds() const;
	double				GetMilliSeconds() const;

	hsWide*				GetRawTicks(hsWide* ticks) const;

	double				RawTicksToDSeconds(const hsWide& ticks);
	hsWide				DSecondsToRawTicks(double secs);

	hsScalar			GetDelSysSeconds() const { return fDelSysSeconds; }
	double				GetSysSeconds() const { return fSysSeconds; }
	double				IncSysSeconds();

	void				SetRealTime(hsBool realTime);
	hsBool				IsRealTime() const { return !fRunningFrameTime; }

	void				SetFrameTimeInc(hsScalar inc) { fFrameTimeInc = inc; }

	void				SetTimeScale(hsScalar s) { fSysTimeScale = s; }
	hsScalar			GetTimeScale() const { return fSysTimeScale; }

	void				SetTimeClamp(hsScalar secs) { fTimeClampSecs = secs; }
	void				SetSmoothingCap(hsScalar secs) { fSmoothingClampSecs = secs; }
	hsScalar			GetTimeClamp() const { return fTimeClampSecs; }
	hsBool				IsClamping() const { return fClamping; }

	friend class hsTimer;
public:
	plTimerShare();
	~plTimerShare();
};

class hsTimer 
{
protected:
	static const double				fPrecTicksPerSec;
	static const hsWide				fRawBase;

	static	hsWide					IInitRawBase();
	
	static plTimerShare*			fTimer;
public:

	static hsBool	VerifyRawBase() { return fRawBase == IInitRawBase(); }
	static	const hsWide&		GetRawBase() { return fRawBase; }

	static	hsWide*				GetRawTicks(hsWide* ticks) { return fTimer->GetRawTicks(ticks); }

	static	double		GetSeconds() { return fTimer->GetSeconds(); }
	static	double		GetMilliSeconds() { return fTimer->GetMilliSeconds(); }

	static double		RawTicksToDSeconds(const hsWide& ticks) { return fTimer->RawTicksToDSeconds(ticks); }
	static hsWide		DSecondsToRawTicks(double secs) { return fTimer->DSecondsToRawTicks(secs); }

	static hsScalar		GetDelSysSeconds() { return fTimer->GetDelSysSeconds(); }
	static double		GetSysSeconds() { return fTimer->GetSysSeconds(); }

	static double		IncSysSeconds() { return fTimer->IncSysSeconds(); }

	static void			SetRealTime(hsBool realTime) { fTimer->SetRealTime(realTime); }
	static hsBool		IsRealTime() { return fTimer->IsRealTime(); }

	static void			SetFrameTimeInc(hsScalar inc) { fTimer->SetFrameTimeInc(inc); }

	static void			SetTimeScale(hsScalar s) { fTimer->SetTimeScale(s); }
	static hsScalar		GetTimeScale() { return fTimer->GetTimeScale(); }

	static void			SetTimeClamp(hsScalar secs) { fTimer->SetTimeClamp(secs); }
	static void			SetTimeSmoothingClamp(hsScalar secs) { fTimer->SetSmoothingCap(secs); }
	static hsScalar		GetTimeClamp() { return fTimer->GetTimeClamp(); }
	static hsBool		IsClamping() { return fTimer->IsClamping(); }

	///////////////////////////
	// Precision timer routines - these are stateless and implemented as statics.
	///////////////////////////
	static UInt32	GetPrecTickCount();
	static double	GetPrecTicksPerSec();
	static UInt32	PrecSecsToTicks(hsScalar secs);
	static double	PrecTicksToSecs(UInt32 ticks);
	static double	PrecTicksToHz(UInt32 ticks);

	// If you need to time something longer than 20 seconds, use this instead of
	// the precision timer.  It works the same, it just gives you full resolution.
	static UInt64	GetFullTickCount();
	static float	FullTicksToMs(UInt64 ticks);

	//
	// Pass GetTheTimer() into other process space, and then call SetTheTimer() on it.
	static void				SetTheTimer(plTimerShare* timer);
	static plTimerShare*	GetTheTimer() { return fTimer; }
};



#endif


