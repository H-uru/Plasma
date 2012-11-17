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
#include "plProfileManager.h"
#include "plProfile.h"
#include "hsTimer.h"
#include "hsWindows.h"


static uint32_t gCyclesPerMS = 0;

#ifdef HS_BUILD_FOR_WIN32
#define USE_FAST_TIMER
#endif

#ifdef USE_FAST_TIMER

#pragma warning (push)
#pragma warning (disable : 4035)    // disable no return value warning

#ifdef _MSC_VER
#define forceinline __forceinline
#else
#define forceinline inline
#endif

forceinline uint32_t GetPentiumCounter()
{
#ifdef _MSC_VER
    __asm {
        xor   eax,eax       // VC won't realize that eax is modified w/out this
                            //   instruction to modify the val.
                            //   Problem shows up in release mode builds
        _emit 0x0F          // Pentium high-freq counter to edx;eax
        _emit 0x31          // only care about low 32 bits in eax
        
        xor   edx,edx       // so VC gets that edx is modified
    }
#endif
}

#pragma warning (pop)



static uint32_t GetProcSpeed()
{
    const char* keypath[] =
    {
        "HARDWARE",
        "DESCRIPTION",
        "System",
        "CentralProcessor",
        "0"
    };

    HKEY hKey = HKEY_LOCAL_MACHINE;

    int numKeys = sizeof(keypath) / sizeof(char*);
    for (int i = 0; i < numKeys; i++)
    {
        HKEY thisKey = NULL;    
        bool success = (RegOpenKeyEx(hKey, keypath[i], 0, KEY_READ, &thisKey) == ERROR_SUCCESS);

        RegCloseKey(hKey);
        hKey = thisKey;

        if (!success)
            return 0;
    }

    DWORD value=0, size=sizeof(DWORD);
    bool success = (RegQueryValueEx(hKey, "~MHz", 0, NULL, (BYTE*)&value, &size) == ERROR_SUCCESS);
    RegCloseKey(hKey);

    return value*1000000;
}

uint32_t GetProcSpeedAlt()
{
    const uint32_t kSamplePeriodMS = 250;

    // Raise priority to avoid interference from other threads.
    int priority = GetThreadPriority(GetCurrentThread());
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

    uint32_t startTicks, endTicks;
    uint64_t pcStart, pcEnd;

    // Count number of processor cycles inside the specified interval
    QueryPerformanceCounter((LARGE_INTEGER*)&pcStart);
    startTicks = plProfileManager::GetTime();
    Sleep(kSamplePeriodMS);
    endTicks = plProfileManager::GetTime();
    QueryPerformanceCounter((LARGE_INTEGER*)&pcEnd);

    // Restore thread priority.
    SetThreadPriority(GetCurrentThread(), priority);

    // Calculate Rdtsc/PerformanceCounter ratio;
    uint32_t numTicks = endTicks - startTicks;
    uint64_t pcDiff = pcEnd - pcStart;

    double ratio = double(numTicks) / double(pcDiff);
    uint64_t pcFreq;
    QueryPerformanceFrequency((LARGE_INTEGER*)&pcFreq);

    // Calculate CPU frequency.
    uint64_t cpuFreq = uint64_t(pcFreq * ratio);

    return (uint32_t)cpuFreq;
}

#define GetProfileTicks() GetPentiumCounter()

#else

#define GetProfileTicks() hsTimer::GetPrecTickCount()

#endif // USE_FAST_TIMER

#define TicksToMSec(t) (float(t) / float(gCyclesPerMS))
#define MSecToTicks(t) (float(t) * float(gCyclesPerMS))

plProfileManager::plProfileManager() : fLastAvgTime(0), fProcessorSpeed(0)
{
#ifdef USE_FAST_TIMER
    fProcessorSpeed = GetProcSpeed();
    // Registry stuff only works on NT OS's, have to calc it otherwise
    if (fProcessorSpeed == 0)
        fProcessorSpeed = GetProcSpeedAlt();

    gCyclesPerMS = fProcessorSpeed / 1000;
#else
    gCyclesPerMS = hsTimer::GetPrecTicksPerSec() / 1000;
#endif
}

plProfileManager::~plProfileManager()
{
}

plProfileManager& plProfileManager::Instance()
{
    static plProfileManager theInstance;
    return theInstance;
}

void plProfileManager::AddTimer(plProfileVar* var)
{
    fVars.push_back(var);
}

static uint32_t kAvgMilliseconds = 1000;

void plProfileManager::SetAvgTime(uint32_t avgMS)
{
    kAvgMilliseconds = avgMS;
}

static plProfileVar gVarEFPS("EFPS", "General", plProfileVar::kDisplayTime | plProfileVar::kDisplayFPS);

void plProfileManager::BeginFrame()
{
    for (int i = 0; i < fVars.size(); i++)
    {
        fVars[i]->BeginFrame();
        if (fVars[i]->GetLaps())
            fVars[i]->GetLaps()->BeginFrame();
    }

    gVarEFPS.BeginTiming();
}

void plProfileManager::EndFrame()
{
    gVarEFPS.EndTiming();

    bool updateAvgs = false;

    // If enough time has passed, update the averages
    double curTime = hsTimer::GetMilliSeconds();
    if (curTime - fLastAvgTime > kAvgMilliseconds)
    {
        fLastAvgTime = curTime;
        updateAvgs = true;
    }

    int i;

    //
    // Update all the variables
    //
    for (i = 0; i < fVars.size(); i++)
    {
        plProfileVar* var = fVars[i];

        if (updateAvgs)
        {
            // Timers that reset at every BeginTiming() call don't want to average over frames
            if (!hsCheckBits(var->GetDisplayFlags(), plProfileBase::kDisplayResetEveryBegin))
            {
                var->UpdateAvg();
                if (var->GetLaps())
                    var->GetLaps()->UpdateAvgs();
            }
        }

        var->EndFrame();

        if (var->GetLaps())
            var->GetLaps()->EndFrame();
    }
}

uint32_t plProfileManager::GetTime()
{
    return GetProfileTicks();
}

///////////////////////////////////////////////////////////////////////////////

plProfileBase::plProfileBase() :
    fName(nil),
    fDisplayFlags(0),
    fValue(0),
    fTimerSamples(0),
    fAvgCount(0),
    fAvgTotal(0),
    fLastAvg(0),
    fMax(0),
    fActive(false),
    fRunning(true)
{
}

plProfileBase::~plProfileBase()
{
}

void plProfileBase::BeginFrame()
{
    if (!hsCheckBits(fDisplayFlags, kDisplayNoReset))
        fValue = 0;
    fTimerSamples = 0;
}

void plProfileBase::EndFrame()
{
    fAvgCount++;
    fAvgTotal += fValue;
    fMax = hsMaximum(fMax, fValue);
}

void plProfileBase::UpdateAvg()
{
    if (fAvgCount > 0)
    {
        fLastAvg = (uint32_t)(fAvgTotal / fAvgCount);
        fAvgCount = 0;
        fAvgTotal = 0;
    }
}

uint32_t plProfileBase::GetValue()
{
    if (hsCheckBits(fDisplayFlags, kDisplayTime))
        return (uint32_t)TicksToMSec(fValue);
    else
        return fValue;
}

// Stolen from plMemTracker.cpp
static  const char  *insertCommas(unsigned int value)
{
    static char str[30];
    memset(str, 0, sizeof(str));

    sprintf(str, "%u", value);
    if (strlen(str) > 3)
    {
        memmove(&str[strlen(str)-3], &str[strlen(str)-4], 4);
        str[strlen(str) - 4] = ',';
    }
    if (strlen(str) > 7)
    {
        memmove(&str[strlen(str)-7], &str[strlen(str)-8], 8);
        str[strlen(str) - 8] = ',';
    }
    if (strlen(str) > 11)
    {
        memmove(&str[strlen(str)-11], &str[strlen(str)-12], 12);
        str[strlen(str) - 12] = ',';
    }

    return str;
}

void plProfileBase::IPrintValue(uint32_t value, char* buf, bool printType)
{
    if (hsCheckBits(fDisplayFlags, kDisplayCount))
    {
        if (printType)
        {
            const char* valueStr = insertCommas(value);
            strcpy(buf, valueStr);
        }
        else
            sprintf(buf, "%u", value);
    }
    else if (hsCheckBits(fDisplayFlags, kDisplayFPS))
    {
        sprintf(buf, "%.1f", 1000.0f / TicksToMSec(value));
    }
    else if (hsCheckBits(fDisplayFlags, kDisplayTime))
    {
        sprintf(buf, "%.1f", TicksToMSec(value));
        if (printType)
            strcat(buf, " ms");
    }
    else if (hsCheckBits(fDisplayFlags, kDisplayMem))
    {
        if (printType)
        {
            if (value > (1024*1000))
                sprintf(buf, "%.1f MB", float(value) / (1024.f * 1024.f));
            else if (value > 1024)
                sprintf(buf, "%d KB", value / 1024);
            else
                sprintf(buf, "%d b", value);
        }
        else
            sprintf(buf, "%u", value);
    }
}

void plProfileBase::PrintValue(char* buf, bool printType)
{
    IPrintValue(fValue, buf, printType);
}

void plProfileBase::PrintAvg(char* buf, bool printType)
{
    IPrintValue(fLastAvg, buf, printType);
}

void plProfileBase::PrintMax(char* buf, bool printType)
{
    IPrintValue(fMax, buf, printType);
}

////////////////////////////////////////////////////////////////////////////////


plProfileLaps::LapInfo* plProfileLaps::IFindLap(const char* lapName)
{
    static int lastSearch = 0;

    int i;
    for (i = lastSearch; i < fLapTimes.size(); i++)
    {
        if(fLapTimes[i].GetName() == lapName)
        {
            lastSearch = i;
            return &fLapTimes[i];
        }
    }

    if(lastSearch > fLapTimes.size()) lastSearch = fLapTimes.size();
    for (i = 0; i < lastSearch; i++)
    {
        if(fLapTimes[i].GetName() == lapName)
        {
            lastSearch = i;
            return &fLapTimes[i];
        }
    }
    return nil;
}

void plProfileLaps::BeginLap(uint32_t curValue, const char* name)
{
    LapInfo* lap = IFindLap(name);
    if (!lap)
    {
        // Technically we shouldn't hold on to this pointer.  However, I think
        // it will be ok in all cases, so I'll wait until this blows up
        LapInfo info(name);
        fLapTimes.push_back(info);
        lap = &(*(fLapTimes.end()-1));
    }

    lap->fUsedThisFrame = true;
    lap->BeginTiming(curValue);
}

void plProfileLaps::EndLap(uint32_t curValue, const char* name)
{
    LapInfo* lap = IFindLap(name);

    // There's a lap timer around the input code. You display it with "Stats.ShowLaps Update Input"
    // Since the command activates the timer INSIDE the lap, the first call to this function fails to
    // find it. (the timer wasn't active when BeginLap was called)
    if (lap)
        lap->EndTiming(curValue);
}

void plProfileLaps::BeginFrame()
{
    for (int i = 0; i < fLapTimes.size(); i++)
    {
        fLapTimes[i].BeginFrame();
        fLapTimes[i].fUsedThisFrame = false;
    }
}

void plProfileLaps::EndFrame()
{
    for (int i = 0; i < fLapTimes.size(); i++)
    {
        fLapTimes[i].EndFrame();
        if (!fLapTimes[i].fUsedThisFrame)
        {
            char buf[200];
            sprintf(buf, "Dropping unused lap %s", fLapTimes[i].GetName());
            hsStatusMessage(buf);
            fLapTimes.erase(fLapTimes.begin()+i);
            i--;
        }
    }
}

void plProfileLaps::UpdateAvgs()
{
    for (int i = 0; i < fLapTimes.size(); i++)
        fLapTimes[i].UpdateAvg();
}

int plProfileLaps::GetNumLaps()
{
//  std::sort(fLapTimes.begin(), fLapTimes.end());
    return fLapTimes.size();
}

plProfileBase* plProfileLaps::GetLap(int i)
{
    return &fLapTimes[i];
}


///////////////////////////////////////////////////////////////////////////////


plProfileVar::plProfileVar(const char *name, const char* group, uint8_t flags) :
    fGroup(group),
    fLaps(nil)
{
    fName = name;
    fDisplayFlags = flags;
    plProfileManager::Instance().AddTimer(this);
    fLapsActive = 0;
}

plProfileVar::~plProfileVar()
{
    delete fLaps;
}

void plProfileVar::IBeginLap(const char* lapName)
{
    if (!fLaps)
        fLaps = new plProfileLaps;
    fDisplayFlags |= kDisplayLaps;
    if(fLapsActive)
        fLaps->BeginLap(fValue, lapName);
    BeginTiming();
}

void plProfileVar::IEndLap(const char* lapName)
{
    EndTiming();
    if(fLapsActive)
        fLaps->EndLap(fValue, lapName);
}

void plProfileVar::IBeginTiming()
{
    if( hsCheckBits( fDisplayFlags, kDisplayResetEveryBegin ) )
        fValue = 0;

    fValue -= GetProfileTicks();
}

void plProfileVar::IEndTiming()
{
    fValue += GetProfileTicks();

    fTimerSamples++;

    // If we reset every BeginTiming(), then we want to average all the timing calls
    // independent of framerate
    if (hsCheckBits(fDisplayFlags, plProfileBase::kDisplayResetEveryBegin))
        UpdateAvg();
}
