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
#ifndef plProfile_h_inc
#define plProfile_h_inc

#include "HeadSpin.h"

#include <string_theory/string>

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

#define plProfile_CreateTimerNoReset(name, group, varName)  plProfileVar gProfileVar##varName(ST_LITERAL(name), ST_LITERAL(group), plProfileVar::kDisplayTime | plProfileVar::kDisplayNoReset)
#define plProfile_CreateTimer(name, group, varName) plProfileVar gProfileVar##varName(ST_LITERAL(name), ST_LITERAL(group), plProfileVar::kDisplayTime)
#define plProfile_CreateAsynchTimer(name, group, varName)   plProfileVar gProfileVar##varName(ST_LITERAL(name), ST_LITERAL(group), plProfileVar::kDisplayTime | plProfileVar::kDisplayResetEveryBegin | plProfileVar::kDisplayNoReset)
#define plProfile_BeginTiming(varName)              gProfileVar##varName.BeginTiming()
#define plProfile_EndTiming(varName)                gProfileVar##varName.EndTiming()
#define plProfile_BeginLap(varName, lapName)        gProfileVar##varName.BeginLap(lapName)
#define plProfile_EndLap(varName, lapName)          gProfileVar##varName.EndLap(lapName)

#define plProfile_CreateCounter(name, group, varName)   plProfileVar gProfileVar##varName(ST_LITERAL(name), ST_LITERAL(group), plProfileVar::kDisplayCount)
#define plProfile_CreateCounterNoReset(name, group, varName)    plProfileVar gProfileVar##varName(ST_LITERAL(name), ST_LITERAL(group), plProfileVar::kDisplayCount | plProfileVar::kDisplayNoReset)
#define plProfile_Inc(varName)                          gProfileVar##varName.Inc()
#define plProfile_IncCount(varName, count)              gProfileVar##varName.Inc(count)
#define plProfile_Dec(varName)                          gProfileVar##varName.Dec()
#define plProfile_Set(varName, value)                   gProfileVar##varName.Set(value)

#define plProfile_CreateMemCounter(name, group, varName)    plProfileVar gProfileVar##varName(ST_LITERAL(name), ST_LITERAL(group), plProfileVar::kDisplayMem | plProfileVar::kDisplayNoReset)
#define plProfile_CreateMemCounterReset(name, group, varName)   plProfileVar gProfileVar##varName(ST_LITERAL(name), ST_LITERAL(group), plProfileVar::kDisplayMem)
#define plProfile_NewMem(varName, memAmount)                gProfileVar##varName.NewMem(memAmount)
#define plProfile_DelMem(varName, memAmount)                gProfileVar##varName.DelMem(memAmount)

#define plProfile_StopVar(varName) gProfileVar##varName.Stop()
#define plProfile_StartVar(varName) gProfileVar##varName.Start()

#define plProfile_Extern(varName)                   extern plProfileVar gProfileVar##varName

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
        kDisplayCount           = 0x1,
        kDisplayTime            = 0x2,
        kDisplayMem             = 0x4,
        kDisplayNoReset         = 0x8,
        kDisplayFPS             = 0x10,
        kDisplayLaps            = 0x20,
        kDisplaySelected        = 0x40,
        kDisplayResetEveryBegin = 0x80
    };

protected:
    ST::string fName; // Name of timer

    uint64_t fValue;

    uint32_t fAvgCount;
    uint64_t fAvgTotal;
    uint32_t fLastAvg;
    uint64_t fMax;
    bool     fActive;
    bool     fRunning;
    uint8_t fDisplayFlags;

    // Number of times EndTiming was called. Can be used to combine timing and counting in one timer
    uint32_t fTimerSamples;

    void IAddAvg();

    ST::string IPrintValue(uint64_t value, bool printType);

public:
    plProfileBase();
    virtual ~plProfileBase();

    virtual void BeginFrame();
    virtual void EndFrame();

    void UpdateAvg();

    uint64_t GetValue();

    ST::string PrintValue(bool printType = true);
    ST::string PrintAvg(bool printType = true);
    ST::string PrintMax(bool printType = true);

    uint32_t GetTimerSamples() const { return fTimerSamples; }

    ST::string GetName() const { return fName; }

    void SetActive(bool s) { fActive = s; }

    void Stop() { fRunning = false; }
    void Start() { fRunning = true; }

    uint8_t GetDisplayFlags() const { return fDisplayFlags; }

    void ResetMax() { fMax = 0; }
};

class plProfileVar : public plProfileBase
{
protected:
    ST::string fGroup;
    plProfileLaps* fLaps;
    bool fLapsActive;

    plProfileVar() {}

    void IBeginTiming();
    void IEndTiming();

    void IBeginLap(const ST::string& lapName);
    void IEndLap(const ST::string& lapName);

public:
    // Name is the timer name. Each timer group gets its own plStatusLog
    plProfileVar(ST::string name, ST::string group, uint8_t flags);
    ~plProfileVar();

    // For timing
    void BeginTiming() { if (fActive && fRunning) IBeginTiming(); }
    void EndTiming() { if (fActive && fRunning) IEndTiming(); }

    void NewMem(uint32_t memAmount) { fValue += memAmount; }
    void DelMem(uint32_t memAmount) { fValue -= memAmount; }

    // For Counting
    void Inc(int i = 1) { fValue += i;}
    void Dec(int i = 1) { fValue -= i;}

    void Set(uint64_t value) { fValue = value; }

    // 
    // For multiple timings per frame of the same thing ie. Each particle system
    //
    // Will output to log like
    // Timername : lapCnt: (lapName) : 3.22 msec
    //
    void BeginLap(const ST::string& lapName) { if (fActive && fRunning) IBeginLap(lapName); }
    void EndLap(const ST::string& lapName) { if (fActive && fRunning) IEndLap(lapName); }

    ST::string GetGroup() const { return fGroup; }

    plProfileLaps* GetLaps() { return fLaps; }

    // Enable Lap Sampling
    void SetLapsActive(bool s) { fLapsActive = s; }
};

#endif // plProfile_h_inc
