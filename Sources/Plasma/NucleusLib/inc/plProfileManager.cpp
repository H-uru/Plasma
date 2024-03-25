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
#include <algorithm>

plProfileManager::plProfileManager() : fLastAvgTime(0), fProcessorSpeed(0)
{
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

uint64_t plProfileManager::GetTime()
{
    return hsTimer::GetTicks();
}

///////////////////////////////////////////////////////////////////////////////

plProfileBase::plProfileBase() :
    fName(),
    fDisplayFlags(),
    fValue(),
    fTimerSamples(),
    fAvgCount(),
    fAvgTotal(),
    fLastAvg(),
    fMax(),
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
    fMax = std::max(fMax, fValue);
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

uint64_t plProfileBase::GetValue()
{
    if (hsCheckBits(fDisplayFlags, kDisplayTime))
        return hsTimer::GetMilliSeconds<uint64_t>(fValue);
    else
        return fValue;
}

// Stolen from plMemTracker.cpp
static  const char  *insertCommas(unsigned long long value)
{
    static char str[30];
    memset(str, 0, sizeof(str));

    snprintf(str, std::size(str), "%llu", value);
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

void plProfileBase::IPrintValue(uint64_t value, char* buf, bool printType)
{
    if (hsCheckBits(fDisplayFlags, kDisplayCount))
    {
        if (printType)
        {
            const char* valueStr = insertCommas(value);
            strcpy(buf, valueStr);
        }
        else
            sprintf(buf, "%llu", static_cast<unsigned long long>(value));
    }
    else if (hsCheckBits(fDisplayFlags, kDisplayFPS))
    {
        sprintf(buf, "%.2f", 1000.0f / hsTimer::GetMilliSeconds<float>(value));
    }
    else if (hsCheckBits(fDisplayFlags, kDisplayTime))
    {
        sprintf(buf, "%.2f", hsTimer::GetMilliSeconds<float>(value));
        if (printType)
            strcat(buf, " ms");
    }
    else if (hsCheckBits(fDisplayFlags, kDisplayMem))
    {
        if (printType)
        {
            if (value > (1024*1000))
                sprintf(buf, "%.2f MB", float(value) / (1024.f * 1024.f));
            else if (value > 1024)
                sprintf(buf, "%llu KB", static_cast<unsigned long long>(value / 1024));
            else
                sprintf(buf, "%llu b", static_cast<unsigned long long>(value));
        }
        else
            sprintf(buf, "%llu", static_cast<unsigned long long>(value));
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
    return nullptr;
}

void plProfileLaps::BeginLap(uint64_t curValue, const char* name)
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

void plProfileLaps::EndLap(uint64_t curValue, const char* name)
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
    fLaps()
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

    fValue -= hsTimer::GetTicks();
}

void plProfileVar::IEndTiming()
{
    fValue += hsTimer::GetTicks();

    fTimerSamples++;

    // If we reset every BeginTiming(), then we want to average all the timing calls
    // independent of framerate
    if (hsCheckBits(fDisplayFlags, plProfileBase::kDisplayResetEveryBegin))
        UpdateAvg();
}
