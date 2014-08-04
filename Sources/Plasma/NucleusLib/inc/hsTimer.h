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

#include <chrono>

class plTimerShare
{
private:
    typedef std::chrono::high_resolution_clock  Clock;
    typedef std::chrono::time_point<Clock>      TimePoint;
    typedef Clock::duration                     Duration;

protected:
    mutable bool        fFirstTime;
    mutable TimePoint   fRawTimeZero;
    mutable bool        fResetSmooth;

    enum {
        kSmoothBuffLen = 10
    };
    double      fSmoothBuff[kSmoothBuffLen];
    int         fCurrSlot;

    float       fSysTimeScale;
    double      fRealSeconds;
    double      fSysSeconds;
    float       fDelSysSeconds;
    float       fFrameTimeInc;
    bool        fRunningFrameTime;
    float       fTimeClampSecs;
    float       fSmoothingClampSecs;
    bool        fClamping;


    template<typename T = double>
    T GetSeconds() const
    {
        typedef std::chrono::duration<T> duration_type;
        Duration d = GetRawTicks();
        return std::chrono::duration_cast<duration_type>(d).count();
    }

    template<typename T = double>
    T GetSeconds(uint64_t ticks) const
    {
        typedef std::chrono::duration<T> duration_type;
        Duration d(ticks);
        return std::chrono::duration_cast<duration_type>(d).count();
    }

    template<typename T = double>
    T GetMilliSeconds() const
    {
        typedef std::chrono::duration<T, std::milli> duration_type;
        Duration d = GetRawTicks();
        return std::chrono::duration_cast<duration_type>(d).count();
    }

    template<typename T = double>
    T GetMilliSeconds(uint64_t ticks) const
    {
        typedef std::chrono::duration<T, std::milli> duration_type;
        Duration d(ticks);
        return std::chrono::duration_cast<duration_type>(d).count();
    }

    uint64_t    GetTicks() const;

    float       GetDelSysSeconds() const { return fDelSysSeconds; }
    double      GetSysSeconds() const { return fSysSeconds; }
    double      IncSysSeconds();

    void        SetRealTime(bool realTime);
    bool        IsRealTime() const { return !fRunningFrameTime; }

    void        SetFrameTimeInc(float inc) { fFrameTimeInc = inc; }

    void        SetTimeScale(float s) { fSysTimeScale = s; }
    float       GetTimeScale() const { return fSysTimeScale; }

    void        SetTimeClamp(float secs) { fTimeClampSecs = secs; }
    void        SetSmoothingCap(float secs) { fSmoothingClampSecs = secs; }
    float       GetTimeClamp() const { return fTimeClampSecs; }
    bool        IsClamping() const { return fClamping; }

    friend class hsTimer;


private:
    Duration    GetRawTicks() const;


public:
    plTimerShare();
    ~plTimerShare();
};


class hsTimer
{
protected:
    static plTimerShare*            fTimer;


public:
    template<typename T = double>
    static T GetSeconds() { return fTimer->GetSeconds<T>(); }

    template<typename T = double>
    static T GetSeconds(uint64_t ticks) { return fTimer->GetSeconds<T>(ticks); }

    template<typename T = double>
    static T GetMilliSeconds() { return fTimer->GetMilliSeconds<T>(); }

    template<typename T = double>
    static T GetMilliSeconds(uint64_t ticks) { return fTimer->GetMilliSeconds<T>(ticks); }

    static uint64_t GetTicks() { return fTimer->GetTicks(); }

    static float    GetDelSysSeconds() { return fTimer->GetDelSysSeconds(); }
    static double   GetSysSeconds() { return fTimer->GetSysSeconds(); }

    static double   IncSysSeconds() { return fTimer->IncSysSeconds(); }

    static void     SetRealTime(bool realTime) { fTimer->SetRealTime(realTime); }
    static bool     IsRealTime() { return fTimer->IsRealTime(); }

    static void     SetFrameTimeInc(float inc) { fTimer->SetFrameTimeInc(inc); }

    static void     SetTimeScale(float s) { fTimer->SetTimeScale(s); }
    static float    GetTimeScale() { return fTimer->GetTimeScale(); }

    static void     SetTimeClamp(float secs) { fTimer->SetTimeClamp(secs); }
    static void     SetTimeSmoothingClamp(float secs) { fTimer->SetSmoothingCap(secs); }
    static float    GetTimeClamp() { return fTimer->GetTimeClamp(); }
    static bool     IsClamping() { return fTimer->IsClamping(); }

    //
    // Pass GetTheTimer() into other process space, and then call SetTheTimer() on it.
    static void             SetTheTimer(plTimerShare* timer);
    static plTimerShare*    GetTheTimer() { return fTimer; }
};



#endif


