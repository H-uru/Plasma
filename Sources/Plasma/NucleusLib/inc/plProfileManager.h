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
#ifndef plProfileManager_h_inc
#define plProfileManager_h_inc

#include "HeadSpin.h"
#include <vector>

#include "plProfile.h"

class plProfileManager 
{
protected:
    friend class plProfileManagerFull;

    typedef std::vector<plProfileVar*> VarVec;
    VarVec fVars;

    double fLastAvgTime;

    uint32_t fProcessorSpeed;

    plProfileManager();

public:
    ~plProfileManager();

    static plProfileManager& Instance();

    void AddTimer(plProfileVar* var);   // Called by plProfileVar

    void BeginFrame();  // Call begin frame on all timers
    void EndFrame();    // Call end frame on all timers

    void SetAvgTime(uint32_t avgMS);

    uint32_t GetProcessorSpeed() { return fProcessorSpeed; }

    // Backdoor for hack timers in calculated profiles
    static uint64_t GetTime();
};

class plProfileLaps
{
protected:
    class LapInfo : public plProfileBase
    {
    protected:

    public:
        bool fUsedThisFrame;
        LapInfo(const char* name) : fUsedThisFrame() { fName = name; fDisplayFlags = kDisplayTime; }
        bool operator<(const LapInfo& rhs) const { return fLastAvg < rhs.fLastAvg; }

        void BeginTiming(uint64_t value) { fValue -= value; }
        void EndTiming(uint64_t value) { fValue += value; fTimerSamples++; }
    };
    std::vector<LapInfo> fLapTimes;

    LapInfo* IFindLap(const char* lapName);

public:
    void BeginLap(uint64_t curValue, const char* name);
    void EndLap(uint64_t curValue, const char* name);

    void BeginFrame();
    void EndFrame();
    void UpdateAvgs();

    int GetNumLaps();
    plProfileBase* GetLap(int i);
};

#endif // plProfileManager_h_inc
