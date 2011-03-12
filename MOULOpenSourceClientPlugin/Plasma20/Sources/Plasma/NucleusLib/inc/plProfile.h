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
#ifndef plProfile_h_inc
#define plProfile_h_inc

#include "hsTypes.h"

#ifndef PLASMA_EXTERNAL_RELEASE
#define PL_PROFILE_ENABLED
#endif

//
// These macros are all you should need to use to profile your code.  They are
// compiled out in external release mode for maximum performance.
//
// plProfile_Create* should only be used at global scope, not in functions.
// Use plProfile_Extern if the plProfile_Create* for a variable is in another source file.
//
// --Examples--
//
// plProfile_CreateCounter("Num Foobars", "Test", NumFoobars);
// void SomeFunc1()
// {
//     plProfile_Inc(NumFoobars);
//     plProfile_IncCount(NumFoobars, 5);
// }
//
// plProfile_CreateTimer("Foobar Time", "Test", FoobarTime);
// void SomeFunc2()
// {
//     plProfile_BeginTiming(FoobarTime);
//     (execute some code...)
//     plProfile_EndTiming(FoobarTime);
//
//     plProfile_BeginLap(FoobarTime, pKeyedObj->GetKeyName());
//     pKeyedObj->DoStuff();
//     plProfile_EndLap(FoobarTime, pKeyedObj->GetKeyName());
// }
//

#ifdef PL_PROFILE_ENABLED

#define plProfile_CreateTimerNoReset(name, group, varName)	plProfileVar gProfileVar##varName(name, group, plProfileVar::kDisplayTime | plProfileVar::kDisplayNoReset)
#define plProfile_CreateTimer(name, group, varName)	plProfileVar gProfileVar##varName(name, group, plProfileVar::kDisplayTime)
#define plProfile_CreateAsynchTimer(name, group, varName)	plProfileVar gProfileVar##varName(name, group, plProfileVar::kDisplayTime | plProfileVar::kDisplayResetEveryBegin | plProfileVar::kDisplayNoReset)
#define plProfile_BeginTiming(varName)				gProfileVar##varName.BeginTiming()
#define plProfile_EndTiming(varName)				gProfileVar##varName.EndTiming()
#define plProfile_BeginLap(varName, lapName)		gProfileVar##varName.BeginLap(lapName)
#define plProfile_EndLap(varName, lapName)			gProfileVar##varName.EndLap(lapName)

#define plProfile_CreateCounter(name, group, varName)	plProfileVar gProfileVar##varName(name, group, plProfileVar::kDisplayCount)
#define plProfile_CreateCounterNoReset(name, group, varName)	plProfileVar gProfileVar##varName(name, group, plProfileVar::kDisplayCount | plProfileVar::kDisplayNoReset)
#define plProfile_Inc(varName)							gProfileVar##varName.Inc()
#define plProfile_IncCount(varName, count)				gProfileVar##varName.Inc(count)
#define plProfile_Dec(varName)							gProfileVar##varName.Dec()
#define plProfile_Set(varName, value)					gProfileVar##varName.Set(value)

#define plProfile_CreateMemCounter(name, group, varName)	plProfileVar gProfileVar##varName(name, group, plProfileVar::kDisplayMem | plProfileVar::kDisplayNoReset)
#define plProfile_CreateMemCounterReset(name, group, varName)	plProfileVar gProfileVar##varName(name, group, plProfileVar::kDisplayMem)
#define plProfile_NewMem(varName, memAmount)				gProfileVar##varName.NewMem(memAmount)
#define plProfile_DelMem(varName, memAmount)				gProfileVar##varName.DelMem(memAmount)

#define plProfile_StopVar(varName) gProfileVar##varName.Stop()
#define plProfile_StartVar(varName) gProfileVar##varName.Start()

#define plProfile_Extern(varName)					extern plProfileVar gProfileVar##varName

#else

#define plProfile_CreateTimerNoReset(name, group, varName)
#define plProfile_CreateTimer(name, group, varName)
#define plProfile_CreateAsynchTimer(name, group, varName)
#define plProfile_BeginTiming(varName)
#define plProfile_EndTiming(varName)
#define plProfile_BeginLap(varName, lapName)
#define plProfile_EndLap(varName, lapName)

#define plProfile_CreateCounter(name, group, varName)
#define plProfile_CreateCounterNoReset(name, group, varName)
#define plProfile_Inc(varName)
#define plProfile_IncCount(varName, count)
#define plProfile_Dec(varName)
#define plProfile_Set(varName, value)

#define plProfile_CreateMemCounter(name, group, varName)
#define plProfile_CreateMemCounterReset(name, group, varName)
#define plProfile_NewMem(varName, memAmount)
#define plProfile_DelMem(varName, memAmount)

#define plProfile_StopVar(varName)
#define plProfile_StartVar(varName)

#define plProfile_Extern(varName)

#endif

class plProfileLaps;

class plProfileBase
{
public:
	enum
	{
		kDisplayCount			= 0x1,
		kDisplayTime			= 0x2,
		kDisplayMem				= 0x4,
		kDisplayNoReset			= 0x8,
		kDisplayFPS				= 0x10,
		kDisplayLaps			= 0x20,
		kDisplaySelected		= 0x40,
		kDisplayResetEveryBegin	= 0x80
	};

protected:
	const char* fName;		// Name of timer

	UInt32 fValue;

	UInt32 fAvgCount;
	UInt64 fAvgTotal;
	UInt32 fLastAvg;
	UInt32 fMax;
	hsBool fActive;
	hsBool fRunning;
	UInt8 fDisplayFlags;

	// Number of times EndTiming was called. Can be used to combine timing and counting in one timer
	UInt32 fTimerSamples;

	void IAddAvg();

	void IPrintValue(UInt32 value, char* buf, hsBool printType);

public:
	plProfileBase();
	virtual ~plProfileBase();

	virtual void BeginFrame();
	virtual void EndFrame();

	void UpdateAvg();

	UInt32 GetValue();

	void PrintValue(char* buf, hsBool printType=true);
	void PrintAvg(char* buf, hsBool printType=true);
	void PrintMax(char* buf, hsBool printType=true);

	UInt32 GetTimerSamples() const { return fTimerSamples; }

	const char* GetName() { return fName; }

	void SetActive(hsBool s) { fActive = s; }

	void Stop() { fRunning = false; }
	void Start() { fRunning = true; }

	UInt8 GetDisplayFlags() const { return fDisplayFlags; }

	void ResetMax() { fMax = 0; }
};

class plProfileVar : public plProfileBase
{
protected:
	const char* fGroup;
	plProfileLaps* fLaps;
	hsBool fLapsActive;

	plProfileVar() {}

	void IBeginTiming();
	void IEndTiming();
	
	void IBeginLap(const char* lapName); 
	void IEndLap(const char* lapName);

public:
	// Name is the timer name. Each timer group gets its own plStatusLog
	plProfileVar(const char *name, const char* group, UInt8 flags);
	~plProfileVar();

	// For timing
	void BeginTiming() { if (fActive && fRunning) IBeginTiming(); }
	void EndTiming() { if (fActive && fRunning) IEndTiming(); }

	void NewMem(UInt32 memAmount) { fValue += memAmount; }
	void DelMem(UInt32 memAmount) { fValue -= memAmount; }

	// For Counting
	void Inc(int i = 1) { fValue += i;}
	void Dec(int i = 1) { fValue -= i;}

	void Set(UInt32 value) { fValue = value; }

	// 
	// For multiple timings per frame of the same thing ie. Each particle system
	//
	// Will output to log like
	// Timername : lapCnt: (lapName) : 3.22 msec
	//
	void BeginLap(const char* lapName) { if(fActive && fRunning) IBeginLap(lapName); }
	void EndLap(const char* lapName) { if(fActive && fRunning) IEndLap(lapName); }
	
	const char* GetGroup() { return fGroup; }

	plProfileLaps* GetLaps() { return fLaps; }

	// Enable Lap Sampling
	void SetLapsActive(hsBool s) { fLapsActive = s; }
};

#endif // plProfile_h_inc